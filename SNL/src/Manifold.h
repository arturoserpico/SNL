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
	class Manifold {
		Set<Ref<MeshElement<dimension, meshDimension>>> elementsVal = {};
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

		size_t size() const {
			return elementsVal.size();
		}

		template<size_t elementDimension = dimension>
		Set<Ref<MeshElement<elementDimension, meshDimension>>> elements();

		template<size_t elementDimension = dimension>
		Set<Ref<const MeshElement<elementDimension, meshDimension>>> elements() const;

		std::unordered_map<Ref<MeshElement<dimension - 1, meshDimension>>, size_t> makeOccurenceMap();

		std::unordered_map<Ref<const MeshElement<dimension - 1, meshDimension>>, size_t> makeOccurenceMap() const;

		Manifold<dimension - 1, meshDimension> boundary() {
			std::unordered_map<Ref<MeshElement<dimension - 1, meshDimension>>, size_t> occurenceMap = makeOccurenceMap();
			Set<Ref<MeshElement<dimension - 1, meshDimension>>> result;

			for (const std::pair<Ref<MeshElement<dimension - 1, meshDimension>>, size_t>& p : occurenceMap)
				if (p.second == 1)
					result.insert(p.first);

			return Manifold<dimension - 1, meshDimension>(mesh(), result);
		}

		const Manifold<dimension - 1, meshDimension> boundary() const {
			std::unordered_map<Ref<const MeshElement<dimension - 1, meshDimension>>, size_t> occurenceMap = makeOccurenceMap();
			Set<Ref<const MeshElement<dimension - 1, meshDimension>>> result;

			for (const std::pair<Ref<const MeshElement<dimension - 1, meshDimension>>, size_t>& p : occurenceMap)
				if (p.second == 1)
					result.insert(p.first);

			return Manifold<dimension - 1, meshDimension>(mesh(), result);
		}

		Manifold() = default;

		Manifold(Mesh<meshDimension>& mesh, Set<Ref<MeshElement<dimension, meshDimension>>> elements) :
			meshVal(mesh),
			elementsVal(elements) 
		{}

		friend bool operator==(const Manifold<dimension, meshDimension>& a, const Manifold<dimension, meshDimension>& b) {
			return a.elementsVal == b.elementsVal && &a.mesh() == &b.mesh();
		}
	};

	template<size_t meshDimension>
	using PointCloud = Manifold<0, meshDimension>;

	using PointCloud2D = PointCloud<2>;
	using PointCloud3D = PointCloud<3>;

	template<size_t meshDimension>
	using Line = Manifold<1, meshDimension>;

	using Line2D = Line<2>;
	using Line3D = Line<3>;

	template<size_t meshDimension>
	using Surface = Manifold<2, meshDimension>;

	using Area2D = Surface<2>;
	using Surface3D = Surface<3>;

	template<size_t meshDimension>
	using Volume = Manifold<3, meshDimension>;

	using Volume3D = Volume<3>;
}