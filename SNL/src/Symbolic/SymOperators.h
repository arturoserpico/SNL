#pragma once

#include "Sym.h"
#include "Function.h"

namespace snl {
	auto binOperatorsWrapper(IsSym auto& a, IsSym auto& b) {
		using A = RemSym<decltype(a)>;
		using B = RemSym<decltype(b)>;
		return std::tuple<Ref<Sym<A>>, Ref<Sym<B>>>{ a, b };
	}

	auto binOperatorsWrapper(IsSym auto& a, const IsSym auto& b) {
		using A = RemSym<decltype(a)>;
		using B = RemSym<decltype(b)>;
		return std::tuple<Ref<Sym<A>>, Ref<Sym<B>>>{ a, makeManaged<Sym<B>>(b) };
	}

	auto binOperatorsWrapper(const IsSym auto& a, IsSym auto& b) {
		using A = RemSym<decltype(a)>;
		using B = RemSym<decltype(b)>;
		return std::tuple<Ref<Sym<A>>, Ref<Sym<B>>>{ makeManaged<Sym<A>>(a), b };
	}

	auto binOperatorsWrapper(const IsSym auto& a, const IsSym auto& b) {
		using A = RemSym<decltype(a)>;
		using B = RemSym<decltype(b)>;
		return std::tuple<Ref<Sym<A>>, Ref<Sym<B>>>{ makeManaged<Sym<A>>(a), makeManaged<Sym<B>>(b) };
	}

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