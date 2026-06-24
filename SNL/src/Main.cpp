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

int main() {
	using namespace snl::literals;

	auto x = snl::matchVar<snl::Nat>();
	
	snl::Set set(snl::nat * snl::nat);

	set = { (x, snl::pow(x, 2)) };
	
	std::cout
		<< set.contains((1_nat, 1_nat)) << " "
		<< set.contains((2_nat, 4_nat)) << " "
		<< set.contains((3_nat, 9_nat));
}