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
	auto x = snl::matchVar<int>();
	auto y = snl::matchVar<int>();

	auto constructor = snl::Sym(snl::Pair<int, int>::tuple);

	auto pattern = constructor(x, x*x);

	auto target = constructor(3, y);

	snl::MatchResult varMap;

	std::cout << snl::symMatch(pattern, target, varMap) << std::endl;

	std::cout << "x: " << varMap.get(x) << std::endl;
	std::cout << "y: " << varMap.get(y) << std::endl;
}