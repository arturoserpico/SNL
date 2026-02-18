#pragma once

#include "MeshElement.h"

namespace snl {
	template<size_t meshDimension>
	using Node = MeshElement<0, meshDimension>;

	template<size_t meshDimension>
	class MeshElement<0, meshDimension> {
		Eigen::Vector<double, meshDimension> posVal = Eigen::Vector<double, meshDimension>::Zero();
		Ref<Mesh<meshDimension>> meshVal = nullptr;
	public:
		Mesh<meshDimension>& mesh() {
			return meshVal.get();
		}

		const Mesh<meshDimension>& mesh() const {
			return meshVal.get();
		}

		//Set<MeshElement<dimension - 1, meshDimension>*> boundary() const {
		//	return {};
		//};

		Eigen::Vector<double, meshDimension>& pos() {
			return posVal;
		}

		const Eigen::Vector<double, meshDimension>& pos() const {
			return posVal;
		}

		template<size_t elementDimension>
		Set<Ref<MeshElement<elementDimension, meshDimension>>> elements() {
			if constexpr (elementDimension == 0) {
				return { *this };
			} else {
				return {};
			}
		}

		template<size_t elementDimension>
		Set<Ref<const MeshElement<elementDimension, meshDimension>>> elements() const {
			if constexpr (elementDimension == 0) {
				return { *this };
			}
			else {
				return {};
			}
		}

		MeshElement() = default;

		MeshElement(
			Mesh<meshDimension>& mesh,
			Eigen::Vector<double, meshDimension> pos
		) :
			meshVal(mesh),
			posVal(pos)
		{}

		friend bool operator==(const Node<meshDimension>& a, const Node<meshDimension>& b) {
			return a.pos() == b.pos();
		}
	};

	using Node2D = Node<2>;
	using Node3D = Node<3>;
}

namespace std {
	template<size_t meshDimension>
	struct hash<snl::Node<meshDimension>> {
		size_t operator()(const snl::Node<meshDimension>& node) const noexcept {
			return std::hash<Eigen::Vector<double, meshDimension>>{}(node.pos()) ^
				(1 << std::hash<snl::Ref<const snl::Mesh<meshDimension>>>{}(node.mesh())); // combine hashes
		}
	};
}