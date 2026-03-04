#pragma once
#include <type_traits>

namespace snl {
	template<typename Fn>
	struct SymOpType;

	template<typename R, typename... Args>
	struct SymOpType<R(Args...)> {
		virtual R eval(Args...) = 0;
	};

	template<typename A, typename B>
	struct SymAddOp : SymOpType<decltype(A() + B())(A, B)> {
		decltype(A() + B()) eval(A a, B b) {
			return a + b;
		}
	};

	template<typename A, typename B>
	struct SymSubOp : SymOpType<decltype(A() - B())(A, B)> {
		decltype(A() - B()) eval(A a, B b) {
			return a - b;
		}
	};

	template<typename A, typename B>
	struct SymMulOp : SymOpType<decltype(A() * B())(A, B)> {
		decltype(A() * B()) eval(A a, B b) {
			return a * b;
		}
	};

	template<typename A, typename B>
	struct SymDivOp : SymOpType<decltype(A() / B())(A, B)> {
		decltype(A() / B()) eval(A a, B b) {
			return a / b;
		}
	};
}