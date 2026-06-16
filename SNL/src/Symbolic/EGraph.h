#pragma once

#include "../Utils/UnionFind.h"
#include "../Utils/Any.h"
#include "SymNodes.h"
#include "../Utils/HashCombine.h"

namespace snl {
	struct EClass;
	class ENode;
	class EGraph;
}

namespace std {
	template<>
	struct hash<snl::EClass>;

	template<>
	struct hash<snl::ENode>;
}

namespace snl {
	struct EClass {
		size_t id;
		const std::type_info* type;

		EClass() = default;
		EClass(size_t id, const std::type_info& type) : id(id), type(&type) {}
	};

	bool operator==(const EClass& a, const EClass& b) {
		return a.id == b.id && a.type == b.type;
	}

	using InvalidENodeArgsError = Error<"invalid arguments provided in ENode initialization">;

	class ENode {
		const std::type_info* type;
		Any<> evalObj;
		std::vector<EClass> args;

		template<IsSymOpType SymOpType, size_t index = 0>
		void checkArgsTypes() const {
			if constexpr (index == lenght<typename SymOpType::ArgsList>)
				return;
			else {
				expect<InvalidENodeArgsError>(
					*args[index].type == typeid(Get<typename SymOpType::ArgsList, index>)
				);

				checkArgsTypes<SymOpType, index + 1>();
			}
		}
	public:
		ENode() = default;

		template<IsSymOpType SymOpType>
		ENode(SymOpType evalObj, std::initializer_list<EClass> args) :
			type(&typeid(typename SymOpType::R)), evalObj(evalObj)
		{
			SNLDebugCall(1, expect<InvalidENodeArgsError>(lenght<typename SymOpType::ArgsList> == args.size()));
			this->args.reserve(args.size());

			for (const EClass& arg : args)
				this->args.push_back(arg);

			SNLDebugCall(1, checkArgsTypes<SymOpType>());
		}

		template<IsSymOpType SymOpType>
		ENode(SymOpType evalObj) : type(&typeid(typename SymOpType::R)), evalObj(evalObj) {
			SNLDebugCall(1, expect<InvalidENodeArgsError>(lenght<typename SymOpType::ArgsList> == 0));
		}

		const std::type_info& getType() const {
			return *type;
		}

		std::vector<EClass>& getArgs() {
			return args;
		}

		const std::vector<EClass>& getArgs() const {
			return args;
		}

		friend class std::hash<snl::ENode>;
	
		friend bool operator==(const ENode& a, const ENode& b) {
			return a.type == b.type && a.evalObj == b.evalObj && a.args == b.args;
		}
	};
}

namespace std {
	template<>
	struct hash<snl::EClass> {
		size_t operator()(const snl::EClass& eclass) const {
			return snl::hashCombine(
				hash<size_t>{}(eclass.id),
				hash<const type_info*>{}(eclass.type)
			);
		}
	};

	template<>
	struct hash<snl::ENode> {
		size_t operator()(const snl::ENode& node) const {
			size_t result = snl::hashCombine(
				hash<const type_info*>{}(node.type),
				hash<snl::Any<>>{}(node.evalObj)
			);

			for (const snl::EClass& eclass : node.args)
				result = snl::hashCombine(result, hash<snl::EClass>{}(eclass));

			return result;
		}
	};
}

namespace snl {
	class EGraph {
		std::unordered_map<ENode, EClass> memo;
		UnionFind<EClass> classes;

		ENode canonicalize(ENode node) const {
			for(EClass& arg : node.getArgs())
				arg = classes.find(arg);

			return node;
		}
	public:

		EGraph& add(ENode node) {
			node = canonicalize(node);

			if (memo.count(node))
				return *this;

			EClass newClass(memo.size(), node.getType());

			memo.emplace(node, newClass);
			classes.add(newClass);

			return *this;
		}

		EClass find(const ENode& node) const {
			return memo.at(canonicalize(node));
		}

		EGraph& merge(const EClass& a, const EClass& b) {
			classes.merge(a, b);
			return *this;
		}

		EGraph& rebuild() {
			bool changed = true;
			while (changed) {
				changed = false;
				std::unordered_map<ENode, EClass> newMemo;

				for (const auto& [node, eclass] : memo) {
					ENode canonical = canonicalize(node);
					EClass representative = classes.find(eclass);

					if(newMemo.count(canonical)) {
						EClass newRep = find(canonical);

						if (representative != newRep) {
							classes.merge(representative, newRep);
							changed = true;
						}
					}
					else
						newMemo.emplace(canonical, representative);
				}

				memo = newMemo;
			}

			return *this;
		}

		bool equal(const ENode& a, const ENode& b) const {
			return find(a) == find(b);
		}
	};
}