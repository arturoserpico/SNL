#pragma once

#include <map>
#include <queue>
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

	using RuleUnappliyableError = Error<"Can't apply rule to given Sym">;
	using RuleSetUnappliyableError = Error<"Can't apply ruleset to given Sym">;

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

			if (node.nodeType() != other.nodeType() || node.rawDeps().size() != other.rawDeps().size())
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

		Ref<GenericSym> genericApply(const GenericSym& node) const {
			ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;

			std::unordered_map<Ref<const GenericSym>, Ref<const GenericSym>> varMap;
			expect<RuleUnappliyableError>(match(node, varMap));

			Ref<GenericSym> substituteCopy = substitute.get().heapCopy();

			for (auto [ruleVar, expr] : varMap)
				substituteCopy.get().substitute(ruleVar, expr);

			return substituteCopy;
		}

		template<typename T>
		Sym<T> apply(const Sym<T>& node) const {
			return genericApply(Ref(node).as<const GenericSym>().get()).as<Sym<T>>().get();
		}
	};

	class RuleSet {
		std::vector<MathRule> rules;
	public:
		RuleSet() = default;
		RuleSet(std::vector<MathRule> rules) : rules(rules) {}

		void add(const MathRule& rule) {
			rules.push_back(rule);
		}

		bool match(const GenericSym& node) const {
			for (const MathRule& rule : rules)
				if (rule.match(node))
					return true;

			return false;
		}

		bool match(GenericSym& node) const {
			for (const MathRule& rule : rules)
				if (rule.match(node))
					return true;

			return false;
		}

		Ref<GenericSym> genericApply(const GenericSym& node) const {
			for (const MathRule& rule : rules)
				if (rule.match(node))
					return rule.genericApply(node);

			throwError<RuleSetUnappliyableError>();
		}

		template<typename T>
		Sym<T> apply(const Sym<T>& node) const {
			for (const MathRule& rule : rules)
				if (rule.match(node))
					return rule.apply(node);

			throwError<RuleSetUnappliyableError>();
		}

		auto begin() {
			return rules.begin();
		}

		auto begin() const {
			return rules.begin();
		}

		auto end() {
			return rules.end();
		}

		auto end() const {
			return rules.end();
		}

		size_t size() const {
			return rules.size();
		}

		friend RuleSet mergeRuleSets(const RuleSet& a, const RuleSet& b);
	};

	RuleSet mergeRuleSets(const RuleSet& a, const RuleSet& b) {
		RuleSet result;
		result.rules.reserve(a.size() + b.size());
		
		for (const MathRule& rule : a)
			result.add(rule);

		for (const MathRule& rule : b)
			result.add(rule);

		return result;
	}

	class MathContext;

	class MathScope {
		Ref<MathContext> context;
		RuleSet rules;
	public:
		MathScope(Ref<MathContext> context, RuleSet rules = {}) : context(context), rules(rules) {}
		MathScope(MathContext& context, RuleSet rules = {}) : context(context), rules(rules) {}

		RuleSet& getRules() {
			return rules;
		}

		const RuleSet& getRules() const {
			return rules;
		}

		void add(const MathRule& rule) {
			rules.add(rule);
		}
	};

	class MathContext {
		std::deque<MathScope> scopes;
	public:
		MathContext() : scopes{ MathScope(*this) } {}

		RuleSet getRules() {
			RuleSet result;

			for (const MathScope& scope : scopes)
				result = mergeRuleSets(result, scope.getRules());

			return result;
		}

		MathScope& beginMathScope() {
			scopes.push_back(MathScope(*this));
			return scopes.back();
		}

		void endMathScope() {
			scopes.pop_back();
		}

		void addRule(const MathRule& rule) {
			scopes.back().add(rule);
		}

		bool match(const GenericSym& sym) {
			return getRules().match(sym);
		}

		template<typename T>
		Sym<T> apply(const Sym<T>& sym) {
			return getRules().apply(sym);
		}

		Ref<GenericSym> genericApply(const GenericSym& sym) {
			return getRules().genericApply(sym);
		}
	};

	static MathContext mathContext;

	bool matchRules(const GenericSym& sym) {
		return mathContext.match(sym);
	}

	template<typename T>
	Sym<T> applyRules(const Sym<T>& sym) {
		return mathContext.apply(sym);
	}

	Ref<GenericSym> genericApplyRules(const GenericSym& sym) {
		return mathContext.genericApply(sym);
	}

	MathScope& beginMathScope() {
		return mathContext.beginMathScope();
	}

	void endMathScope() {
		mathContext.endMathScope();
	}

	template<typename T, typename... Vars>
	void defineRule(const Sym<T>& original, const Sym<T>& substitute, Sym<Vars>&... variables) {
		mathContext.addRule(MathRule(original, substitute, variables...));
	}

	void defineRule(const MathRule& rule) {
		mathContext.addRule(rule);
	}

	class Simplifier {
	protected:
		Ref<MathContext> context;

		virtual Ref<GenericSym> genericSimplify(const GenericSym&) = 0;
	public:
		Simplifier() : context(mathContext) {}
		Simplifier(Ref<MathContext> context) : context(context) {}
		Simplifier(MathContext& context) : context(context) {}

		template<typename T>
		Sym<T> simplify(const Sym<T>& sym) {
			return genericSimplify(sym).as<Sym<T>>().get();
		}
	};

	class MakeEvaluableSimplifier : public Simplifier {
		void recursiveSimplify(GenericSym& sym) {
			for (Ref<GenericSym> dep : sym.rawDeps())
				recursiveSimplify(dep.get());

			if (sym.isEvaluable())
				return;

			for(const MathRule& rule : context.get().getRules())
				if (rule.match(sym)) {
					auto applied = rule.genericApply(sym);

					if (applied.get().unevaluableLeafCount() < sym.unevaluableLeafCount()) {
						sym = applied.get();
						return;
					}
				}
		}

		Ref<GenericSym> genericSimplify(const GenericSym& sym) {
			Ref<GenericSym> result = sym.heapCopy();
			Ref<GenericSym> last = result.get().heapCopy();
			recursiveSimplify(result.get());

			while (last.get() != result.get()) {
				last = result.get().heapCopy();
				recursiveSimplify(result.get());
			}

			return result;
		}
	public:
		using Simplifier::Simplifier;
	};

	template<typename T>
	T Sym<T>::eval() {
		if (!isEvaluable()) {
			MakeEvaluableSimplifier simp;
			auto simplified = simp.simplify(*this);

			if (!simplified.isEvaluable())
				throwError<UnevaulableSymEvalError>();

			return simplified.eval();
		}

		return fun(isEvaluable(), evalObj, deps);
	}
}

namespace std {
	template<typename T>
	struct hash<snl::SymRuleVar<T>> {
		size_t operator()(const snl::SymRuleVar<T>& obj) {
			return obj.labelId;
		}
	};
}