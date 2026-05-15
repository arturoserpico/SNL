#pragma once

#include <map>
#include <queue>
#include "../Utils/Set.h"
#include "Sym.h"
#include "Function.h"


namespace snl {
	using RuleNotInvertibleError = Error<"tried to invert a not invertible MathRule">;

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
		Ref<GenericSym> original = nullptr, substitute = nullptr;

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
		MathRule(Ref<const GenericSym> original, Ref<const GenericSym> substitute, std::vector<Ref<GenericSym>> variables)
		{
			ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;
			this->variables = variables;
			this->original = original.get().heapCopy();
			this->substitute = substitute.get().heapCopy();
		}

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

		bool invertible() const {
			for (Ref<GenericSym> var : variables) {
				if (original.get().occurences(var.asConst()) == 0 ||
					substitute.get().occurences(var.asConst()) == 0)
					return false;
			}

			return true;
		}

		MathRule inverse() const {
			expect<RuleNotInvertibleError>(invertible());
			return MathRule(substitute, original, variables);
		}
	};

	class RuleSet {
		std::vector<MathRule> rules;
	public:
		RuleSet() = default;
		RuleSet(std::vector<MathRule> rules) : rules(rules) {}
		RuleSet(const RuleSet& other) : rules(other.rules) {}

		template<std::three_way_comparable T>
		RuleSet sort(const std::function<T(const MathRule&)>& fun, const auto& comp = std::less()) const {
			auto comparator = [fun, comp](const MathRule& a, const MathRule& b) {
					return  comp(fun(a), fun(b));
				};

			RuleSet result = *this;

			std::sort(result.rules.begin(), result.rules.end(), comparator);

			return result;
		}

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
	void defineRule(bool includeInverse, const Sym<T>& original, const Sym<T>& substitute, Sym<Vars>&... variables) {
		defineRule(MathRule(original, substitute, variables...), includeInverse);
	}

	template<typename T, typename... Vars>
	void defineRule(const Sym<T>& original, const Sym<T>& substitute, Sym<Vars>&... variables) {
		defineRule(MathRule(original, substitute, variables...));
	}

	void defineRule(const MathRule& rule, bool includeInverse = true) {
		mathContext.addRule(rule);

		if (rule.invertible() && includeInverse)
			mathContext.addRule(rule.inverse());
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
		Ref<GenericSym> recursiveSimplify(const GenericSym& sym, std::set<size_t>& visited) {
			auto original = sym.heapCopy();
			
			if (visited.count(std::hash<GenericSym>()(sym)))
				return original;

			visited.insert(std::hash<GenericSym>()(sym));

			if (original.get().isEvaluable())
				return original;

			for (Ref<GenericSym>& dep : original.get().rawDeps())
				dep = recursiveSimplify(dep.get());

			if (original.get().isEvaluable())
				return original;

			auto orderer = [original](const MathRule& rule) -> size_t {
					if (!rule.match(original.get()))
						return 0;

					auto applied = rule.genericApply(original.get());
					return original.get().unevaluableLeafCount() - applied.get().unevaluableLeafCount();
				};

			RuleSet rules = context.get().getRules();

			rules = rules.sort(std::function(orderer), std::greater());

			for(const MathRule& rule : rules)
				if (rule.match(original.get())) {
					auto copy = rule.genericApply(original.get());
					
					if (copy.get().isEvaluable())
						return copy;

					copy = recursiveSimplify(copy.get(), visited);

					if (copy.get().isEvaluable())
						return copy;
				}

			return original;
		}

		Ref<GenericSym> recursiveSimplify(const GenericSym& sym) {
			std::set<size_t> visited;
			return recursiveSimplify(sym, visited);
		}

		Ref<GenericSym> genericSimplify(const GenericSym& sym) {
			Ref<GenericSym> result = sym.heapCopy();
			//Ref<GenericSym> last = sym.heapCopy();
			//result = recursiveSimplify(result.get());
			//
			//while (last.get() != result.get()) {
			//	last = result.get().heapCopy();
			//	result = recursiveSimplify(result.get());
			//} 
			//
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