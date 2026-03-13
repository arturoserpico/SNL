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
	snl::Sym<double> x, y;

	snl::Function<double(double)> f;
	f(x) = x * 2 + x;

	x.set(4);

	std::cout << f(1).sym().occurences(x) << std::endl;

	snl::Sym<double> z = f(5);

	//std::cout << z.occurences(x) << std::endl;

	//std::cout << "ci siamo" << std::endl;
	//z.substitute(x, snl::Sym<double>(1.));

	std::cout << f(1).sym().compute().get() << std::endl;
	std::cout << z.compute().get() << std::endl;
}