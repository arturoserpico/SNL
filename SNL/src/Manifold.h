#pragma once

#include "Ref.h"
#include "Edge.h"

namespace snl {
	template<size_t dimension, size_t meshDimension>
	class Manifold {
		std::unordered_set<Ref<MeshElement<dimension, meshDimension>>> elements;
		Ref<Mesh<meshDimension>> meshVal;
	public:
		Mesh<meshDimension>& mesh() {
			expect(!meshVal.empty(), "cannot access not bound Mesh reference");
			return meshVal;
		}

		const Mesh<meshDimension>& mesh() const {
			expect(!meshVal.empty(), "cannot access not bound Mesh reference");
			return meshVal;
		}


	};
}