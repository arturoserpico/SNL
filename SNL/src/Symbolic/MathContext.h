#pragma once

#include <ranges>
#include <map>
#include <queue>
#include "Sym.h"

namespace snl {
	using RuleNotInvertibleError = Error<"tried to invert a not invertible MathRule">;
	using MatchNotAppliableError = Error<"SymMatch impossible with given inputs">;

	static std::map<const std::type_info*, const std::type_info*> matchVarTypes;

	template<typename T>
	struct MatchVar : SymOpType<T()> {
		MatchVar() {
			matchVarTypes.emplace(&typeid(T), &typeid(MatchVar));
		}

		static size_t lastLabelId;
		size_t labelId = lastLabelId++;
		T eval() = delete;
	};

	template<typename T>
	size_t MatchVar<T>::lastLabelId = 0;

	template<typename T>
	bool operator==(const MatchVar<T>& a, const MatchVar<T>& b) {
		return a.labelId == b.labelId;
	}

	template<typename T>
	Sym<T> matchVar() {
		return Sym<T>(MatchVar<T>());
	}

	bool isMatchVar(const GenericSym& node) {
		return matchVarTypes[&node.symType()] == &node.nodeType();
	}

	template<typename T>
	bool isMatchVar(const Sym<T>& node) {
		ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;
		return isMatchVar(Ref(node).as<const GenericSym>().get());
	}

	class MatchResult {
		std::unordered_map<Ref<const GenericSym>, Ref<const GenericSym>> map;
	public:
		auto begin() const {
			return map.begin();
		}

		auto end() const {
			return map.end();
		}

		void add(const GenericSym& var, const GenericSym& substitute) {
			map.emplace(var.heapCopy(), substitute.heapCopy());
		}

		void substituteAll(GenericSym& target) const {
			for (auto [var, substitute] : map)
				target.substitute(var.get(), substitute.get());
		}

		const GenericSym& get(const GenericSym& var) const {
			ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;
			return map.at(Ref(var)).get();
		}

		template<typename T>
		const Sym<T>& get(const Sym<T>& var) const {
			ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;
			return Ref(get(Ref(var).as<const GenericSym>().get())).as<const Sym<T>>().get();
		}
	};

	bool canBeMatched(const GenericSym& node) {
		if (isMatchVar(node))
			return false;

		bool result = true;

		for (Ref<const GenericSym> dep : node.rawDeps())
			result &= canBeMatched(dep.get());

		return result;
	}

	bool symMatch(
		const GenericSym& a,
		const GenericSym& b,
		MatchResult& varMap
	) {
		auto bSubst = b.heapCopy();
		varMap.substituteAll(bSubst.get());
		auto aSubst = a.heapCopy();
		varMap.substituteAll(aSubst.get());

		if (isMatchVar(a)) {
			if (!canBeMatched(bSubst.get()) || a.symType() != bSubst.get().symType())
				return false;
			
			varMap.add(a, bSubst.get());
			return true;
		}

		if (isMatchVar(b)) {
			if (!canBeMatched(aSubst.get()) || b.symType() != aSubst.get().symType())
				return false;

			varMap.add(b, aSubst.get());
			return true;
		}

		if (aSubst.get().isEvaluable() && bSubst.get().isEvaluable())
			return aSubst.get().genericEvalCompare(bSubst.get());

		if (a.evalObj != b.evalObj)
			return false;

		if (a.rawDeps().size() != b.rawDeps().size())
			return false;

		for (size_t i = 0; i < a.rawDeps().size(); i++)
			if (!symMatch(a.rawDeps()[i].get(), b.rawDeps()[i].get(), varMap))
				return false;

		return true;
	}
	
	template<typename T>
	bool symMatch(
		const Sym<T>& a,
		const Sym<T>& b,
		MatchResult& varMap
	) {
		ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;
		return symMatch(Ref(a).as<const GenericSym>().get(), Ref(b).as<const GenericSym>().get(), varMap);
	}

	template<typename T>
	bool symMatch(const Sym<T>& a, const Sym<T>& b) {
		MatchResult varMap;
		return symMatch(a, b, varMap);
	}

	using RuleUnappliyableError = Error<"Can't apply rule to given Sym">;
	using RuleSetUnappliyableError = Error<"Can't apply ruleset to given Sym">;

	class MathRule {
	public:
		//std::vector<Ref<GenericSym>> variables;
		Ref<GenericSym> original = nullptr, substitute = nullptr;

		//template<typename... Ts> requires (sizeof...(Ts) == 0)
		//void makeRuleVars() {}
		//
		//template<typename First, typename... Rest>
		//void makeRuleVars(Sym<First>& first, Sym<Rest>&... rest) {
		//	variables.insert(variables.begin(), makeManaged<Sym<First>>(MatchVar<First>()).as<GenericSym>());
		//	
		//	original.get().substitute(first, variables[0].as<Sym<First>>().get());
		//	substitute.get().substitute(first, variables[0].as<Sym<First>>().get());
		//
		//	if constexpr (sizeof...(rest) != 0)
		//		makeRuleVars<Rest...>(rest...);
		//}
	public:
		//MathRule(Ref<const GenericSym> original, Ref<const GenericSym> substitute, std::vector<Ref<GenericSym>>)
		//{
		//	ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;
		//	//this->variables = variables;
		//	this->original = original.get().heapCopy();
		//	this->substitute = substitute.get().heapCopy();
		//}

		//template<typename T, typename... Vars>
		//MathRule(const Sym<T>& original, const Sym<T>& substitute, Sym<Vars>&... variables) :
		//	original(makeManaged<Sym<T>>(original.deepCopy()).as<GenericSym>()),
		//	substitute(makeManaged<Sym<T>>(substitute.deepCopy()).as<GenericSym>())
		//{
		//	this->variables.reserve(sizeof...(variables));
		//	makeRuleVars<Vars...>(variables...);
		//}

		template<typename T>
		MathRule(const Sym<T>& original, const Sym<T>& substitute) :
			original(makeManaged<Sym<T>>(original.deepCopy()).as<GenericSym>()),
			substitute(makeManaged<Sym<T>>(substitute.deepCopy()).as<GenericSym>())
		{}

		bool match(const GenericSym& node) const {
			MatchResult varMap;
			return symMatch(original.get(), node, varMap);
		}

		bool match(GenericSym& node) const {
			MatchResult varMap;
			return symMatch(original.get(), node, varMap);
		}

		bool match(
			const GenericSym& node, 
			MatchResult& varMap
		) const {
			return symMatch(original.get(), node, varMap);
		}

		bool match(
			GenericSym& node,
			MatchResult& varMap
		) const {
			return symMatch(original.get(), node, varMap);
		}

		Ref<GenericSym> genericApply(const GenericSym& node) const {
			ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;

			MatchResult varMap;
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

		//bool invertible() const {
		//	for (Ref<GenericSym> var : variables) {
		//		if (original.get().occurences(var.asConst()) == 0 ||
		//			substitute.get().occurences(var.asConst()) == 0)
		//			return false;
		//	}
		//
		//	return true;
		//}
		//
		//MathRule inverse() const {
		//	expect<RuleNotInvertibleError>(invertible());
		//	return MathRule(substitute, original, variables);
		//}
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

	template<typename T>
	void defineRule(const Sym<T>& original, const Sym<T>& substitute, bool includeInverse = true) {
		defineRule(MathRule(original, substitute), includeInverse);
	}

	void defineRule(const MathRule& rule, bool includeInverse = true) {
		mathContext.addRule(rule);

		//if (rule.invertible() && includeInverse)
		//	mathContext.addRule(rule.inverse());
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

	class TrivialLeafSimplify : public Simplifier {
		Ref<GenericSym> genericSimplify(const GenericSym& sym) override {
			auto result = sym.heapCopy();

			if (sym.isEvaluable())
				return result;

			for (Ref<GenericSym>& dep : result.get().rawDeps())
				dep = genericSimplify(dep.get());

			if (sym.isEvaluable())
				return result;

			auto orderer = [result](const MathRule& rule) -> size_t {
				if (!rule.match(result.get()))
					return 0;

				auto applied = rule.genericApply(result.get());
				return result.get().unevaluableLeafCount() - applied.get().unevaluableLeafCount();
				};

			RuleSet rules = context.get().getRules();

			rules = rules.sort(std::function(orderer), std::greater());

			for (const MathRule& rule : rules)
				if (rule.match(result.get())) {
					result = rule.genericApply(result.get());

					if (result.get().isEvaluable())
						return result;
				}

			return result;
		}
	public:
		using Simplifier::Simplifier;
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
	T Sym<T>::eval() const {
		return evalFun(isEvaluable(), evalObj, deps);
	}
}

namespace std {
	template<typename T>
	struct hash<snl::MatchVar<T>> {
		size_t operator()(const snl::MatchVar<T>& obj) {
			return obj.labelId;
		}
	};
}