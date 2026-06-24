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
	auto y = snl::matchVar<snl::Nat>();
	
	snl::Set set(snl::nat * snl::nat);

	set = { (x, snl::pow(x, 2_nat)) };
	
	std::cout
		<< set.contains((1_nat, 1_nat)) << " " //true 1^2 == 1
		<< set.contains((2_nat, 4_nat)) << " " //true 2^2 == 4 
		<< set.contains((2_nat, 5_nat)) << " " //false
		<< set.contains((3_nat, 9_nat)) << " " //true 3^2 == 9
		<< set.contains((7_nat, y)); //true y binds to 49
}