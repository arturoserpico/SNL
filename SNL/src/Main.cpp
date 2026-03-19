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
	snl::Matrix11<double, 2> t;

	v(0) = 14;
	v(1) = 2;

	w(0) = 43;
	w(1) = -2;

	t(i, j) = v(i) * w(j);
	
	double sum1 = snl::sum(i, j) | v(i) * w(j);
	double sum2 = snl::sum(i, j) | t(i, j);

	std::cout << (sum1 == sum2) << std::endl;

	//std::cout << t(1, 0) << std::endl; 
}