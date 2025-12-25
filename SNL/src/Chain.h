#pragma once

#include <unordered_set>
#include "Error.h"
#include "Ref.h"

namespace snl {
	template<size_t dimesion>
	class Mesh;

	template<size_t dimension, size_t meshDimension>
	class MeshElement;

	template<size_t dimension, size_t meshDimension>
	class Chain {
		std::unordered_set<Ref<MeshElement<dimension, meshDimension>>> elementsVal = {};
		Ref<Mesh<meshDimension>> meshVal;
	public:
		Mesh<meshDimension>& mesh() {
			return meshVal.get();
		}

		const Mesh<meshDimension>& mesh() const {
			return meshVal.get();
		}

		bool contains(const MeshElement<dimension, meshDimension>& element) const {
			return elementsVal.contains(element);
		}

		auto begin() {
			return elementsVal.begin();
		}

		auto end() {
			return elementsVal.end();
		}

		auto begin() const {
			return elementsVal.begin();
		}

		auto end() const {
			return elementsVal.end();
		}

		template<size_t elementDimension = dimension>
		std::unordered_set<Ref<MeshElement<elementDimension, meshDimension>>> elements();

		template<size_t elementDimension = dimension>
		std::unordered_set<Ref<const MeshElement<elementDimension, meshDimension>>> elements() const;

		std::unordered_map<Ref<MeshElement<dimension - 1, meshDimension>>, size_t> makeOccurenceMap();

		std::unordered_map<Ref<const MeshElement<dimension - 1, meshDimension>>, size_t> makeOccurenceMap() const;

		Chain<dimension - 1, meshDimension> boundary() {
			std::unordered_map<Ref<MeshElement<dimension - 1, meshDimension>>, size_t> occurenceMap = makeOccurenceMap();
			std::unordered_set<Ref<MeshElement<dimension - 1, meshDimension>>> result;

			for (const std::pair<Ref<MeshElement<dimension - 1, meshDimension>>, size_t>& p : occurenceMap)
				if (p.second == 1)
					result.insert(p.first);

			return Chain<dimension - 1, meshDimension>(mesh(), result);
		}

		const Chain<dimension - 1, meshDimension> boundary() const {
			std::unordered_map<Ref<const MeshElement<dimension - 1, meshDimension>>, size_t> occurenceMap = makeOccurenceMap();
			std::unordered_set<Ref<const MeshElement<dimension - 1, meshDimension>>> result;

			for (const std::pair<Ref<const MeshElement<dimension - 1, meshDimension>>, size_t>& p : occurenceMap)
				if (p.second == 1)
					result.insert(p.first);

			return Chain<dimension - 1, meshDimension>(mesh(), result);
		}

		Chain() = default;

		Chain(Mesh<meshDimension>& mesh, std::unordered_set<Ref<MeshElement<dimension, meshDimension>>> elements) :
			meshVal(mesh),
			elementsVal(elements) 
		{}

		friend bool operator==(const Chain<dimension, meshDimension>& a, const Chain<dimension, meshDimension>& b) {
			return a.elementsVal == b.elementsVal && &a.mesh() == &b.mesh();
		}
	};

	template<size_t meshDimension>
	using PointCloud = Chain<0, meshDimension>;

	using PointCloud2D = PointCloud<2>;
	using PointCloud3D = PointCloud<3>;

	template<size_t meshDimension>
	using Line = Chain<1, meshDimension>;

	using Line2D = Line<2>;
	using Line3D = Line<3>;

	template<size_t meshDimension>
	using Surface = Chain<2, meshDimension>;

	using Area2D = Surface<2>;
	using Surface3D = Surface<3>;

	template<size_t meshDimension>
	using Volume = Chain<3, meshDimension>;

	using Volume3D = Volume<3>;
}