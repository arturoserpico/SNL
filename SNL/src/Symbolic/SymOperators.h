#pragma once

#include "Sym.h"
#include "SymUtils.h"

namespace snl {
	auto operator+(auto&& _a, auto&& _b) {
		auto [a, b] = convertArgsToSymRef(std::forward<decltype(_a)>(_a), std::forward<decltype(_b)>(_b));
		using A = RemSym<RemRef<decltype(a)>>;
		using B = RemSym<RemRef<decltype(b)>>;
		return Sym(SymAddOp<A, B>(), a, b);
	}

	auto operator-(auto&& _a, auto&& _b) {
		auto [a, b] = convertArgsToSymRef(std::forward<decltype(_a)>(_a), std::forward<decltype(_b)>(_b));
		using A = RemSym<RemRef<decltype(a)>>;
		using B = RemSym<RemRef<decltype(b)>>;
		return Sym(SymSubOp<A, B>(), a, b);
	}

	auto operator*(auto&& _a, auto&& _b) {
		auto [a, b] = convertArgsToSymRef(std::forward<decltype(_a)>(_a), std::forward<decltype(_b)>(_b));
		using A = RemSym<RemRef<decltype(a)>>;
		using B = RemSym<RemRef<decltype(b)>>;
		return Sym(SymMulOp<A, B>(), a, b);
	}

	auto operator/(auto&& _a, auto&& _b) {
		auto [a, b] = convertArgsToSymRef(std::forward<decltype(_a)>(_a), std::forward<decltype(_b)>(_b));
		using A = RemSym<RemRef<decltype(a)>>;
		using B = RemSym<RemRef<decltype(b)>>;
		return Sym(SymDivOp<A, B>(), a, b);
	}

	auto pow(auto&& _a, auto&& _b) {
		auto [a, b] = convertArgsToSymRef(std::forward<decltype(_a)>(_a), std::forward<decltype(_b)>(_b));
		using A = RemSym<RemRef<decltype(a)>>;
		using B = RemSym<RemRef<decltype(b)>>;
		return Sym(SymPowOp<A, B>(), a, b);
	}
}