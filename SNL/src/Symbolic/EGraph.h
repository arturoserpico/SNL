#pragma once

#include "../Utils/UnionFind.h"
#include "../Utils/Any.h"
#include "SymNodes.h"
#include "../Utils/HashCombine.h"

namespace snl {
	struct EClass {
		size_t id;
		const std::type_info* type;

		EClass() = default;
		EClass(size_t id, const std::type_info& type) : id(id), type(&type) {}
	};

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

		friend class std::hash<snl::ENode>;
	};
}

namespace std {
	template<>
	struct hash<snl::EClass> {
		size_t operator()(const snl::EClass& eclass) {
			return snl::hashCombine(
				hash<size_t>{}(eclass.id),
				hash<const type_info*>{}(eclass.type)
			);
		}
	};

	template<>
	struct hash<snl::ENode> {
		size_t operator()(const snl::ENode& node) {
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