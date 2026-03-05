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

	snl::Function<double(double)> g;
	g(x) = 2 * x;

	z = g(y) * 2 + f(x, y);

	x.set(2);
	y.set(3);

	z.compute();

	std::cout << z.get() << std::endl;
}