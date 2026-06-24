#pragma once

#include "SymNodes.h"
#include "Sym.h"
#include "SymUtils.h"

namespace snl {
	auto operator+(auto&& _a, auto&& _b) requires IsSymCall<decltype(_a), decltype(_b)> {
		auto [a, b] = convertArgsToSymRef(std::forward<decltype(_a)>(_a), std::forward<decltype(_b)>(_b));
		using A = RemSym<decltype(a)>;
		using B = RemSym<decltype(b)>;
		return Sym(SymAddOp<A, B>(), a, b);
	}

	auto operator-(auto&& _a, auto&& _b) requires IsSymCall<decltype(_a), decltype(_b)> {
		auto [a, b] = convertArgsToSymRef(std::forward<decltype(_a)>(_a), std::forward<decltype(_b)>(_b));
		using A = RemSym<decltype(a)>;
		using B = RemSym<decltype(b)>;
		return Sym(SymSubOp<A, B>(), a, b);
	}

	auto operator*(auto&& _a, auto&& _b) requires IsSymCall<decltype(_a), decltype(_b)> {
		auto [a, b] = convertArgsToSymRef(std::forward<decltype(_a)>(_a), std::forward<decltype(_b)>(_b));
		using A = RemSym<decltype(a)>;
		using B = RemSym<decltype(b)>;
		return Sym(SymMulOp<A, B>(), a, b);
	}

	auto operator/(auto&& _a, auto&& _b) requires IsSymCall<decltype(_a), decltype(_b)> {
		auto [a, b] = convertArgsToSymRef(std::forward<decltype(_a)>(_a), std::forward<decltype(_b)>(_b));
		using A = RemSym<decltype(a)>;
		using B = RemSym<decltype(b)>;
		return Sym(SymDivOp<A, B>(), a, b);
	}

	auto pow(auto&& _a, auto&& _b) requires IsSymCall<decltype(_a), decltype(_b)> {
		auto [a, b] = convertArgsToSymRef(std::forward<decltype(_a)>(_a), std::forward<decltype(_b)>(_b));
		using A = RemSym<decltype(a)>;
		using B = RemSym<decltype(b)>;
		return Sym(SymPowOp<A, B>(), a, b);
	}

	template<typename T>
	auto Sym<T>::operator()(auto&&... args) const {
		using TargetTs = If<std::is_pointer_v<T>&& std::is_function_v<std::remove_pointer_t<T>>, typename FunctionTypeInfo<T>::ArgsList, TypeList<>>;

		auto tuple = convertArgsToSymRef<TargetTs>(std::forward<decltype(args)>(args)...);
		using SymArgsList = Transform<TupleToTypeList<decltype(tuple)>, RemSym>;
		return Sym<typename SymCall<T, SymArgsList>::R>(SymCall<T, SymArgsList>(), std::tuple_cat(std::make_tuple(*this), tuple));
	}

	template<typename T>
	auto Sym<T>::operator-() const {
		return Sym(SymNegOp<T>(), *this);
	}

	//template<typename T>
	//auto Sym<T>::operator|=(auto&& _decl) & {
	//	auto [decl] = convertArgsToSymRef(std::forward<decltype(_decl)>(_decl));
	//	using Decl = RemSym<RemRef<decltype(decl)>>;
	//	return Sym<Empty>(SymDeclaration<T, Decl>(), Ref(*this), decl);
	//}
	//
	//template<typename T>
	//auto Sym<T>::operator|=(auto&& _decl) && {
	//	auto [decl] = convertArgsToSymRef(std::forward<decltype(_decl)>(_decl));
	//	using Decl = RemSym<RemRef<decltype(decl)>>;
	//	return Sym<Empty>(SymDeclaration<T, Decl>(), makeManaged(*this), decl);
	//}
}