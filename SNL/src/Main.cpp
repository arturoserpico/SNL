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
#include "Metaprogramming/TypeList.h"
#include "Memory/ObjectManager.h"

constexpr double PI = 3.14159265358979323846;


int main() {
	snl::Sym<double> x;
	snl::Sym<double> z;
	snl::Sym<double> k;

	snl::Sym<double> y = 2 + (x + 3) * x;
	snl::Sym<double> a = y.dep();

	y.substitute(x, z);

	z.set(2);

	y.compute();

	a.compute();

	std::cout << a.get() << std::endl;
}