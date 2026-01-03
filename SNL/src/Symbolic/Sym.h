#pragma once

#include <optional>
#include <vector>
#include <functional>
#include "../Utils/Ref.h"
#include "../Utils/Error.h"

namespace snl {
	template<typename T>
	class GenericSym {
	public:
		virtual void compute() = 0;
		virtual T get() const = 0;
	};

	template<typename T, typename... Dependencies>
	class Sym : public GenericSym<T> {
		std::optional<T> data;
		std::optional<std::function<T(Dependencies...)>> fun;
		std::tuple<Ref<GenericSym<Dependencies>>...> dependencies;
		std::vector<void*> ownedList;
	public:
		Sym() requires (sizeof...(Dependencies) == 0) = default;
		Sym(T value) requires (sizeof...(Dependencies) == 0) : data(value) {}
		
		Sym(
			std::function<T(Dependencies...)> fun, 
			std::tuple<Ref<GenericSym<Dependencies>>...> dependencies
		) : fun(fun), dependencies(dependencies) {}

		Sym(
			std::function<T(Dependencies...)> fun,
			GenericSym<Dependencies>&... dependencies
		) : fun(fun), dependencies(Ref(dependencies)...) {}

		~Sym() {
			for (void* ptr : ownedList)
				delete ptr;
		}

		template<typename... Ts>
		void addOwnedSym(Sym<Ts...>& owned) {
			ownedList.push_back(owned);
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



	template<typename T>
	constexpr bool _isSymbolic = false;

	template<typename... Ts>
	constexpr bool _isSymbolic<Sym<Ts...>> = true;

	template<typename T>
	constexpr bool isSymbolic = _isSymbolic<std::remove_cvref_t<T>>;
	


	template<typename A, typename B, typename R = decltype(A() + B()), typename... ADep, typename... BDep>
		requires requires(A a, B b) { a + b; }
	Sym<R, A, B> operator+(Sym<A, ADep...>& a, Sym<B, BDep...>& b) {
		return Sym<R, A, B>([](A a, B b) -> R { return a + b; }, a, b);
	}

	template<typename A, typename B, typename R = decltype(A() + B()), typename... ADep>
		requires requires(A a, B b) { a + b; }
	Sym<R, A> operator+(Sym<A, ADep...>& a, B b) {
		return Sym<R, A>([b](A a) -> R { return a + b; }, a);
	}

	template<typename A, typename B, typename R = decltype(A() + B()), typename... BDep>
		requires requires(A a, B b) { a + b; }
	Sym<R, B> operator+(A a, Sym<B, BDep...>& b) {
		return Sym<R, B>([a](B b) -> R { return a + b; }, b);
	}



	template<typename A, typename B, typename R = decltype(A() * B()), typename... ADep, typename... BDep>
		requires requires(A a, B b) { a * b; }
	Sym<R, A, B> operator*(Sym<A, ADep...>& a, Sym<B, BDep...>& b) {
		return Sym<R, A, B>([](A a, B b) -> R { return a * b; }, a, b);
	}

	template<typename A, typename B, typename R = decltype(A() * B()), typename... ADep>
		requires requires(A a, B b) { a * b; }
	Sym<R, A> operator*(Sym<A, ADep...>& a, B b) {
		return Sym<R, A>([b](A a) -> R { return a * b; }, a);
	}

	template<typename A, typename B, typename R = decltype(A() * B()), typename... BDep>
		requires requires(A a, B b) { a * b; }
	Sym<R, B> operator*(A a, Sym<B, BDep...>& b) {
		return Sym<R, B>([a](B b) -> R { return a * b; }, b);
	}
}