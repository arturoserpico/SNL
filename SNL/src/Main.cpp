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
	snl::Index<2> i, j, k;

	snl::Vector<double, 2> v, w, c;
	snl::Matrix11<double, 2> A;
	snl::Tensor<double, 0, 3, 2, 2, 2> T;

	v(0) = 1;
	v(1) = 2;

	w(0) = 1;
	w(1) = -2;

	A(i, j) = v(i) * w(j);
	
	T(i, j, k) = A(i, j) * v(k);

	c(i) = snl::sum(j, k) | T(i, j, k) * A(j, k);

	for (size_t val = 0; val < 2; val++)
		std::cout << c(val) << std::endl;
}