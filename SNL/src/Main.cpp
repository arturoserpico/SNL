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
#include "Typing/TypeOperations.h"
#include "Typing/Function.h"

int main() {
	using namespace snl::literals;

	auto x = snl::matchVar<snl::Nat>();
	auto y = snl::matchVar<snl::Nat>();

	snl::Sym f(snl::nat * snl::nat >> snl::nat);

	f = (x, y) >> snl::pow(x, 2_nat) + 3_nat * y;

	std::cout << f(2_nat, 1_nat) << std::endl;
}