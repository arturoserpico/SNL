#pragma once

#include <array>
#include <algorithm>

#include "../Utils/Bounded.h"
#include "../Symbolic/Function.h"
#include "../Symbolic/ExprOperators.h"

namespace snl {
	template<typename T, size_t nCovariant, size_t nContravariant, size_t... sizes>
		requires (sizeof...(sizes) == nCovariant + nContravariant)
	class Tensor;

	template<typename T, size_t dim>
	using Vector = Tensor<T, 0, 1, dim>;

	template<typename T, size_t dim>
	using CoVector = Tensor<T, 1, 0, dim>;

	template<typename T, size_t n, size_t m = n>
	using Matrix11 = Tensor<T, 1, 1, n, m>;

	template<typename T, size_t n, size_t m = n>
	using Matrix20 = Tensor<T, 2, 0, n, m>;

	template<typename T, size_t n, size_t m = n>
	using Matrix02 = Tensor<T, 2, 0, n, m>;

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
				//#pragma omp parallel for
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

		TensorIndexingProxy<T, nCovariant, nContravariant, sizes...>& operator|=(Sym<T> expr) {
			assignTensor(expr);
			return *this;
		}
	};

	template<typename T>
	class Tensor<T, 0, 0> {
		T value;
	public:
		Tensor() = default;
		Tensor(T value) : value(value) {}

		T& get() {
			return value;
		}

		const T& get() const {
			return value;
		}

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

		//using SliceType = std::conditional_t<isContravariant, 
		//	Tensor<T, nCovariant, nContravariant - 1, rest...>, 
		//	Tensor<T, nCovariant - 1, nContravariant, rest...>
		//>;

		template<size_t index>
		using VectorArg =
			std::conditional_t<
				index < nCovariant,  
				Vector<T, std::array{ first, rest... }[index]>,
				CoVector<T, std::array{ first, rest... }[index]>>;

		using DataT = std::conditional_t<(first * ... * rest) < 1000, std::array<T, (first * ... * rest)>, std::vector<T>>;

		DataT data;

		TensorIndexingProxy<T, nCovariant, nContravariant, first, rest...> 
		indexTensor(auto&&... indexs) {
			using Types = TypeList<Bounded<size_t, 0, first - 1>, Bounded<size_t, 0, rest - 1>...>;
			auto tuple = convertArgsToSymRef<Types>(std::forward<decltype(indexs)>(indexs)...);
			return TensorIndexingProxy<T, nCovariant, nContravariant, first, rest...>(*this, tuple, witchSymbolic(std::forward<decltype(indexs)>(indexs)...));
		}

		template<size_t... indexs>
		Sym<T> multilinearApplication(
			std::index_sequence<indexs...> _,
			VectorArg<indexs>&... vectors
		) {
			std::tuple<Ref<Index<first>>, Ref<Index<rest>>...> symIndexs =
			{ makeManaged<Index<first>>(), makeManaged<Index<rest>>()... };

			return sum(std::get<indexs>(symIndexs).get()...) | this->indexTensor(std::get<indexs>(symIndexs).get()...) * (vectors(std::get<indexs>(symIndexs).get()) * ...);
		}

		size_t flattenIndexs(const std::array<size_t, nCovariant + nContravariant>& indexs) {
			constexpr std::array<size_t, nCovariant + nContravariant> sizes{ first, rest... };
			
			std::size_t result = 0;
			std::size_t stride = 1;

			for (int i = nCovariant + nContravariant - 1; i >= 0; --i) {
				result += indexs[i] * stride;
				stride *= sizes[i];
			}

			return result;
		}
	public:
		Tensor() {
			if constexpr ((first * ... * rest) >= 1000)
				data.resize((first * ... * rest));
		}

		const T& operator[](const std::array<size_t, nCovariant + nContravariant>& indexs) const {
			return data[flattenIndexs(indexs)];
		}

		T& operator[](const std::array<size_t, nCovariant + nContravariant>& indexs) {
			return data[flattenIndexs(indexs)];
		}

		auto operator()(auto&&... indexs) {
			constexpr bool isMultilinearApplication = requires(decltype(indexs)... vals) {
				multilinearApplication(std::make_index_sequence<nCovariant + nContravariant>(), vals...);
			};

			if constexpr (isMultilinearApplication)
				return multilinearApplication(std::make_index_sequence<nCovariant + nContravariant>(), indexs...);
			else
				return indexTensor(std::forward<decltype(indexs)>(indexs)...);
		}
	};

	template<typename T>
	constexpr bool isTensor = false;

	template<typename T, size_t nCovariant, size_t nContravariant, size_t... sizes>
	constexpr bool isTensor<Tensor<T, nCovariant, nContravariant, sizes...>> = true;

	template<typename T>
	concept IsTensor = isTensor<T>;

	template<typename T>
	struct _TensorType;

	template<typename T, size_t nCovariant, size_t nContravariant, size_t... sizes>
	struct _TensorType<Tensor<T, nCovariant, nContravariant, sizes...>> : TypeAlias<T> {};

	template<typename T>
	using TensorType = _TensorType<T>::Type;

	template<typename A, typename B>
		requires requires (A a, B b) { a + b; }
	auto operator+(Tensor<A, 0, 0> a, Tensor<B, 0, 0> b) {
		return Tensor<decltype(A() + B()), 0, 0>(a.get() + b.get());
	}

	template<typename A, typename B, size_t nCovariant, size_t nContravariant, size_t first, size_t... rest>
		requires requires (A a, B b) { a + b; }
	auto operator+(
		Tensor<A, nCovariant, nContravariant, first, rest...> a,
		Tensor<B, nCovariant, nContravariant, first, rest...> b
	) {
		Tensor<decltype(A() + B()), nCovariant, nContravariant, first, rest...> result;

		for (size_t i = 0; i < first; i++)
			result[i] = a[i] + b[i];

		return result;
	}

	template<typename T, size_t nCovariant, size_t nContravariant, size_t n>
	std::ostream& operator<<(std::ostream& stream, Tensor<T, nCovariant, nContravariant, n> vec) {
		for(size_t i = 0; i < n; i++) {
			stream << vec(i) << "\n";
		}

		return stream;
	}

	template<typename T, size_t nCovariant, size_t nContravariant, size_t n, size_t m>
	std::ostream& operator<<(std::ostream& stream, Tensor<T, nCovariant, nContravariant, n, m> mat) {
		for (size_t j = 0; j < m; j++) {
			for (size_t i = 0; i < n; i++) {
				stream << mat(i, j) << " ";
			}

			stream << "\n";
		}

		return stream;
	}
}