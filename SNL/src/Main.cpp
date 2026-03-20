#include <iostream>
#include <functional>
#include <fstream>
#include <unordered_set>
#include <filesystem>
//#include <Eigen/Dense>
//#include <Eigen/Sparse>
//#include "Geometry/GridMesh2D.h"
#include "Symbolic/ExprOperators.h"
#include "Symbolic/Random.h"
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
	f(x, y) = x * 2 + y;

	snl::Index<2> i, j, k;

	snl::Vector<double, 2> v, w, c;
	snl::CoVector<double, 2> d;

	snl::Matrix11<double, 2> A;

	A(i, k) = f(i, k);

	A(0, 1) = 14;

	v(0) = 1;
	v(1) = 0;

	d(0) = 0;
	d(1) = 1;

	w(i) = snl::random<double>(0, 1);

	c = v + w;

	std::cout << w(0) << " " << w(1) << std::endl;

	//std::cout << x << std::endl;
}