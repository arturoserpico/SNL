#pragma once

#include "Sym.h"
#include "../Utils/Bounded.h"
#include "../Typing/Function.h"

namespace snl {
	template<typename T, typename I>
	struct SymSumOp : SymOpType<T(I, I, Function<I, T>)> {
		T eval(I min, I max, const Function<I, T>& expr) {
			T result = 0;

			I i = min;

			while (i < max) {
				result = result + expr(i);
				i++;
			}

			result = result + expr(i);

			return result;
		}
	};

	template<typename T, typename I>
	bool operator==(const SymSumOp<T, I>& a, const SymSumOp<T, I>& b) {
		return true;
	}

	template<typename I>
	class Sum {
		Sym<I> iterator, min, max;
	public:
		Sum(const Sym<I>& iterator, const Sym<I>& min, const Sym<I>& max) : 
			iterator(iterator), min(min), max(max) {}

		template<typename T>
		auto operator|(const Sym<T>& expr) {

			Sym<Function<I, T>> f;

			f = iterator >> expr;

			return Sym<T>(SymSumOp<T, I>(), min, max, f);
		}

		template<typename T>
		auto operator|(T val) {
			return this->operator|(Sym(val));
		}
	};


	template<typename... Is>
	class MultiSum {
		std::tuple<Sum<Is>...> sums;

		template<typename T, size_t index = 0>
		Sym<T> applyAll(const Sym<T>& expr) {
			if constexpr (index == sizeof...(Is) - 1)
				return std::get<index>(sums) | expr;
			else
				return std::get<index>(sums) | applyAll<T, index + 1>(expr);
		}
	public:
		MultiSum(Sum<Is>... sums) : sums(sums...) {}

		auto operator|(auto&& _expr) {
			auto [expr] = convertArgsToSymRef(std::forward<decltype(_expr)>(_expr));
			return applyAll(expr);
		}
	};

	template<typename I> 
	Sum<I> sum(const Sym<I>& iterator, const Sym<I>& min, const Sym<I>& max) {
		return Sum<I>(iterator, min, max);
	}

	template<typename I>
	Sum<I> sum(const Sym<I>& iterator, I min, I max) {
		return Sum<I>(iterator, Sym(min), Sym(max));
	}

	template<IsBounded I>
	Sum<I> sum(Sym<I>& iterator) {
		return sum(iterator, I(boundedMin<I>), I(boundedMax<I>));
	}

	template<IsBounded... Is>
	MultiSum<Is...> sum(Sym<Is>&... iterators) {
		return MultiSum<Is...>(sum(iterators)...);
	}
}