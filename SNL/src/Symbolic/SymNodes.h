#pragma once
#include <type_traits>

namespace snl {
	template<typename Fn>
	struct SymOpType;

	template<typename _R, typename... Args>
	struct SymOpType<_R(Args...)> {
		using R = _R;
		using ArgsList = TypeList<Args...>;

		virtual R eval(Args...) = 0;
	};

	template<typename A, typename B> requires requires (A a, B b) { a + b; }
	struct SymAddOp : SymOpType<decltype(A() + B())(A, B)> {
		decltype(A() + B()) eval(A a, B b) {
			return a + b;
		}
	};

	template<typename A, typename B> requires requires (A a, B b) { a - b; }
	struct SymSubOp : SymOpType<decltype(A() - B())(A, B)> {
		decltype(A() - B()) eval(A a, B b) {
			return a - b;
		}
	};

	template<typename A, typename B> requires requires (A a, B b) { a * b; }
	struct SymMulOp : SymOpType<decltype(A() * B())(A, B)> {
		decltype(A() * B()) eval(A a, B b) {
			return a * b;
		}
	};

	template<typename A, typename B> requires requires (A a, B b) { a / b; }
	struct SymDivOp : SymOpType<decltype(A() / B())(A, B)> {
		decltype(A() / B()) eval(A a, B b) {
			return a / b;
		}
	};

	template<typename A, typename B> requires requires (A a, B b) { std::pow(a, b); }
	struct SymPowOp : SymOpType<decltype(std::pow(A(), B()))(A, B)> {
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
}