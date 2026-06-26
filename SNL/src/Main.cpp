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

#include "Linear/Tensor.h"
#include "Symbolic/MathContext.h"
#include "Typing/AlgebraicBase.h"
#include "Typing/AlgebraicTypes.h"
#include "Typing/Set.h"
#include "Symbolic/Sym.h"
#include "Symbolic/SymOperators.h"
#include "Symbolic/ExprOperators.h"
#include "Typing/TypeOperations.h"
#include "Typing/Function.h"

int main() {
	using namespace snl::literals;

	auto x = snl::index<10>();
	auto y = snl::matchVar<int>();
	auto z = snl::matchVar<int>();
	auto i = snl::index<3>();

	auto a = snl::sum(x) | x.cast<int>();
	auto b = snl::sum(y, 3, 7) | 2 * y;

	auto f = y >> ( snl::sum(z, snl::Sym(0), y) | 2 * z );

	auto c = snl::sum(x) | (snl::sum(z, snl::Sym(0), x.cast<int>()) | x.cast<int>() * 2);

	std::cout << f(7) << std::endl;
	std::cout << a << std::endl << b << std::endl << c << std::endl;
}