#include <iostream>
#include <functional>
#include <fstream>
#include <unordered_set>
#include <filesystem>
//#include <Eigen/Dense>
//#include <Eigen/Sparse>
//#include "Geometry/GridMesh2D.h"
#include "Symbolic/ExprOperators.h"
#include "Linear/Tensor.h"
#include "Utils/Bounded.h"
#include "Symbolic/Sym.h"
#include "Symbolic/SymOperators.h"
#include "Symbolic/Function.h"
#include "Metaprogramming/TypeList.h"
#include "Memory/ObjectManager.h"

constexpr double PI = 3.14159265358979323846;

int main() {
	snl::Sym<double> x, y;

	snl::Function<double(double, double)> f;
	f(x, y) = x * y;

	std::cout << f(1, 2) << std::endl;

	snl::Index<2> i, j, k;

	snl::Vector<double, 2> v, w;

	v(i) = f(i, 1);
	w(i) = f(i, 2);

	double dot = snl::sum(i) | v(i) * w(i);

	std::cout << dot << std::endl;
}