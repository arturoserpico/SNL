#pragma once

#include "SymNodes.h"
#include "Sym.h"
#include "SymUtils.h"
#include "Function.h"

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
	auto Sym<T>::operator()(auto&&... args) & {
		auto tuple = convertArgsToSymRef(std::forward<decltype(args)>(args)...);
		using SymArgsList = Transform<Transform<TupleToTypeList<decltype(tuple)>, RemRef>, RemSym>;
		return Sym<typename SymCall<T, SymArgsList>::R>(SymCall<T, SymArgsList>(), std::tuple_cat(std::make_tuple(Ref(*this)), tuple));
	}

	template<typename T>
	auto Sym<T>::operator()(auto&&... args) && {
		auto tuple = convertArgsToSymRef(std::forward<decltype(args)>(args)...);
		using SymArgsList = Transform<Transform<TupleToTypeList<decltype(tuple)>, RemRef>, RemSym>;
		return Sym<typename SymCall<T, SymArgsList>::R>(SymCall<T, SymArgsList>(), std::tuple_cat(std::make_tuple(makeManaged(*this)), tuple));
	}

	template<typename T>
	void Sym<T>::operator|=(auto&& _decl) & {
		auto [decl] = convertArgsToSymRef(std::forward<decltype(_decl)>(_decl));
		using Decl = RemSym<RemRef<decltype(decl)>>;
		Sym<Empty>(SymDeclaration<T, Decl>(), Ref(*this), decl).compute();
	}

	template<typename T>
	void Sym<T>::operator|=(auto&& _decl) && {
		auto [decl] = convertArgsToSymRef(std::forward<decltype(_decl)>(_decl));
		using Decl = RemSym<RemRef<decltype(decl)>>;
		Sym<Empty>(SymDeclaration<T, Decl>(), makeManaged(*this), decl).compute();
	}
}