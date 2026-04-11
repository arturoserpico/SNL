#pragma once

#include <map>
#include "../Utils/Set.h"
#include "Sym.h"
#include "Function.h"


namespace snl {
	static std::map<const std::type_info*, const std::type_info*> symRuleVarTypes;

	template<typename T>
	struct SymRuleVar : SymOpType<T()> {
		SymRuleVar() {
			symRuleVarTypes.emplace(&typeid(T), &typeid(SymRuleVar));
		}

		static size_t lastLabelId;
		size_t labelId = lastLabelId++;
		T eval() = delete;
	};

	template<typename T>
	size_t SymRuleVar<T>::lastLabelId = 0;

	template<typename T>
	bool operator==(const SymRuleVar<T>& a, const SymRuleVar<T>& b) {
		return a.labelId == b.labelId;
	}

	class MathRule {
	public:
		std::vector<Ref<GenericSym>> variables;
		size_t specificity;
		Ref<GenericSym> original, substitute;

		template<typename First, typename... Rest>
		void substituteAll(Sym<First>& first, Sym<Rest>&... rest) {
			variables.insert(variables.begin(), makeManaged<Sym<First>>(SymRuleVar<First>()).as<GenericSym>());
			
			original.get().substitute(first, variables[0].as<Sym<First>>().get());
			substitute.get().substitute(first, variables[0].as<Sym<First>>().get());

			if constexpr (sizeof...(rest) != 0)
				substituteAll<Rest...>(rest...);
		}
	public:

		template<typename T, typename... Vars>
		MathRule(const Sym<T>& original, const Sym<T>& substitute, Sym<Vars>&... variables) :
			original(makeManaged<Sym<T>>(original.deepCopy()).as<GenericSym>()),
			substitute(makeManaged<Sym<T>>(substitute.deepCopy()).as<GenericSym>())
		{
			this->variables.reserve(sizeof...(variables));
			substituteAll<Vars...>(variables...);
		}

		static bool match(
			const GenericSym& node, 
			const GenericSym& other, 
			std::unordered_map<Ref<const GenericSym>, Ref<const GenericSym>>& varMap
		) {
			ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;
			
			if (symRuleVarTypes.count(&node.symType())) {
				if (other.nodeType() == *symRuleVarTypes.at(&node.symType())) {
					if (varMap.count(other))
						return varMap.at(other).get() == node;
					else
						varMap.emplace(other, node);

					return true;
				}
			}

			if (node.nodeType() != other.nodeType() && node.rawDeps().size() == other.rawDeps().size())
				return false;

			if (node.rawDeps().size() == 0)
				return node == other;

			for (size_t i = 0; i < node.rawDeps().size(); i++)
				if (!match(node.rawDeps()[i].get(), other.rawDeps()[i].get(), varMap))
					return false;

			return true;
		}

		bool match(const GenericSym& node) {
			std::unordered_map<Ref<const GenericSym>, Ref<const GenericSym>> varMap;
			return match(node, original, varMap);
		}
	};

	class MathScope {
		
	};

	MathScope currentMathScope;
}

namespace std {
	template<typename T>
	struct hash<snl::SymRuleVar<T>> {
		size_t operator()(const snl::SymRuleVar<T>& obj) {
			return obj.labelId;
		}
	};
}