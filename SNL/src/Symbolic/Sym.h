#pragma once

#include <optional>
#include <vector>
#include <functional>
#include "../Utils/Ref.h"
#include "../Utils/Error.h"
#include "../Metaprogramming/Concepts.h"
#include "../Metaprogramming/TypeList.h"

namespace snl {
	template<typename T>
	class GenericSym {
	public:
		virtual void compute() = 0;
		virtual T get() const = 0;
	};

	template<typename T, typename... Dependencies>
	class Sym;

	template<typename T>
	struct _GetSymbolicTypeInfo;

	template<typename T, typename... Dependencies>
	struct _GetSymbolicTypeInfo<Sym<T, Dependencies...>> {
		using Type = T;
		using DepList = TypeList<Dependencies...>;
	};

	template<typename T>
	using GetSymbolicTypeInfo = _GetSymbolicTypeInfo<std::remove_cvref_t<T>>;

	template<typename T>
	constexpr bool _isSymbolic = false;

	template<typename... Ts>
	constexpr bool _isSymbolic<Sym<Ts...>> = true;

	template<typename T>
	constexpr bool isSymbolic = _isSymbolic<std::remove_cvref_t<T>>;

	template<typename T>
	concept IsSymbolic = isSymbolic<T>;

	template<typename T>
	concept IsSymbolicIgnoreV = isSymbolic<std::remove_volatile_t<T>>;

	template<typename T>
	concept IsSymbolicIgnoreCV = isSymbolic<std::remove_cv_t<T>>;

	template<typename T>
	concept IsSymbolicIgnoreCVRef = isSymbolic<std::remove_cvref_t<T>>;

	template<typename T, typename... Dependencies>
	class Sym : public GenericSym<T> {
		std::optional<T> data;
		std::optional<std::function<T(Dependencies...)>> fun;
		std::tuple<Ref<GenericSym<Dependencies>>...> dependencies;
		std::vector<std::shared_ptr<void>> ownedList;

		template<size_t index = 0>
		void initDepList(IsSymbolicIgnoreCV auto& first, IsSymbolicIgnoreCV auto&... rest) {
			Ref<GenericSym<Get<TypeList<Dependencies...>, index>>> ref;

			if constexpr (std::is_const_v<std::remove_reference_t<decltype(first)>>) {
				using T = std::remove_cvref_t<decltype(first)>;
				std::shared_ptr<T> elementCopy = std::make_shared<T>(T(first));
				ownedList.push_back(elementCopy);
				ref.bind(*elementCopy);
			}
			else {
				ref.bind(first);
			}

			std::get<index>(dependencies) = ref;

			if constexpr (index < (sizeof...(Dependencies) - 1))
				initDepList<index + 1>(rest...);
		}
	public:
		Sym() requires (sizeof...(Dependencies) == 0) = default;
		Sym(T value) requires (sizeof...(Dependencies) == 0) : data(value) {}
		
		Sym(
			std::function<T(Dependencies...)> fun, 
			std::tuple<Ref<GenericSym<Dependencies>>...> dependencies
		) : fun(fun), dependencies(dependencies) {
			check();
		}

		Sym(
			std::function<T(Dependencies...)> fun,
			IsSymbolicIgnoreCV auto&... dependencies
		) : fun(fun) {
			initDepList(dependencies...);
			check();
		}

		Sym(const Sym& other) : 
			data(other.data), 
			fun(other.fun), 
			dependencies(other.dependencies), 
			ownedList(other.ownedList) {
			check();
		}

		void check() {
			if (!(bool) fun.value()) {
				std::cout << "fun is empty" << std::endl;
				__debugbreak();
			}
		}

		void set(T value) requires (sizeof...(Dependencies) == 0) {
			expect(!fun.has_value(), "Cannot set symbolic object with construction method");
			data = value;
		}

		void compute() {
			if (fun.has_value()) {
				std::tuple<Dependencies...> values;

				auto toValues =
				[&]<size_t... indices>(std::index_sequence<indices...>) -> void {
					struct _ {};

					auto doCompute = []<typename T>(Ref<GenericSym<T>> sym) -> _ {
						sym.get().compute();
						return _();
					};

					std::array<_, sizeof...(Dependencies)>({ doCompute(std::get<indices>(dependencies))... });

					values = std::tuple<Dependencies...>(std::get<indices>(dependencies).get().get()...);
				};

				toValues(std::make_index_sequence<sizeof...(Dependencies)>());

				data = std::apply(fun.value(), values);
			}
		}

		T get() const {
			expect(data.has_value(), "cannot get value of symbolic object if value is not present");

			return data.value();
		}

		template<typename R, typename... Args>
		Sym<R, T> callMethod(const std::function<R(T, Args...)>& fun, Args... args) {
			return Sym<R, T>([](T val) -> R {
					return fun(val, args...);
				}, *this);
		}
	};
	
	auto SymAdd(IsSymbolicIgnoreCV auto& a, IsSymbolicIgnoreCV auto& b) 
		requires requires(GetSymbolicTypeInfo<decltype(a)>::Type a, GetSymbolicTypeInfo<decltype(b)>::Type b) { a + b; }
	{
		using A = GetSymbolicTypeInfo<decltype(a)>::Type;
		using B = GetSymbolicTypeInfo<decltype(b)>::Type;
		using R = decltype(A() + B());

		return Sym<R, A, B>([](A a, B b) -> R { return a + b; }, a, b);
	}

	template<typename B>
	auto SymAdd(IsSymbolicIgnoreCV auto& a, B b) 
		requires requires(typename GetSymbolicTypeInfo<decltype(a)>::Type a, B b) { a + b; } 
	{
		using A = GetSymbolicTypeInfo<decltype(a)>::Type;
		using R = decltype(A() + B());

		return Sym<R, A>([b](A a) -> R { return a + b; }, a);
	}
	
	template<typename A>
	auto SymAdd(A a, IsSymbolicIgnoreCV auto& b)
		requires requires(A a, typename GetSymbolicTypeInfo<decltype(b)>::Type b) { a + b; }
	{
		using B = GetSymbolicTypeInfo<decltype(b)>::Type;
		using R = decltype(A() + B());

		return Sym<R, B>([a](B b) -> R { return a + b; }, b);
	}

	auto operator+(IsSymbolicIgnoreCV auto& a, IsSymbolicIgnoreCV auto& b)
		requires requires(GetSymbolicTypeInfo<decltype(a)>::Type a, GetSymbolicTypeInfo<decltype(b)>::Type b) { a + b; }
	{
		return SymAdd(a, b);
	}

	template<typename B>
	auto operator+(IsSymbolicIgnoreCV auto& a, B b)
		requires requires(typename GetSymbolicTypeInfo<decltype(a)>::Type a, B b) { a + b; }
	{
		return SymAdd(a, b);
	}
	
	template<typename A>
	auto operator+(A a, IsSymbolicIgnoreCV auto& b)
		requires requires(A a, typename GetSymbolicTypeInfo<decltype(b)>::Type b) { a + b; }
	{
		return SymAdd(a, b);
	}



	auto operator+(IsSymbolicIgnoreCV auto&& a, IsSymbolicIgnoreCV auto&& b)
		requires requires(GetSymbolicTypeInfo<decltype(a)>::Type a, GetSymbolicTypeInfo<decltype(b)>::Type b) { a + b; }
	{
		return SymAdd(
			static_cast<const std::remove_cvref_t<decltype(a)>&>(a), 
			static_cast<const std::remove_cvref_t<decltype(b)>&>(b));
	}

	template<typename B>
	auto operator+(IsSymbolicIgnoreCV auto&& a, B b)
		requires requires(typename GetSymbolicTypeInfo<decltype(a)>::Type a, B b) { a + b; }
	{
		return SymAdd(static_cast<const std::remove_cvref_t<decltype(a)>&>(a), b);
	}

	template<typename A>
	auto operator+(A a, IsSymbolicIgnoreCV auto&& b)
		requires requires(A a, typename GetSymbolicTypeInfo<decltype(b)>::Type b) { a + b; }
	{
		return SymAdd(a, static_cast<const std::remove_cvref_t<decltype(b)>&>(b));
	}



	auto operator-(IsSymbolicIgnoreCV auto& x) {
		using X = GetSymbolicTypeInfo<decltype(x)>::Type;
		using R = decltype(-X());
		return Sym<R, X>([](X x) -> R { return -x; }, x);
	}

	auto operator-(IsSymbolicIgnoreCV auto&& x) {
		using X = GetSymbolicTypeInfo<decltype(x)>::Type;
		using R = decltype(-X());
		return Sym<R, X>([](X x) -> R { return -x; }, static_cast<const std::remove_cvref_t<decltype(x)>&>(x));
	}

	

	auto SymSub(IsSymbolicIgnoreCV auto& a, IsSymbolicIgnoreCV auto& b)
		requires requires(GetSymbolicTypeInfo<decltype(a)>::Type a, GetSymbolicTypeInfo<decltype(b)>::Type b) { a - b; }
	{
		using A = GetSymbolicTypeInfo<decltype(a)>::Type;
		using B = GetSymbolicTypeInfo<decltype(b)>::Type;
		using R = decltype(A() - B());

		return Sym<R, A, B>([](A a, B b) -> R { return a - b; }, a, b);
	}

	template<typename B>
	auto SymSub(IsSymbolicIgnoreCV auto& a, B b)
		requires requires(typename GetSymbolicTypeInfo<decltype(a)>::Type a, B b) { a - b; }
	{
		using A = GetSymbolicTypeInfo<decltype(a)>::Type;
		using R = decltype(A() - B());

		return Sym<R, A>([b](A a) -> R { return a - b; }, a);
	}

	template<typename A>
	auto SymSub(A a, IsSymbolicIgnoreCV auto& b)
		requires requires(A a, typename GetSymbolicTypeInfo<decltype(b)>::Type b) { a - b; }
	{
		using B = GetSymbolicTypeInfo<decltype(b)>::Type;
		using R = decltype(A() - B());

		return Sym<R, B>([a](B b) -> R { return a - b; }, b);
	}



	auto operator-(IsSymbolicIgnoreCV auto& a, IsSymbolicIgnoreCV auto& b)
		requires requires(GetSymbolicTypeInfo<decltype(a)>::Type a, GetSymbolicTypeInfo<decltype(b)>::Type b) { a - b; }
	{
		return SymSub(a, b);
	}

	template<typename B>
	auto operator-(IsSymbolicIgnoreCV auto& a, B b)
		requires requires(typename GetSymbolicTypeInfo<decltype(a)>::Type a, B b) { a - b; }
	{
		return SymSub(a, b);
	}

	template<typename A>
	auto operator-(A a, IsSymbolicIgnoreCV auto& b)
		requires requires(A a, typename GetSymbolicTypeInfo<decltype(b)>::Type b) { a - b; }
	{
		return SymSub(a, b);
	}



	auto operator-(IsSymbolicIgnoreCV auto&& a, IsSymbolicIgnoreCV auto&& b)
		requires requires(GetSymbolicTypeInfo<decltype(a)>::Type a, GetSymbolicTypeInfo<decltype(b)>::Type b) { a - b; }
	{
		return SymSub(
			static_cast<const std::remove_cvref_t<decltype(a)>&>(a), 
			static_cast<const std::remove_cvref_t<decltype(a)>&>(b)
		);
	}

	template<typename B>
	auto operator-(IsSymbolicIgnoreCV auto&& a, B b)
		requires requires(typename GetSymbolicTypeInfo<decltype(a)>::Type a, B b) { a - b; }
	{
		return SymSub(static_cast<const std::remove_cvref_t<decltype(a)>&>(a), b);
	}

	template<typename A>
	auto operator-(A a, IsSymbolicIgnoreCV auto&& b)
		requires requires(A a, typename GetSymbolicTypeInfo<decltype(b)>::Type b) { a - b; }
	{
		return SymSub(a, static_cast<const std::remove_cvref_t<decltype(b)>&>(b));
	}
	//
	//
	//
	//template<typename A, typename B, typename R = decltype(A() * B()), typename... ADep, typename... BDep>
	//	requires requires(A a, B b) { a * b; }
	//Sym<R, A, B> operator*(Sym<A, ADep...>& a, Sym<B, BDep...>& b) {
	//	return Sym<R, A, B>([](A a, B b) -> R { return a * b; }, a, b);
	//}
	//
	//template<typename A, typename B, typename R = decltype(A() * B()), typename... ADep>
	//	requires requires(A a, B b) { a * b; }
	//Sym<R, A> operator*(Sym<A, ADep...>& a, B b) {
	//	return Sym<R, A>([b](A a) -> R { return a * b; }, a);
	//}
	//
	//template<typename A, typename B, typename R = decltype(A() * B()), typename... BDep>
	//	requires requires(A a, B b) { a * b; }
	//Sym<R, B> operator*(A a, Sym<B, BDep...>& b) {
	//	return Sym<R, B>([a](B b) -> R { return a * b; }, b);
	//}
}