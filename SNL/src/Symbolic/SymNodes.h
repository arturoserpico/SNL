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

	template<typename A, typename B>
	bool operator==(const SymAddOp<A, B>& a, const SymAddOp<A, B>& b) {
		return true;
	}

	template<typename A, typename B> requires requires (A a, B b) { a - b; }
	struct SymSubOp : SymOpType<decltype(std::declval<A>() - std::declval<B>())(A, B)> {
		decltype(std::declval<A>() - std::declval<B>()) eval(A a, B b) {
			return a - b;
		}
	};

	template<typename A, typename B>
	bool operator==(const SymSubOp<A, B>& a, const SymSubOp<A, B>& b) {
		return true;
	}

	template<typename A, typename B> requires requires (A a, B b) { a * b; }
	struct SymMulOp : SymOpType<decltype(std::declval<A>() + std::declval<B>())(A, B)> {
		decltype(std::declval<A>() * std::declval<B>()) eval(A a, B b) {
			return a * b;
		}
	};

	template<typename A, typename B>
	bool operator==(const SymMulOp<A, B>& a, const SymMulOp<A, B>& b) {
		return true;
	}

	template<typename A, typename B> requires requires (A a, B b) { a / b; }
	struct SymDivOp : SymOpType<decltype(std::declval<A>() / std::declval<B>())(A, B)> {
		decltype(std::declval<A>() / std::declval<B>()) eval(A a, B b) {
			return a / b;
		}
	};

	template<typename A, typename B>
	bool operator==(const SymDivOp<A, B>& a, const SymDivOp<A, B>& b) {
		return true;
	}

	template<typename A, typename B> requires requires (A a, B b) { std::pow(a, b); }
	struct SymPowOp : SymOpType<decltype(std::pow(std::declval<A>(), std::declval<B>()))(A, B)> {
		auto eval(A a, B b) {
			return std::pow(a, b);
		}
	};

	template<typename A, typename B>
	bool operator==(const SymPowOp<A, B>& a, const SymPowOp<A, B>& b) {
		return true;
	}

	template<typename T>
	struct SymIdentity : SymOpType<T(T)> {
		T eval(T val) {
			return val;
		}
	};

	template<typename T>
	bool operator==(const SymIdentity<T>& a, const SymIdentity<T>& b) {
		return true;
	}

	template<typename T, std::convertible_to<T> A>
	struct SymCast : SymOpType<T(A)> {
		T eval(A val) {
			return static_cast<T>(val);
		}
	};

	template<typename T, std::convertible_to<T> A>
	bool operator==(const SymCast<T, A>& a, const SymCast<T, A>& b) {
		return true;
	}

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

	template<typename Callable, typename ArgsList>
	bool operator==(const SymCall<Callable, ArgsList>& a, const SymCall<Callable, ArgsList>& b) {
		return true;
	}

	template<typename T> requires requires (T a) { -a; }
	struct SymNegOp : SymOpType<decltype(-std::declval<T>())(T)> {
		decltype(-std::declval<T>()) eval(T val) {
			return -val;
		}
	};

	template<typename T>
	bool operator==(const SymNegOp<T>& a, const SymNegOp<T>& b) {
		return true;
	}

	//template<typename A, typename B> requires requires(A a, B b) { a |= b; }
	//struct SymDeclaration : SymOpType<Empty(Ref<Sym<A>>, Ref<Sym<B>>)> {
	//	Empty eval(Ref<Sym<A>> a, Ref<Sym<B>> b) {
	//		a.get().computeGet() |= b.get();
	//		return {};
	//	}
	//};
}