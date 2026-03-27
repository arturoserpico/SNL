#pragma once

#include "../Linear/Tensor.h"

namespace snl {
	template<typename T, typename I>
	struct SymSumOp : SymOpType<T(Ref<Sym<I>>, I, I, Ref<Sym<T>>)> {
		T eval(Ref<Sym<I>> iter, I min, I max, Ref<Sym<T>> expr) {
			T result = 0;

			iter.get() = min;

			while (true) {
				result += expr.get().eval();

				if (iter.get().eval() == max)
					break;

				iter.get() = iter.get().eval() + 1;
			}

			return result;
		}
	};

	template<typename T, typename I>
	bool operator==(const SymSumOp<T, I>& a, const SymSumOp<T, I>& b) {
		return true;
	}

	template<typename I>
	class Sum {
		Ref<Sym<I>> oldIterator;
		Ref<Sym<I>> iterator;
		Ref<Sym<I>> min, max;
	public:
		Sum(Ref<Sym<I>> iterator, Ref<Sym<I>> min, Ref<Sym<I>> max) : 
			oldIterator(iterator), iterator(makeManaged<Sym<I>>()), min(min), max(max) {}

		auto operator|(auto&& _expr) {
			auto [expr] = convertArgsToSymRef(std::forward<decltype(_expr)>(_expr));
			expr.get().substitute(oldIterator, iterator);
			return Sym(SymSumOp<RemSym<RemRef<decltype(expr)>>, I>(), iterator, min, max, expr);
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

		template<typename T, size_t index = 0>
		Sym<T> applyAll(Sym<T>& expr) {
			if constexpr (index == sizeof...(Is) - 1)
				return std::get<index>(sums) | expr;
			else
				return std::get<index>(sums) | applyAll<T, index + 1>(expr);
		}
	public:
		MultiSum(Sum<Is>... sums) : sums(sums...) {}

		auto operator|(auto&& _expr) {
			auto [expr] = convertArgsToSymRef(std::forward<decltype(_expr)>(_expr));
			return applyAll(expr.get());
		}
	};

	template<typename I> 
	Sum<I> sum(Sym<I> iterator, Sym<I>& min, Sym<I>& max) {
		return Sum<I>(Ref(iterator), min, max);
	}

	template<typename I>
	Sum<I> sum(Sym<I>& iterator, const Sym<I>& min, const Sym<I>& max) {
		return Sum<I>(Ref(iterator), makeManaged(min), makeManaged(max));
	}

	template<typename I>
	Sum<I> sum(Sym<I>& iterator, I min, I max) {
		return Sum<I>(Ref(iterator), makeManaged<Sym<I>>(min), makeManaged<Sym<I>>(max));
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