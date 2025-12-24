#pragma once

#include "Mesh.h"

namespace snl {
	class GridMesh2D : public Mesh2D {
		std::map<std::pair<size_t, size_t>, Ref<Node2D>> gridAccessor;
	public:
		double width;
		double height;
		size_t numNodesX; 
		size_t numNodesY;
		double dx;
		double dy;

		Node2D& nodeAtPosition(size_t x, size_t y) {
			return gridAccessor[{ x, y }];
		}
		
		GridMesh2D(double width, double height, size_t numNodesX, size_t numNodesY) :
			width(width),
			height(height),
			numNodesX(numNodesX),
			numNodesY(numNodesY),
			dx(width / numNodesX),
			dy(height / numNodesY)
		{
			auto index = [numNodesY](size_t x, size_t y) -> size_t {
				return  x * numNodesY + y;
				};

			for(size_t i = 0; i < numNodesX; i++) {
				for (size_t j = 0; j < numNodesY; j++) {
					Node2D& node = addNode({ i * dx, j * dy });
					gridAccessor.emplace(std::pair{ i, j }, node);
				}
			}

			for (size_t i = 0; i < numNodesX; i++) {
				for (size_t j = 0; j < numNodesY; j++) {
					if (i != 0)
						addEdge(nodeAtPosition(i, j), nodeAtPosition(i - 1, j));
					if (i != numNodesX - 1)
						addEdge(nodeAtPosition(i, j), nodeAtPosition(i + 1, j));
					if (j != 0)
						addEdge(nodeAtPosition(i, j), nodeAtPosition(i, j - 1));
					if (j != numNodesY - 1)
						addEdge(nodeAtPosition(i, j), nodeAtPosition(i, j + 1));
				}
			}

			for (size_t i = 0; i < numNodesX - 1; i++) {
				for (size_t j = 0; j < numNodesY - 1; j++) {
					addFace({
						findElementByBoundary<1>({ nodeAtPosition(i, j), nodeAtPosition(i + 1, j) }),
						findElementByBoundary<1>({ nodeAtPosition(i, j), nodeAtPosition(i, j + 1) }),
						findElementByBoundary<1>({ nodeAtPosition(i + 1, j), nodeAtPosition(i + 1, j + 1) }),
						findElementByBoundary<1>({ nodeAtPosition(i, j + 1), nodeAtPosition(i + 1, j + 1) }),
					});
				}
			}
		}
	};
}