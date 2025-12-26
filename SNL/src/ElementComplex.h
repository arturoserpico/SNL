#pragma once

#include "Error.h"
#include "Set.h"
#include "Ref.h"

namespace snl {
	template<size_t dimesion>
	class Mesh;

	template<size_t dimension, size_t meshDimension>
	class MeshElement;

	template<size_t dimension, size_t meshDimension>
	struct Manifold;

	template<size_t dimension, size_t meshDimension>
	class ElementComplex {
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

		void add(MeshElement<dimension, meshDimension>& element) {
			elementsVal.insert(element);
		}

		bool empty() {
			return elementsVal.empty();
		}

		size_t size() const {
			return elementsVal.size();
		}

		template<size_t elementDimension = dimension>
		ElementComplex<elementDimension, meshDimension> elements();

		template<size_t elementDimension = dimension>
		const ElementComplex<elementDimension, meshDimension> elements() const;

		std::unordered_map<Ref<MeshElement<dimension - 1, meshDimension>>, size_t> makeOccurenceMap();

		std::unordered_map<Ref<const MeshElement<dimension - 1, meshDimension>>, size_t> makeOccurenceMap() const;

		bool isManifold() const {
			bool result = true;

			std::unordered_map<Ref<const MeshElement<dimension - 1, meshDimension>>, size_t> occurenceMap = makeOccurenceMap();

			for (const std::pair<Ref<const MeshElement<dimension - 1, meshDimension>>, size_t>& p : occurenceMap)
				result = result && (p.second < 3);

			return result;
		}

		bool isClosed() const {
			return boundary().empty();
		}

		ElementComplex<dimension - 1, meshDimension> boundary() {
			std::unordered_map<Ref<MeshElement<dimension - 1, meshDimension>>, size_t> occurenceMap = makeOccurenceMap();
			Set<Ref<MeshElement<dimension - 1, meshDimension>>> result;

			for (const std::pair<Ref<MeshElement<dimension - 1, meshDimension>>, size_t>& p : occurenceMap)
				if (p.second == 1)
					result.insert(p.first);

			return ElementComplex<dimension - 1, meshDimension>(mesh(), result);
		}

		const ElementComplex<dimension - 1, meshDimension> boundary() const {
			std::unordered_map<Ref<const MeshElement<dimension - 1, meshDimension>>, size_t> occurenceMap = makeOccurenceMap();
			Set<Ref<const MeshElement<dimension - 1, meshDimension>>> result;

			for (const std::pair<Ref<const MeshElement<dimension - 1, meshDimension>>, size_t>& p : occurenceMap)
				if (p.second == 1)
					result.insert(p.first);

			return ElementComplex<dimension - 1, meshDimension>(mesh(), result);
		}

		Manifold<dimension, meshDimension> manifold();
		const Manifold<dimension, meshDimension> manifold() const;

		friend bool operator==(const ElementComplex<dimension, meshDimension>& a, const ElementComplex<dimension, meshDimension>& b) {
			return a.elementsVal == b.elementsVal && &a.mesh() == &b.mesh();
		}

		ElementComplex() = default;

		ElementComplex(Mesh<meshDimension>& mesh, const Set<Ref<MeshElement<dimension, meshDimension>>>& elements) :
			meshVal(mesh),
			elementsVal(elements)
		{}
	};

	template<size_t meshDimension>
	class ElementComplex<0, meshDimension> {
		Set<Ref<MeshElement<0, meshDimension>>> elementsVal = {};
		Ref<Mesh<meshDimension>> meshVal;
	public:
		Mesh<meshDimension>& mesh() {
			return meshVal.get();
		}

		const Mesh<meshDimension>& mesh() const {
			return meshVal.get();
		}

		bool contains(const MeshElement<0, meshDimension>& element) const {
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

		void add(MeshElement<0, meshDimension>& element) {
			elementsVal.insert(element);
		}

		bool empty() {
			return elementsVal.empty();
		}

		size_t size() const {
			return elementsVal.size();
		}

		template<size_t elementDimension = 0>
		ElementComplex<elementDimension, meshDimension> elements() {
			return *this;
		}

		template<size_t elementDimension = 0>
		const ElementComplex<elementDimension, meshDimension> elements() const {
			return *this;
		}

		//std::unordered_map<Ref<MeshElement<dimension - 1, meshDimension>>, size_t> makeOccurenceMap();

		//std::unordered_map<Ref<const MeshElement<dimension - 1, meshDimension>>, size_t> makeOccurenceMap() const;

		bool isManifold() const {
			return true;
		}

		bool isClosed() const {
			return true;
		}

		Manifold<0, meshDimension> manifold();
		const Manifold<0, meshDimension> manifold() const;

		friend bool operator==(const ElementComplex<0, meshDimension>& a, const ElementComplex<0, meshDimension>& b) {
			return a.elementsVal == b.elementsVal && &a.mesh() == &b.mesh();
		}

		ElementComplex() = default;

		ElementComplex(Mesh<meshDimension>& mesh, const Set<Ref<MeshElement<0, meshDimension>>>& elements) :
			meshVal(mesh),
			elementsVal(elements)
		{}
	};
	
	template<size_t meshDimension>
	using PointComplex = ElementComplex<0, meshDimension>;

	using PointComplex2D = PointComplex<2>;
	using PointComplex3D = PointComplex<3>;

	template<size_t meshDimension>
	using EdgeComplex = ElementComplex<1, meshDimension>;

	using EdgeComplex2D = EdgeComplex<2>;
	using EdgeComplex3D = EdgeComplex<3>;

	template<size_t meshDimension>
	using FaceComplex = ElementComplex<2, meshDimension>;

	using FaceComplex2D = FaceComplex<2>;
	using FaceComplex3D = FaceComplex<3>;
}