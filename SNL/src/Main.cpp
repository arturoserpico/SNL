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

	snl::Function<double(double)> f;
	f(x) = x + 1;

	snl::Function<double(double, double)> g;
	g(x, y) = snl::pow(2, x) + y;

	x.set(4);

	z = g(x, x) + 3;

	std::cout << z << std::endl;
}