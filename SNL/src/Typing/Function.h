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

			Function& operator|=(const Sym<To>& result) const {
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

		auto call(const From& val) const {
			auto x = matchVar<To>();

			MatchResult result;

			auto constructor = Sym(Pair<From, To>::tuple);

			pairSet.contains(constructor(val, x), result);

			return result.get(x).eval();
		}

		auto operator()(const auto&... args) 
			requires (IsTuple<From> && (IsSym<std::remove_cvref_t<decltype(args)>> || ...))
		{
			return this->operator()(Sym(From::tuple)(args...));
		}

		auto operator()(const auto&... args)
			requires (IsTuple<From> && !(IsSym<std::remove_cvref_t<decltype(args)>> || ...))
		{
			return this->operator()(From::tuple(args...));
		}

		auto operator()(const auto&... args) const
			requires (IsTuple<From> && !(IsSym<std::remove_cvref_t<decltype(args)>> || ...))
		{
			return this->operator()(From::tuple(args...));
		}

		To operator()(const From& val) {
		 	return call(val);
		}

		To operator()(const From& val) const {
			return call(val);
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