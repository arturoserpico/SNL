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
#include "Metaprogramming/TypeList.h"

constexpr double PI = 3.14159265358979323846;

int main() {
	////Eigen::setNbThreads(24);
	//
	//size_t numNodes = 50;
	//
	//Eigen::VectorXd up = Eigen::VectorXd::Zero(numNodes);
	//
	//for (size_t i = 0; i < up.size(); i++) {
	//	up(i) = std::sin(PI * i / up.size() * 2);
	//}
	//
	//HeatTransfer2D sim(
	//	1, 
	//	numNodes, 
	//	up,
	//	-up, 
	//	Eigen::VectorXd::Constant(numNodes, 0), 
	//	Eigen::VectorXd::Constant(numNodes, 0)
	//);
	//
	//std::cout << sim.T << std::endl;
	//
	//std::ofstream file("output\\Data.dat");
	//
	//file << sim.T;

	//snl::Mesh<2> mesh(
	//	{
	//		{0, 0}, {1, 0}, {0, 1}, {1, 1}
	//	},
	//	{
	//		{ {0, 1}, {0, 2}, {1, 2}, {3, 1}, {3, 2} },
	//		{ {0, 1, 2}, {2, 3, 4} }
	//	}
	//);


	//std::cout << edge.lenght() << std::endl
	//for (std::reference_wrapper<int> ref : test) {
	//	std::cout << ref << std::endl;
	//}

	//snl::Tensor<double, 2, 1, 2, 2, 2> t;

	//snl::Sym<double> x;
	//
	//snl::Sym<double, double> y = x + 2;
	//
	//x.set(5);
	//
	//y.compute();
	//
	//std::cout << y.get() << std::endl;

	snl::Sym<double> x;

	snl::Sym<double> z;

	snl::Sym<double, double> y = 1 / (x + 1);

	y.substitute(x, z);

	z.set(2);

	y.compute();

	std::cout << y.get();

	snl::GridMesh2D mesh(10, 5, 10, 5);

	//snl::Region2D manifold = mesh.manifold();

	snl::Edge2D& edge = mesh.findElementByBoundary<1>({ mesh.nodeAtPosition(1, 1), mesh.nodeAtPosition(2, 1) });

	snl::FaceComplex2D connected = mesh.findElementsByBoundary<2>(edge);

	size_t count = connected.elements<2>().size() + connected.elements<1>().size() - connected.elements<0>().size();

	//snl::Area2D chain = mesh.chain();

	//std::cout << chain.boundary().elements().size() << std::endl;

	std::ofstream positionsPrint("output\\Positions.dat");
	std::ofstream edgesPrint("output\\Edges.dat");
	std::ofstream facesPrint("output\\Faces.dat");

	mesh.print(positionsPrint, edgesPrint, facesPrint);

	//std::cout << grid.nodesMatrix << std::endl;
	//std::cout << grid.linkMatrix << std::endl
}