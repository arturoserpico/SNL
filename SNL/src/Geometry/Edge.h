#pragma once

#include "Node.h"

namespace snl {
	template<size_t meshDimension>
	using Edge = MeshElement<1, meshDimension>;

	template<size_t meshDimension>
	class MeshElement<1, meshDimension> {
		PointCloud<meshDimension> boundaryVal = {};
		Ref<Mesh<meshDimension>> meshVal = nullptr;
	public:
		Mesh<meshDimension>& mesh() {
			return meshVal.get();
		}

		const Mesh<meshDimension>& mesh() const {
			return meshVal.get();
		}

		virtual const PointCloud<meshDimension> boundary() const {
			return boundaryVal;
		}

		virtual PointCloud<meshDimension> boundary() {
			return boundaryVal;
		}

		std::pair<Node<meshDimension>&, Node<meshDimension>&> nodes() {
			auto boundary = this->boundary();
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

		Line<meshDimension> manifold() {
			return Line<meshDimension>(mesh(), { *this });
		}

		const Line<meshDimension> manifold() const {
			return Line<meshDimension>(mesh(), { *this });
		}

		template<size_t elementDimension>
		ElementComplex<elementDimension, meshDimension> elements() {
			if constexpr (elementDimension == 1) {
				return ElementComplex<elementDimension, meshDimension>(mesh(), { *this });
			} else {
				return boundary().elements();
			}
		}

		template<size_t elementDimension>
		const ElementComplex<elementDimension, meshDimension> elements() const {
			if constexpr (elementDimension == 1) {
				return ElementComplex<elementDimension, meshDimension>(mesh(), { *this });
			}
			else {
				return boundary().elements();
			}
		}

		MeshElement() = default;

		MeshElement(
			Mesh<meshDimension>& mesh,
			const PointCloud<meshDimension>& boundary = {}
		) :
			meshVal(mesh),
			boundaryVal(boundary)
		{
			SNLDebugCall(1, expect(boundary.elements().size() == 2, "Edge must have exactly two nodes in its boundary."));
		}

		MeshElement(
			Mesh<meshDimension>& mesh,
			const Set<Ref<Node<meshDimension>>>& boundary = {}
		) :
			meshVal(mesh),
			boundaryVal(mesh, boundary)
		{
			SNLDebugCall(1, expect(boundary.size() == 2, "Edge must have exactly two nodes in its boundary."));
		}

		MeshElement(
			Mesh<meshDimension>& mesh,
			Node<meshDimension>& a,
			Node<meshDimension>& b
		) :
			meshVal(mesh),
			boundaryVal(mesh, { Ref(a), Ref(b) })
		{}

		friend bool operator==(const Edge<meshDimension>& a, const Edge<meshDimension>& b) {
			return a.boundary() == b.boundary() && &a.mesh() == &b.mesh();
		}
	};

	using Edge2D = Edge<2>;
	using Edge3D = Edge<3>;
}