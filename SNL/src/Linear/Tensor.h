#pragma once

namespace snl {

	template<typename T, size_t nCovariant, size_t nContrvariant, size_t... sizes>
	requires (sizeof...(sizes) == nCovariant + nContrvariant)
	class Tensor;

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
	};

	template<typename T, size_t nCovariant, size_t nContrvariant, size_t first, size_t... rest>
	class Tensor<T, nCovariant, nContrvariant, first, rest...> {

	};
}