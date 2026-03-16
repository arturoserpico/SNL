#pragma once

#include <array>
#include <algorithm>

#include "../Utils/Bounded.h"
#include "../Symbolic/Function.h"

namespace snl {
	template<typename T, size_t nCovariant, size_t nContravariant, size_t... sizes>
		requires (sizeof...(sizes) == nCovariant + nContravariant)
	class Tensor;

	template<size_t dim>
	using Index = Sym<Bounded<size_t, 0, dim - 1>>;

	template<typename T, size_t nCovariant, size_t nContravariant, size_t... sizes>
		requires (sizeof...(sizes) == nCovariant + nContravariant)
	class TensorIndexing : public SymOpType<T(Bounded<size_t, 0, sizes - 1>...)> {
		Ref<Tensor<T, nCovariant, nContravariant, sizes...>> tensor;
	public: 
		TensorIndexing(Tensor<T, nCovariant, nContravariant, sizes...>& tensor) : tensor(tensor) {}

		T eval(Bounded<size_t, 0, sizes - 1>... indexs) {
			return tensor.get()[std::array<size_t, nCovariant + nContravariant>{indexs...}];
		}
	};

	template<typename T, size_t nCovariant, size_t nContravariant, size_t... sizes>
		requires (sizeof...(sizes) == nCovariant + nContravariant)
	class TensorIndexingProxy {
		std::tuple<Ref<Index<sizes>>...> indexs;
		std::array<bool, nCovariant + nContravariant> varying;
		Ref<Tensor<T, nCovariant, nContravariant, sizes...>> tensor;

		static std::array<size_t, nCovariant + nContravariant> computeIndexs(std::tuple<Ref<Index<sizes>>...> indexs) {
			std::array<size_t, nCovariant + nContravariant> result;
			std::apply([&result](auto&&... indexs) {
				size_t i = 0;
				((result[i++] = indexs.get().compute().get()), ...);
			}, indexs);
			return result;
		}
	
		template<size_t index = 0>
		void assignTensor(Sym<T>& expr) {
			if (varying[index]) {
				for (size_t i = 0; i < std::array{ sizes... }[index]; i++) {
					std::get<index>(indexs).get().set(i);

					if constexpr (index == nCovariant + nContravariant - 1)
						tensor.get()[computeIndexs(indexs)] = expr.compute().get();
					else
						assignTensor<index + 1>(expr);
				}
			}
			else {
				if constexpr (index == nCovariant + nContravariant - 1)
					tensor.get()[computeIndexs(indexs)] = expr.compute().get();
				else
					assignTensor<index + 1>(expr);
			}
		}
	public:
		TensorIndexingProxy(
			Tensor<T, nCovariant, nContravariant, sizes...>& tensor,
			std::tuple<Ref<Index<sizes>>...> indexs, 
			std::array<bool, nCovariant + nContravariant> varying
		) : tensor(tensor), indexs(indexs), varying(varying) {
		}

		Sym<T> sym() {
			return Sym<T>(TensorIndexing<T, nCovariant, nContravariant, sizes...>(tensor), indexs);
		}

		operator Sym<T> () {
			return sym();
		}

		operator T() {
			return sym().compute().get();
		}

		TensorIndexingProxy<T, nCovariant, nContravariant, sizes...>& operator=(Sym<T> expr) {
			assignTensor(expr);
			return *this;
		}
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

		T& operator[](const std::array<size_t, 0>&) {
			return value;
		}

		const T& operator[](const std::array<size_t, 0>&) const {
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
		const T& operator[](const std::array<size_t, nCovariant + nContravariant>& indexs) const {
			std::array<size_t, nCovariant + nContravariant - 1> inner;
			std::copy(indexs.begin().operator+(1), indexs.end(), inner.begin());
			return data[indexs[0]][inner];
		}

		T& operator[](const std::array<size_t, nCovariant + nContravariant>& indexs) {
			std::array<size_t, nCovariant + nContravariant - 1> inner;
			std::copy(indexs.begin().operator+(1), indexs.end(), inner.begin());
			return data[indexs[0]][inner];
		}

		auto operator()(auto&&... indexs) {
			using Types = TypeList<Bounded<size_t, 0, first - 1>, Bounded<size_t, 0, rest - 1>...>;
			auto tuple = convertArgsToSymRef<Types>(std::forward<decltype(indexs)>(indexs)...);
			return TensorIndexingProxy<T, nCovariant, nContravariant, first, rest...>(*this, tuple, witchSymbolic(std::forward<decltype(indexs)>(indexs)...));
		}
	};
}