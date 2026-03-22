#pragma once

#include "ElementComplex.h"

namespace snl {
	template<size_t dimension, size_t meshDimension>
	struct Manifold : public ElementComplex<dimension, meshDimension> {
		Manifold<dimension - 1, meshDimension> boundary() {
			return static_cast<ElementComplex<dimension, meshDimension>&>(*this).boundary().manifold();
		}

		const Manifold<dimension - 1, meshDimension> boundary() const {
			return static_cast<const ElementComplex<dimension, meshDimension>&>(*this).boundary().manifold();
		}

		Manifold() = default;

		Manifold(const ElementComplex<dimension, meshDimension>& cellComplex) :
			ElementComplex<dimension, meshDimension>(cellComplex)
		{
			SNLDebugCall(1, expect(this->isManifold(), "cannot create Manifold from non manifold CellComplex"));
		}

		Manifold(Mesh<meshDimension>& mesh, const Set<Ref<MeshElement<dimension, meshDimension>>>& elements) :
			ElementComplex<dimension, meshDimension>(mesh, elements)
		{
			SNLDebugCall(1, expect(this->isManifold(), "cannot create Manifold from non manifold MeshElement set"));
		}
	};

	template<size_t dimension, size_t meshDimension>
	Manifold<dimension, meshDimension> ElementComplex<dimension, meshDimension>::manifold() {
		return Manifold<dimension, meshDimension>(*this);
	}

	template<size_t dimension, size_t meshDimension>
	const Manifold<dimension, meshDimension> ElementComplex<dimension, meshDimension>::manifold() const {
		return Manifold<dimension, meshDimension>(*this);
	}

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

	using Region2D = Surface<2>;
	using Surface3D = Surface<3>;

	template<size_t meshDimension>
	using Volume = Manifold<3, meshDimension>;

	using Volume3D = Volume<3>;
}