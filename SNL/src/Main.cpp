#include <iostream>
#include <functional>
#include <fstream>
#include <unordered_set>
#include <filesystem>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include "GridMesh2D.h"

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

	snl::GridMesh2D mesh(1, 1, 3, 3);

	std::unordered_set<snl::Ref<snl::Face2D>> faces = mesh.faces();

	snl::Face2D& face = *faces.begin();

 	std::cout << face.elements<0>().size() << std::endl;

	//std::ofstream positions("output\\Positions.dat");
	//std::ofstream edges("output\\Edges.dat");
	//std::ofstream faces("output\\Faces.dat");

	//mesh.print(positions, edges, faces);

	//std::cout << grid.nodesMatrix << std::endl;
	//std::cout << grid.linkMatrix << std::endl
}