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

#include "Symbolic/MathContext.h"
#include "Typing/AlgebraicBase.h"
#include "Typing/AlgebraicTypes.h"
#include "Typing/Set.h"
#include "Symbolic/Sym.h"
#include "Symbolic/SymOperators.h"

int main() {
	using namespace snl::literals;

	std::cout << (2_nat * 2_nat == 4_nat) << std::endl;

	auto x = snl::matchVar<snl::Nat>();
	auto y = snl::matchVar<snl::Nat>();
	auto z = snl::matchVar<snl::Nat>();

	using F = snl::Set<snl::Pair<snl::Nat, snl::Nat>>;

	auto constructor = snl::Sym(snl::Pair<snl::Nat, snl::Nat>::tuple);

	F set = { constructor(x, x * x) };

	auto pattern = constructor(x, x * x);

	std::cout << snl::symMatch(pattern, snl::Pair(1_nat, 1_nat).sym()) << std::endl;

	std::cout << set.contains(snl::Pair(1_nat, 1_nat)) << " " << set.contains(snl::Pair(2_nat, 4_nat));

	//auto constructor = snl::Sym(snl::Pair<int, int>::tuple);
	//
	//auto pattern = constructor(x, x*x);
	//
	//auto target = constructor(3, y);
	//
	//snl::MatchResult varMap;
	//
	//std::cout << snl::symMatch(pattern, target, varMap) << std::endl;
	//
	//std::cout << "x: " << varMap.get(x) << std::endl;
	//std::cout << "y: " << varMap.get(y) << std::endl;
}