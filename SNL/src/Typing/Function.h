#pragma once

#include "AlgebraicTypes.h"
#include "Set.h"

namespace snl {

	template<typename From, typename To>
	class Function {
		Set<Pair<From, To>> pairSet = Set<Pair<From, To>>::empty;

		class PatternCreationProxy {
			Function& fun;
			Sym<From> pattern;
		public:
			PatternCreationProxy(Function& fun, const Sym<From>& pattern) : fun(fun), pattern(pattern) {}

			Function& operator|=(const Sym<To>& result) {
				auto constructor = Sym(Pair<From, To>::tuple);
				fun.pairSet = fun.pairSet | Set<Pair<From, To>>{ constructor(pattern, result) };
				return fun;
			}
		};
	public:
		Function() = default;
		Function(IsTypeObject auto type) : Function() {}

		Function(const Function<From, To>& other) {
			pairSet = other.pairSet;
		}

		PatternCreationProxy operator()(const Sym<From>& pattern) {
			return PatternCreationProxy(*this, pattern);
		}

		auto operator()(const auto&... args) requires IsTuple<From> {
			auto constructor = From::tuple;
			
			constexpr bool isSymbolic = (IsSym<std::remove_cvref_t<decltype(args)>> || ...);

			if constexpr (isSymbolic)
				return this->operator()(Sym(constructor)(args...));
			else
				return this->operator()(constructor(args...));
		}

		To operator()(const From& val) {
			auto x = matchVar<To>();

			MatchResult result;

			auto constructor = Sym(Pair<From, To>::tuple);

			pairSet.contains(constructor(val, x), result);

			return result.get(x).eval();
		}
	};

	template<typename From, typename To>
	consteval TypeObject<Function<From, To>> operator>>(TypeObject<From>, TypeObject<To>) {
		return {};
	}

	template<typename From, typename To>
	Function(TypeObject<Function<From, To>>) -> Function<From, To>;

	template<typename From, typename To>
	Function<From, To> operator>>(const Sym<From>& pattern, const Sym<To>& result) {
		Function<From, To> f;
		f(pattern) |= result;
		return f;
	}
}