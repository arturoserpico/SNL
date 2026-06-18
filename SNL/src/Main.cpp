#include <iostream>
#include <functional>
#include <fstream>
#include <unordered_set>
#include <filesystem>
//#include <Eigen/Dense>
//#include <Eigen/Sparse>
//#include "Geometry/GridMesh2D.h"

#define SNLDebugLevel 2
//#define SNLObjectManagerDebugLogging true

#include "Symbolic/ExprOperators.h"
#include "Symbolic/Random.h"
//#include "Linear/Tensor.h"
#include "Utils/Any.h"
#include "Utils/Bounded.h"
#include "Symbolic/Sym.h"
#include "Symbolic/SymOperators.h"
#include "Metaprogramming/TypeList.h"
#include "Memory/ObjectManager.h"
#include "Utils/ErasedFunction.h"
#include "Symbolic/MathContext.h"
#include "Utils/Restricted.h"
#include "Utils/DebugName.h"
#include "Linear/Tensor.h"
#include "Symbolic/EGraph.h"
#include "Typing/TypeBase.h"

constexpr double PI = 3.14159265358979323846;

struct Nat : public snl::TypeBase<Nat> {
	static inline snl::Constructor<Nat()> zero;
	static inline snl::Constructor<Nat(Nat)> succ;

	Nat() = default;

	Nat(size_t n) {
		*this = zero;

		for (size_t i = 0; i < n; i++)
			*this = succ(*this);
	}

	int toInt() {
		return snl::match(*this,
			Nat::zero >> []() { return 0; },
			Nat::succ >> [](Nat n) { return n.toInt() + 1; }
		);
	}
};

template<typename T>
struct BinTree : public snl::TypeBase<BinTree<T>> {
	static inline const snl::Constructor<BinTree<T>(T)> leaf;
	static inline const snl::Constructor<BinTree<T>(T, BinTree<T>, BinTree<T>)> node;
};

std::string printTree(BinTree<int> tree) {
	return snl::match(tree,
		BinTree<int>::leaf >> [](int i) { return std::to_string(i); },
		BinTree<int>::node >> [](int i, BinTree<int> a, BinTree<int> b) { 
			return std::to_string(i) + "{" + printTree(a) + "," + printTree(b) + "}";
		}
	);
}



template<typename T>
struct Expr : public snl::TypeBase<Expr<T>> {
	static inline const snl::Constructor<Expr<T>(T)> val;
	static inline const snl::Constructor<Expr<T>(Expr<T>, Expr<T>)> add;
	static inline const snl::Constructor<Expr<T>(Expr<T>, Expr<T>)> mul;
};

template<typename T>
Expr<T> val(T val) {
	return Expr<T>::val(val);
}

template<typename T>
Expr<T> operator+(Expr<T> a, Expr<T> b) {
	return Expr<T>::add(a, b);
}

template<typename T>
Expr<T> operator*(Expr<T> a, Expr<T> b) {
	return Expr<T>::mul(a, b);
}

std::string printExpr(Expr<int> tree) {
	using T = Expr<int>;
	return snl::match(tree,
		T::val >> [](int i) { return std::to_string(i); },
		T::add >> [](T a, T b) {
			return "(" + printExpr(a) + "+" + printExpr(b) + ")";
		},
		T::mul >> [](T a, T b) {
			return "(" + printExpr(a) + "*" + printExpr(b) + ")";
		}
	);
}


int main() {
	//snl::breakOnThrow<snl::UnmanagedRefToManagedObjWarning>();

	BinTree<int> tree = BinTree<int>::node(3,
		BinTree<int>::node(4,
			BinTree<int>::leaf(12),
			BinTree<int>::leaf(13)
		),
		BinTree<int>::leaf(2)
	);

	auto expr = val(2) + val(3) * val(4);

	std::cout << printExpr(expr) << std::endl;

	Nat n = Nat::succ(Nat::succ(Nat::zero));

	std::cout << n.toInt() << std::endl;

	std::cout << n.constructor().id << std::endl;
}