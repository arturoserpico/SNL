#include <iostream>
#include <functional>
#include <fstream>
#include <unordered_set>
#include <filesystem>
//#include <Eigen/Dense>
//#include <Eigen/Sparse>
//#include "Geometry/GridMesh2D.h"

#define SNLDebugLevel 2
//#define SNLObjectManagerDebugLogging true

#include "Symbolic/ExprOperators.h"
#include "Symbolic/Random.h"
//#include "Linear/Tensor.h"
#include "Utils/Any.h"
#include "Utils/Bounded.h"
#include "Symbolic/Sym.h"
#include "Symbolic/SymOperators.h"
#include "Metaprogramming/TypeList.h"
#include "Memory/ObjectManager.h"
#include "Utils/ErasedFunction.h"
#include "Symbolic/MathContext.h"
#include "Utils/Restricted.h"
#include "Utils/DebugName.h"

constexpr double PI = 3.14159265358979323846;

int main() {
	snl::breakOnThrow<snl::UnmanagedRefToManagedObjWarning>();

	snl::Sym<int> n;
	snl::Sym<double> x, y, z;
	snl::Sym<double(*)(double)> f;

	snl::addDebugName(f, "test");

	std::cout << snl::getDebugName(f) << std::endl;

	snl::defineRule(x - x, snl::Sym<double>(0), x);
	snl::defineRule(x, 1 * x, x);
	snl::defineRule(n * x + x, (n + 1) * x);
	snl::defineRule(y + x, x + y, x, y);
	snl::defineRule((x + y) + z, x + (y + z), x, y, z);
	snl::defineRule(x + y, snl::Sym<double>(2));
	snl::defineRule(f(x), snl::pow(x, 2), x);
	snl::defineRule(x - y, x + (-y), x, y);
	snl::defineRule(x + (-y), x - y, x, y);


	//std::cout << snl::objManager.count() << std::endl;
	//
	//for(auto [location, count] : snl::objManager.getObjects())
	//	std::cout << location << ": " << count.second << std::endl;

	//snl::Index<100> i, j, k;
	//
	//snl::Vector<double, 100> v, w, c;
	//
	//snl::Matrix11<double, 100> A;
	//
	//A(i, k) |= snl::random<double>(0, 1);
	//
	//w(i) |= snl::random<double>(0, 1);
	//
	//auto begin = std::chrono::high_resolution_clock::now();
	//c(i) |= snl::sum(j) | A(i, j) * w(j);
	//auto end = std::chrono::high_resolution_clock::now();
	//
	//std::cout << c << std::endl;
	//std::cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "ms" << std::endl;
	//
	//Eigen::VectorXd eigenV = Eigen::VectorXd::Random(100);
	//Eigen::VectorXd eigenC = Eigen::VectorXd::Random(100);
	//Eigen::MatrixXd eigenA = Eigen::MatrixXd::Random(100, 100);
	//
	//begin = std::chrono::high_resolution_clock::now();
	//eigenC = eigenA * eigenV;
	//end = std::chrono::high_resolution_clock::now();
	//
	//std::cout << eigenC << std::endl;
	//std::cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "ms" << std::endl;


	//std::cout << x << std::endl;
}