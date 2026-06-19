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

#include "Typing/Set.h"
#include "Typing/Types.h"

int main() {
	auto x = snl::matchVar<double>();

	snl::Sym<double> y, c(12);

	snl::defineRule(x + y, x + 1);

	snl::Sym<double> expr = c + y;

	//std::cout << expr << std::endl;

	std::cout << snl::mathContext.getRules().match(expr) << std::endl;

	expr = snl::mathContext.getRules().apply(expr);

	std::cout << expr << std::endl;

	using namespace snl::literals;

	auto n = 13_nat;

	std::cout << n + 5_nat << std::endl;

	using U = snl::Union<int, std::string, char>;

	U u = U::constructor<int>(2);

	std::string str = u.match(
		[](int i) { return std::to_string(i); },
		[](std::string s) { return std::string("string"); },
		[](char c) { return std::string("char"); }
	);

	std::cout << str << std::endl;
}