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

	using RuleUnappliyableError = Error<"Can't apply rule to given GenericSym">;

	class MathRule {
	public:
		std::vector<Ref<GenericSym>> variables;
		size_t specificity;
		Ref<GenericSym> original, substitute;

		template<typename... Ts> requires (sizeof...(Ts) == 0)
		void makeRuleVars() {}

		template<typename First, typename... Rest>
		void makeRuleVars(Sym<First>& first, Sym<Rest>&... rest) {
			variables.insert(variables.begin(), makeManaged<Sym<First>>(SymRuleVar<First>()).as<GenericSym>());
			
			original.get().substitute(first, variables[0].as<Sym<First>>().get());
			substitute.get().substitute(first, variables[0].as<Sym<First>>().get());

			if constexpr (sizeof...(rest) != 0)
				makeRuleVars<Rest...>(rest...);
		}
	public:

		template<typename T, typename... Vars>
		MathRule(const Sym<T>& original, const Sym<T>& substitute, Sym<Vars>&... variables) :
			original(makeManaged<Sym<T>>(original.deepCopy()).as<GenericSym>()),
			substitute(makeManaged<Sym<T>>(substitute.deepCopy()).as<GenericSym>())
		{
			this->variables.reserve(sizeof...(variables));
			makeRuleVars<Vars...>(variables...);
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
						return varMap.at(Ref(other)).get() == node;
					else
						varMap.emplace(Ref(other), Ref(node, Ref(node).realManagmentState()));

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

		bool match(const GenericSym& node) const {
			std::unordered_map<Ref<const GenericSym>, Ref<const GenericSym>> varMap;
			return match(node, original.get(), varMap);
		}

		bool match(GenericSym& node) const {
			std::unordered_map<Ref<const GenericSym>, Ref<const GenericSym>> varMap;
			return match(node, original.get(), varMap);
		}

		bool match(
			const GenericSym& node, 
			std::unordered_map<Ref<const GenericSym>, Ref<const GenericSym>>& varMap
		) const {
			return match(node, original.get(), varMap);
		}

		bool match(
			GenericSym& node,
			std::unordered_map<Ref<const GenericSym>, Ref<const GenericSym>>& varMap
		) const {
			return match(node, original.get(), varMap);
		}

		template<typename T>
		Sym<T> apply(const Sym<T>& node) const {
			ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;

			std::unordered_map<Ref<const GenericSym>, Ref<const GenericSym>> varMap;
			expect<RuleUnappliyableError>(match(node, varMap));

			Ref<GenericSym> substituteCopy = substitute.get().heapCopy();

			for (auto [ruleVar, expr] : varMap)
				substituteCopy.get().substitute(ruleVar, expr);

			return substituteCopy.as<Sym<T>>().get();
		}
	};

	class RuleSet {
		std::vector<MathRule> rules;
	public:
		bool match(const GenericSym& node) {
			for (const MathRule& rule : rules)
				if (rule.match(node))
					return true;

			return false;
		}

		bool match(GenericSym& node) {
			for (const MathRule& rule : rules)
				if (rule.match(node))
					return true;

			return false;
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