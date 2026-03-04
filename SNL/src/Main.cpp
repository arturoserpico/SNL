#include <iostream>
#include <functional>
#include <fstream>
#include <unordered_set>
#include <filesystem>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include "Geometry/GridMesh2D.h"
//#include "Linear/Tensor.h"
#include "Symbolic/Sym.h"
#include "Symbolic/SymOperators.h"
#include "Symbolic/Function.h"
#include "Metaprogramming/TypeList.h"
#include "Memory/ObjectManager.h"

constexpr double PI = 3.14159265358979323846;


int main() {
	snl::Sym<double> x, y, z;

	snl::Function<double(double, double)> f;
	f(x, y) = x * y + 2 * x;

	auto e = f(x, y);

	z = e;
	std::cout << &std::get<0>(f.variables) << std::endl;
	std::cout << f.expr.print() << std::endl;
	std::cout << z.print() << std::endl;

	e.inverseSubstituteAll<>(z);

	x.set(2);
	y.set(4);

	z.compute();

	std::cout << z.get() << std::endl;

	//snl::Function<double(double, double)> f;
	//
	//snl::Sym<double> x, y;
	//f(x, y) = x * y - (x + y);
	//
	//std::cout << f(2, 3) << std::endl;
}