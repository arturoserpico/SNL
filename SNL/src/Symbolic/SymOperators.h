#pragma once

#include "Sym.h"

#define SymBinOpDecl(OP)\
template<typename A, typename B>\
auto operator##OP##(Ref<Sym<A>> a, Ref<Sym<B>> b) -> Sym<decltype(A() + B())>\
	requires requires (A a, B b) { a + b; }\
{\
	return Sym<decltype(A() + B())>(std::function([](A a, B b) {\
		return a OP b;\
		}), a, b);\
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

	SymBinOpDecl(+)
	SymBinOpDecl(-)
	SymBinOpDecl(*)
	SymBinOpDecl(/)
}