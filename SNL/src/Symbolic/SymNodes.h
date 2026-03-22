#pragma once
#include <type_traits>
#include "Sym.h"

namespace snl {
	template<typename A, typename B> requires requires (A a, B b) { a + b; }
	struct SymAddOp : SymOpType<decltype(std::declval<A>() + std::declval<B>())(A, B)> {
		decltype(std::declval<A>() + std::declval<B>()) eval(A a, B b) {
			return a + b;
		}
	};

	template<typename A, typename B> requires requires (A a, B b) { a - b; }
	struct SymSubOp : SymOpType<decltype(std::declval<A>() - std::declval<B>())(A, B)> {
		decltype(std::declval<A>() - std::declval<B>()) eval(A a, B b) {
			return a - b;
		}
	};

	template<typename A, typename B> requires requires (A a, B b) { a * b; }
	struct SymMulOp : SymOpType<decltype(std::declval<A>() + std::declval<B>())(A, B)> {
		decltype(std::declval<A>() * std::declval<B>()) eval(A a, B b) {
			return a * b;
		}
	};

	template<typename A, typename B> requires requires (A a, B b) { a / b; }
	struct SymDivOp : SymOpType<decltype(std::declval<A>() / std::declval<B>())(A, B)> {
		decltype(std::declval<A>() / std::declval<B>()) eval(A a, B b) {
			return a / b;
		}
	};

	template<typename A, typename B> requires requires (A a, B b) { std::pow(a, b); }
	struct SymPowOp : SymOpType<decltype(std::pow(std::declval<A>(), std::declval<B>()))(A, B)> {
		auto eval(A a, B b) {
			return std::pow(a, b);
		}
	};

	template<typename T>
	struct SymIdentity : SymOpType<T(T)> {
		T eval(T val) {
			return val;
		}
	};

	template<typename T, std::convertible_to<T> A>
	struct SymCast : SymOpType<T(A)> {
		T eval(A val) {
			return static_cast<T>(val);
		}
	};

	template<typename Callable, typename ArgsList>
	struct SymCall;

	template<typename Callable, typename... Args> requires std::invocable<Callable, Sym<Args>&...>
	struct SymCall<Callable, TypeList<Args...>> {
		auto eval(Ref<Sym<Callable>> f, Ref<Sym<Args>>... args) {
			return f.get().computeGet()(args.get()...);
		}

		using ArgsList = TypeList<Ref<Sym<Callable>>, Ref<Sym<Args>>...>;
		using R = decltype(std::declval<Callable>()(std::declval<Sym<Args>>()...));
	};

	template<typename A, typename B> requires requires(A a, B b) { a |= b; }
	struct SymDeclaration : SymOpType<Empty(Ref<Sym<A>>, Ref<Sym<B>>)> {
		Empty eval(Ref<Sym<A>> a, Ref<Sym<B>> b) {
			a.get().computeGet() |= b.get();
			return {};
		}
	};
}