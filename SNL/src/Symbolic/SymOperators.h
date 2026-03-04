#pragma once

#include "Sym.h"

#define SymBinOpDecl(OP, TYPE)\
template<typename A, typename B>\
auto operator##OP##(Ref<Sym<A>> a, Ref<Sym<B>> b) -> Sym<decltype(A() OP B())>\
	requires requires (A a, B b) { a OP b; }\
{\
	return Sym<decltype(A() + B())>(##TYPE##<A, B>(), a, b);\
}\
\
auto operator##OP##(IsSym auto& _a, IsSym auto& _b) {\
	auto [a, b] = binOperatorsWrapper(_a, _b);\
	return a OP b;\
}\
\
auto operator##OP##(IsSym auto& _a, const IsSym auto& _b) {\
	auto [a, b] = binOperatorsWrapper(_a, _b);\
	return a OP b;\
}\
\
auto operator##OP##(const IsSym auto& _a, IsSym auto& _b) {\
	auto [a, b] = binOperatorsWrapper(_a, _b);\
	return a OP b;\
}\
\
auto operator##OP##(const IsSym auto& _a, const IsSym auto& _b) {\
	auto [a, b] = binOperatorsWrapper(_a, _b);\
	return a OP b;\
}\
\
auto operator##OP##(IsSym auto& a, auto b) requires !IsSym<decltype(b)> {\
	return a OP Sym(b);\
}\
\
auto operator##OP##(const IsSym auto& a, auto b) requires !IsSym<decltype(b)> {\
	return a OP Sym(b);\
}\
\
auto operator##OP##(auto a, IsSym auto& b) requires !IsSym<decltype(a)> {\
	return Sym(a) OP b;\
}\
\
auto operator##OP##(auto a, const IsSym auto& b) requires !IsSym<decltype(a)> {\
		return Sym(a) OP b; \
}

namespace snl {
	auto binOperatorsWrapper(IsSym auto& a, IsSym auto& b) {
		using A = SymT<decltype(a)>;
		using B = SymT<decltype(b)>;
		return std::tuple<Ref<Sym<A>>, Ref<Sym<B>>>{ a, b };
	}

	auto binOperatorsWrapper(IsSym auto& a, const IsSym auto& b) {
		using A = SymT<decltype(a)>;
		using B = SymT<decltype(b)>;
		return std::tuple<Ref<Sym<A>>, Ref<Sym<B>>>{ a, makeManaged<Sym<B>>(b) };
	}

	auto binOperatorsWrapper(const IsSym auto& a, IsSym auto& b) {
		using A = SymT<decltype(a)>;
		using B = SymT<decltype(b)>;
		return std::tuple<Ref<Sym<A>>, Ref<Sym<B>>>{ makeManaged<Sym<A>>(a), b };
	}

	auto binOperatorsWrapper(const IsSym auto& a, const IsSym auto& b) {
		using A = SymT<decltype(a)>;
		using B = SymT<decltype(b)>;
		return std::tuple<Ref<Sym<A>>, Ref<Sym<B>>>{ makeManaged<Sym<A>>(a), makeManaged<Sym<B>>(b) };
	}

	SymBinOpDecl(+, SymAddOp)
	SymBinOpDecl(-, SymSubOp)
	SymBinOpDecl(*, SymMulOp)
	SymBinOpDecl(/, SymDivOp)
}