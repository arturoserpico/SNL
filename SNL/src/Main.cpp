#include <iostream>
#include <functional>
#include <fstream>
#include <unordered_set>
#include <filesystem>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include "Geometry/GridMesh2D.h"
#include "Linear/Tensor.h"
#include "Utils/Bounded.h"
#include "Symbolic/Sym.h"
#include "Symbolic/SymOperators.h"
#include "Symbolic/Function.h"
#include "Metaprogramming/TypeList.h"
#include "Memory/ObjectManager.h"

constexpr double PI = 3.14159265358979323846;

int main() {
	//snl::Sym<double> x, y, z;
	//
	//snl::Function<double(double)> interpolExp;
	//snl::Function<double(double)> exp;
	//exp(x) = snl::pow(2, x);
	//
	//std::ofstream points("output/Points.dat");
	//
	//for (double i = 0; i < 4.1; i += 0.5) {
	//	points << i << ' ' << pow(2, i) << '\n';
	//	interpolExp(i) = std::pow(2, i);
	//}
	//
	//snl::Function<double(double)> diff;
	//diff(x) = interpolExp(x) - exp(x);
	//
	//std::ofstream file("output/Plot.dat");
	//
	//for (double i = 0; i < 4; i += 0.01)
	//	file << i << ' ' << interpolExp(i) << '\n';

	snl::Index<3> i, j, k;
}