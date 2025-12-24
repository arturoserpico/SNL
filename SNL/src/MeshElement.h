#pragma once

#include <unordered_set>
#include <Eigen/Dense>
#include "Ref.h"

namespace snl {
	template<size_t dimesion>
	class Mesh;

	template<size_t dimension, size_t meshDimension>
	class MeshElement {
		std::unordered_set<Ref<MeshElement<dimension - 1, meshDimension>>> boundaryVal = {};
	 	Ref<Mesh<meshDimension>> meshVal = nullptr;
	public:
		Mesh<meshDimension>& mesh() const {
			return meshVal.get();
		}

		virtual std::unordered_set<Ref<const MeshElement<dimension - 1, meshDimension>>> boundary() const {
			std::unordered_set<Ref<const MeshElement<dimension - 1, meshDimension>>> result;

			for (MeshElement<dimension - 1, meshDimension>& element : boundaryVal)
				result.insert(element);

			return result;
		};

		virtual std::unordered_set<Ref<MeshElement<dimension - 1, meshDimension>>> boundary() {
			return boundaryVal;
		};

		template<size_t elementDimension>
		std::unordered_set<Ref<MeshElement<elementDimension, meshDimension>>> elements() {
			std::unordered_set<Ref<MeshElement<elementDimension, meshDimension>>> result;

			if constexpr (elementDimension == dimension) {
				return { *this };
			}

			std::unordered_set<Ref<MeshElement<dimension - 1, meshDimension>>> boundary = this->boundary();
			std::unordered_set<Ref<MeshElement<elementDimension, meshDimension>>> rec;
			for (MeshElement<dimension - 1, meshDimension>& boundaryElement : boundary) {
				rec = boundaryElement.elements<elementDimension>();
				result.insert(rec.begin(), rec.end());
			}

			return result;
		}

		template<size_t elementDimension>
		std::unordered_set<Ref<const MeshElement<elementDimension, meshDimension>>> elements() const {
			std::unordered_set<Ref<const MeshElement<elementDimension, meshDimension>>> result;

			if constexpr (elementDimension == dimension) {
				return { *this };
			}

			std::unordered_set<Ref<const MeshElement<dimension - 1, meshDimension>>> boundary = this->boundary();
			std::unordered_set<Ref<const MeshElement<elementDimension, meshDimension>>> rec;
			for (const MeshElement<dimension - 1, meshDimension>& boundaryElement : boundary) {
				rec = boundaryElement.elements<elementDimension>();
				result.insert(rec.begin(), rec.end());
			}

			return result;
		}

		//bool isClosed() const {
		//	std::map<Ref<const MeshElement<dimension - 2, meshDimension>>, size_t> elementCount;
		//
		//	std::unordered_set<Ref<const MeshElement<dimension - 1, meshDimension>>> boundary = this->boundary();
		//
		//	for (const MeshElement<dimension - 1, meshDimension>& boundaryElement : boundary) {
		//		std::unordered_set<Ref<const MeshElement<dimension - 2, meshDimension>>> boundaryElementBoundary
		//	}
		//}

		MeshElement() = default;

		MeshElement(
			Mesh<meshDimension>& mesh,
			std::unordered_set<Ref<MeshElement<dimension - 1, meshDimension>>> boundary = {}
		) : 
			meshVal(mesh), 
			boundaryVal(boundary)
		{}

		friend bool operator==(const MeshElement<dimension, meshDimension>& a, const MeshElement<dimension, meshDimension>& b) {
			return a.boundary() == b.boundary() && &a.mesh() == &b.mesh();
		}
	};

	template<size_t meshDimension>
	using Face = MeshElement<2, meshDimension>;

	using Face2D = Face<2>;
	using Face3D = Face<3>;
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

			for (const snl::MeshElement<dimension - 1, meshDimension>& boundaryElement : element.boundary())
				hashCombine(std::hash<snl::MeshElement<dimension - 1, meshDimension>>{}(boundaryElement));

			hashCombine(std::hash<snl::Ref<snl::Mesh<meshDimension>>>{}(element.mesh()));

			return seed;
		}
	};
}

// : posVal(pos), meshVal(new Mesh<meshDimension>()), isMeshOwned(true)