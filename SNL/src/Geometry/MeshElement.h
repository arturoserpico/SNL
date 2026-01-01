#pragma once

#include <unordered_set>
#include <unordered_map>
#include <Eigen/Dense>
#include "Manifold.h"

namespace snl {
	template<size_t dimesion>
	class Mesh;

	template<size_t dimension, size_t meshDimension>
	class MeshElement {
		Manifold<dimension - 1, meshDimension> boundaryVal = {};
	 	Ref<Mesh<meshDimension>> meshVal = nullptr;
	public:
		Mesh<meshDimension>& mesh() {
			return meshVal.get();
		}

	 	const Mesh<meshDimension>& mesh() const {
			return meshVal.get();
		}

		virtual const Manifold<dimension - 1, meshDimension> boundary() const {
			return boundaryVal;
		};

		virtual Manifold<dimension - 1, meshDimension> boundary() {
			return boundaryVal;
		};

		template<size_t elementDimension>
		ElementComplex<elementDimension, meshDimension> elements() {
			if constexpr (elementDimension == dimension) {
				return ElementComplex<elementDimension, meshDimension>(mesh(), { *this });
			}
			else {
				Manifold<dimension - 1, meshDimension> boundary = this->boundary();

				return boundary.elements<elementDimension>();
			}
		}

		template<size_t elementDimension>
		const ElementComplex<elementDimension, meshDimension> elements() const {
			if constexpr (elementDimension == dimension) {
				return ElementComplex<elementDimension, meshDimension>(mesh(), { *this });
			}
			else {
				const Manifold<dimension - 1, meshDimension> boundary = this->boundary();

				return boundary.elements<elementDimension>();
			}
		}

		Manifold<dimension, meshDimension> manifold() {
			return Manifold<dimension, meshDimension>(mesh(), { *this });
		}

		const Manifold<dimension, meshDimension> manifold() const {
			return Manifold<dimension, meshDimension>(mesh(), { *this });
		}

		//bool isClosed() const {
		//	std::map<Ref<const MeshElement<dimension - 2, meshDimension>>, size_t> elementCount;
		//
		//	Set<Ref<const MeshElement<dimension - 1, meshDimension>>> boundary = this->boundary();
		//
		//	for (const MeshElement<dimension - 1, meshDimension>& boundaryElement : boundary) {
		//		Set<Ref<const MeshElement<dimension - 2, meshDimension>>> boundaryElementBoundary
		//	}
		//}

		MeshElement() = default;

		MeshElement(
			Mesh<meshDimension>& mesh,
			Manifold<dimension - 1, meshDimension> boundary = {}
		) : 
			meshVal(mesh), 
			boundaryVal(boundary)
		{}

		MeshElement(
			Mesh<meshDimension>& mesh,
			const Set<Ref<MeshElement<dimension - 1, meshDimension>>>& boundary = {}
		) :
			meshVal(mesh),
			boundaryVal(mesh, boundary)
		{}

		friend bool operator==(const MeshElement<dimension, meshDimension>& a, const MeshElement<dimension, meshDimension>& b) {
			return a.boundary() == b.boundary() && &a.mesh() == &b.mesh();
		}
	};

	template<size_t meshDimension>
	using Face = MeshElement<2, meshDimension>;

	using Face2D = Face<2>;
	using Face3D = Face<3>;

	template<size_t dimension, size_t meshDimension>
	template<size_t elementDimension>
	ElementComplex<elementDimension, meshDimension> ElementComplex<dimension, meshDimension>::elements() {
		if constexpr (elementDimension == dimension) {
			return *this;
		} else {
			Set<Ref<MeshElement<elementDimension, meshDimension>>> result;
			ElementComplex<elementDimension, meshDimension> rec;

			for (MeshElement<dimension, meshDimension>& elements : this->elements()) {
				rec = elements.elements<elementDimension>();

				result.insert(rec.begin(), rec.end());
			}

			return ElementComplex<elementDimension, meshDimension>(mesh(), result);
		}
	}

	template<size_t dimension, size_t meshDimension>
	template<size_t elementDimension>
	const ElementComplex<elementDimension, meshDimension> ElementComplex<dimension, meshDimension>::elements() const {
		Set<Ref<MeshElement<elementDimension, meshDimension>>> result;
		
		if constexpr (elementDimension == dimension) {
			for (MeshElement<dimension, meshDimension>& element : elementsVal)
				result.insert(Ref<MeshElement<elementDimension, meshDimension>>(element));
		} else {
			ElementComplex<elementDimension, meshDimension> rec;

			for (MeshElement<dimension, meshDimension>& elements : this->elements()) {
				rec = elements.elements<elementDimension>();

				result.insert(rec.begin(), rec.end());
			}
		}

		return ElementComplex<elementDimension, meshDimension>(meshVal, result);
	}

	template<size_t dimension, size_t meshDimension>
	std::unordered_map<Ref<MeshElement<dimension - 1, meshDimension>>, size_t> ElementComplex<dimension, meshDimension>::makeOccurenceMap() {
		std::unordered_map<Ref<MeshElement<dimension - 1, meshDimension>>, size_t> result;

		for (MeshElement<dimension, meshDimension>& element : elements()) {
			auto boundary = element.boundary();

			for (MeshElement<dimension - 1, meshDimension>& internalElement : boundary) {
				if (result.contains(internalElement))
					result.at(internalElement)++;
				else
					result.emplace(internalElement, 1);
			}
		}

		return result;
	}

	template<size_t dimension, size_t meshDimension>
	std::unordered_map<Ref<const MeshElement<dimension - 1, meshDimension>>, size_t> ElementComplex<dimension, meshDimension>::makeOccurenceMap() const {
		std::unordered_map<Ref<const MeshElement<dimension - 1, meshDimension>>, size_t> result;

		for (const MeshElement<dimension, meshDimension>& element : elements()) {
			auto boundary = element.boundary();

			for (const MeshElement<dimension - 1, meshDimension>& internalElement : boundary) {
				if (result.contains(internalElement))
					result.at(internalElement)++;
				else
					result.emplace(internalElement, 1);
			}
		}

		return result;
	}
}

namespace std {
	template<typename Scalar, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
	struct hash<Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols>>
	{
		size_t operator()(const Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols>& m) const noexcept
		{
			size_t seed = 0;

			auto hashCombine = [&seed](size_t v) {
				seed ^= v + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
				};

			// Include dimensions (important for dynamic-size matrices)
			hashCombine(std::hash<int>()(m.rows()));
			hashCombine(std::hash<int>()(m.cols()));

			// Hash raw data
			const Scalar* data = m.data();
			for (int i = 0; i < m.size(); ++i) {
				hashCombine(std::hash<Scalar>()(data[i]));
			}

			return seed;
		}
	};
}

namespace std {
	template<size_t dimension, size_t meshDimension>
	struct hash<snl::MeshElement<dimension, meshDimension>> {
		size_t operator()(const snl::MeshElement<dimension, meshDimension>& element) const noexcept {
			size_t seed = 0;
			
			auto hashCombine = [&seed](size_t v) {
				seed += v * 0x9e3779b97f4a7c15ULL;
				};

			auto boundary = element.boundary();

			for (const snl::MeshElement<dimension - 1, meshDimension>& boundaryElement : boundary)
				hashCombine(std::hash<snl::MeshElement<dimension - 1, meshDimension>>{}(boundaryElement));

			hashCombine(std::hash<snl::Ref<const snl::Mesh<meshDimension>>>{}(element.mesh()));

			return seed;
		}
	};
}

// : posVal(pos), meshVal(new Mesh<meshDimension>()), isMeshOwned(true)