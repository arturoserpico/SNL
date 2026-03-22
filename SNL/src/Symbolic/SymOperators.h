#pragma once

#include "SymNodes.h"
#include "Sym.h"
#include "SymUtils.h"

namespace snl {
	auto operator+(auto&& _a, auto&& _b) requires IsSymCall<decltype(_a), decltype(_b)> {
		auto [a, b] = convertArgsToSymRef(std::forward<decltype(_a)>(_a), std::forward<decltype(_b)>(_b));
		using A = RemSym<RemRef<decltype(a)>>;
		using B = RemSym<RemRef<decltype(b)>>;
		return Sym(SymAddOp<A, B>(), a, b);
	}

	auto operator-(auto&& _a, auto&& _b) requires IsSymCall<decltype(_a), decltype(_b)> {
		auto [a, b] = convertArgsToSymRef(std::forward<decltype(_a)>(_a), std::forward<decltype(_b)>(_b));
		using A = RemSym<RemRef<decltype(a)>>;
		using B = RemSym<RemRef<decltype(b)>>;
		return Sym(SymSubOp<A, B>(), a, b);
	}

	auto operator*(auto&& _a, auto&& _b) requires IsSymCall<decltype(_a), decltype(_b)> {
		auto [a, b] = convertArgsToSymRef(std::forward<decltype(_a)>(_a), std::forward<decltype(_b)>(_b));
		using A = RemSym<RemRef<decltype(a)>>;
		using B = RemSym<RemRef<decltype(b)>>;
		return Sym(SymMulOp<A, B>(), a, b);
	}

	auto operator/(auto&& _a, auto&& _b) requires IsSymCall<decltype(_a), decltype(_b)> {
		auto [a, b] = convertArgsToSymRef(std::forward<decltype(_a)>(_a), std::forward<decltype(_b)>(_b));
		using A = RemSym<RemRef<decltype(a)>>;
		using B = RemSym<RemRef<decltype(b)>>;
		return Sym(SymDivOp<A, B>(), a, b);
	}

	auto pow(auto&& _a, auto&& _b) requires IsSymCall<decltype(_a), decltype(_b)> {
		auto [a, b] = convertArgsToSymRef(std::forward<decltype(_a)>(_a), std::forward<decltype(_b)>(_b));
		using A = RemSym<RemRef<decltype(a)>>;
		using B = RemSym<RemRef<decltype(b)>>;
		return Sym(SymPowOp<A, B>(), a, b);
	}

	template<typename T>
	auto Sym<T>::operator()(auto&&... args) {
		auto tuple = convertArgsToSymRef(std::forward<decltype(args)>(args)...);
		using SymArgsList = Transform<Transform<TupleToTypeList<decltype(tuple)>, RemRef>, RemSym>;
		std::cout << typeid(SymArgsList).name() << std::endl;
		//return 0;
		auto coso = std::tuple_cat(std::make_tuple(makeManaged(*this)), tuple);

		std::cout << typeid(decltype(coso)).name() << std::endl;


		std::cout << typeid(SymCall<T, SymArgsList>).name() << std::endl;

		return 0;
		//return Sym(SymCall<T, SymArgsList>(), coso);
	}
}