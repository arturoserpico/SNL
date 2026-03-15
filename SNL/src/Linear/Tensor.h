#pragma once

#include <array>

#include "../Utils/Bounded.h"
#include "../Symbolic/Function.h"

namespace snl {
	template<typename T, size_t nCovariant, size_t nContravariant, size_t... sizes>
		requires (sizeof...(sizes) == nCovariant + nContravariant)
	class Tensor;

	template<size_t dim>
	using Index = Sym<Bounded<size_t, 0, dim - 1>>;

	template<size_t nCovariant, size_t nContravariant, size_t... sizes>
	class IndexSet {

	};

	template<typename T>
	class Tensor<T, 0, 0> {
		T value;
	public:
		operator T& () {
			return value;
		}

		operator const T& () const {
			return value;
		}

		T& operator()(const std::array<size_t, 0>&) {
			return value;
		}

		const T& operator()(const std::array<size_t, 0>&) const {
			return value;
		}
	};

	template<typename T, size_t nCovariant, size_t nContravariant, size_t first, size_t... rest>
	class Tensor<T, nCovariant, nContravariant, first, rest...> {
		static constexpr bool isContravariant = nCovariant == 0;

		using SliceType = std::conditional_t<isContravariant, 
			Tensor<T, nCovariant, nContravariant - 1, rest...>, 
			Tensor<T, nCovariant - 1, nContravariant, rest...>
		>;

		std::array<SliceType, first> data;
	public:
		T& operator()(const std::array<size_t, nCovariant + nContravariant>& indexs) {
			std::array<size_t, nCovariant + nContravariant - 1> inner;
			std::copy(indexs.begin() + 1, indexs.end(), inner.begin());
			return data[indexs[0]](inner);
		}
	};
}