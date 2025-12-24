#pragma once

#include "Edge.h"

namespace snl {
	template<size_t meshDimension>
	class Triangle : public Face<meshDimension> {
		std::unordered_set<Ref<Edge<meshDimension>>> boundaryVal = {};
	public:
		virtual std::unordered_set<Ref<const Edge<meshDimension>>> boundary() const {
			std::unordered_set<Ref<const Edge<meshDimension>>> result;

			for (Edge<meshDimension>& element : boundaryVal)
				result.insert(element);

			return result;
		};

		virtual std::unordered_set<Ref<Edge<meshDimension>>> boundary() {
			return boundaryVal;
		};

		std::array<Edge<meshDimension>&, 3> edges() {
			std::array<Edge<meshDimension>&, 3> result;
			
			std::unordered_set<Ref<Edge<meshDimension>>> boundary = this->boundary();

			auto it = boundary.begin();

			for (size_t i = 0; i < 3; i++) {
				result[i] = *it;
				it++;
			}

			return result;
		}

		std::array<const Edge<meshDimension>&, 3> edges() const {
			std::array<const Edge<meshDimension>&, 3> result;

			std::unordered_set<Ref<const Edge<meshDimension>>> boundary = this->boundary();

			auto it = boundary.begin();

			for (size_t i = 0; i < 3; i++) {
				result[i] = *it;
				it++;
			}

			return result;
		}

		std::array<Node<meshDimension>&, 3> nodes() const {

		}

		Triangle() = default;

		Triangle(const std::unordered_set<Ref<Edge<meshDimension>>>& boundary) : boundaryVal(boundary) {
			if (boundary.size() != 3)
				throw Exception("A Triangle must have exactly 3 edges in its boundary");
		};
	};
}