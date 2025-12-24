#pragma once

#include "Node.h"

namespace snl {
	template<size_t meshDimension>
	using Edge = MeshElement<1, meshDimension>;

	template<size_t meshDimension>
	class MeshElement<1, meshDimension> {
		std::unordered_set<Ref<Node<meshDimension>>> boundaryVal = {};
		Ref<Mesh<meshDimension>> meshVal = nullptr;
	public:
		Mesh<meshDimension>& mesh() const {
			return meshVal.get();
		}

		virtual std::unordered_set<Ref<const Node<meshDimension>>> boundary() const {
			std::unordered_set<Ref<const Node<meshDimension>>> result;

			for (Node<meshDimension>& element : boundaryVal)
				result.insert(element);

			return result;
		}

		virtual std::unordered_set<Ref<Node<meshDimension>>> boundary() {
			return boundaryVal;
		}

		std::pair<Node<meshDimension>&, Node<meshDimension>&> nodes() {
			auto it = boundary.begin();
			Node<meshDimension>& a = *it;
			it++;
			Node<meshDimension>& b = *it;

			return { a, b };
		}

		std::pair<const Node<meshDimension>&, const Node<meshDimension>&> nodes() const {
			auto boundary = this->boundary();
			auto it = boundary.begin();
			const Node<meshDimension>& a = *it;
			it++;
			const Node<meshDimension>& b = *it;

			return { a, b };
		}

		double lenght() const {
			const auto& [a, b] = nodes();

			return (a.pos() - b.pos()).norm();
		}

		template<size_t elementDimension>
		std::unordered_set<Ref<MeshElement<elementDimension, meshDimension>>> elements() {
			if constexpr (elementDimension == 1) {
				return { *this };
			} else {
				return boundary();
			}
		}

		template<size_t elementDimension>
		std::unordered_set<Ref<const MeshElement<elementDimension, meshDimension>>> elements() const {
			if constexpr (elementDimension == 1) {
				return { *this };
			}
			else {
				return boundary();
			}
		}

		MeshElement() = default;

		MeshElement(
			Mesh<meshDimension>& mesh,
			std::unordered_set<Ref<Node<meshDimension>>> boundary = {}
		) :
			meshVal(&mesh),
			boundaryVal(boundary)
		{
			if (boundary.size() != 2)
				throw Exception("Edge must have exactly 2 nodes in its boundary");
		}

		MeshElement(
			Mesh<meshDimension>& mesh,
			Node<meshDimension>& a,
			Node<meshDimension>& b
		) :
			meshVal(mesh),
			boundaryVal({ Ref(a), Ref(b) })
		{}

		friend bool operator==(const Edge<meshDimension>& a, const Edge<meshDimension>& b) {
			return a.boundary() == b.boundary() && &a.mesh() == &b.mesh();
		}
	};

	using Edge2D = Edge<2>;
	using Edge3D = Edge<3>;
}