#pragma once

#include <iostream>
#include <initializer_list>
#include <set>
#include <unordered_set>
#include <utility>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include "Ref.h"
#include "UniqueVector.h"
#include "MeshElement.h"
#include "Node.h"
#include "Edge.h"

namespace snl
{
	template<size_t dimension, typename IndexSeq>
	struct _Elements;

	template<size_t dimension, size_t... indexs>
	struct _Elements<dimension, std::index_sequence<indexs...>> {
		using Type = std::tuple<Ref<ElementComplex<indexs, dimension>>...>;
	};

	template<size_t dimension, size_t indexUntil = dimension>
	using Elements = _Elements<dimension, std::make_index_sequence<indexUntil + 1>>::Type;

	//template<size_t dimension, typename IndexSeq>
	//struct _ElementsRef;
	//
	//template<size_t dimension, size_t... indexs>
	//struct _ElementsRef<dimension, std::index_sequence<indexs...>> {
	//	using Type = std::tuple<Set<MeshElement<indexs, dimension>*>*...>;
	//};
	//
	//template<size_t dimension>
	//using ElementsRef = _ElementsRef<dimension, std::make_index_sequence<dimension + 1>>::Type;
	//
	//template<size_t dimension, size_t index = 0>
	//ElementsRef<dimension - index> elementsToElementRef(Elements<dimension> elements) {
	//	return std::tuple_cat(std::tuple(&std::get<index>(elements)) elementsToElementRef<dimension, index + 1>)
	//}

	template<size_t dimension>
	class Mesh {
	protected:
		template<size_t dimension, size_t index = dimension>
		Elements<dimension, index> initElements() {
			if constexpr (index == 0)
				return std::tuple(Ref(*(new ElementComplex<0, dimension>(*this, {}))));
			else
				return std::tuple_cat(initElements<dimension, index - 1>(), std::tuple(Ref(*(new ElementComplex<index, dimension>(*this, {})))));
		}

		Elements<dimension> elementsVal = initElements<dimension>();
	public:
		template<size_t elementDimension>
		ElementComplex<elementDimension, dimension>& elements() {
			return std::get<elementDimension>(elementsVal);
		}

		template<size_t elementDimension>
		const ElementComplex<elementDimension, dimension>& elements() const {
			return std::get<elementDimension>(elementsVal);
		}

		auto& nodes() {
			return elements<0>();
		}

		const auto& nodes() const {
			return elements<0>();
		}

		auto& edges() {
			return elements<1>();
		}

		const auto& edges() const {
			return elements<1>();
		}

		auto& faces() {
			return elements<2>();
		}

		const auto& faces() const {
			return elements<2>();
		}

		Node<dimension>& addNode(const Eigen::Vector<double, dimension>& pos) {
			Node<dimension>* node = new Node<dimension>(*this, pos);

			nodes().add(*node);

			return *node;
		}

		Edge<dimension>& addEdge(Node<dimension>& a, Node<dimension>& b) {
			Edge<dimension>* edge = new Edge<dimension>(*this, a, b);

			edges().add(*edge);

			return *edge;
		}

		Edge<dimension>& addEdge(const Eigen::Vector<double, dimension>& posA, const Eigen::Vector<double, dimension>& posB) {
			Node<dimension>& a = addNode(posA);
			Node<dimension>& b = addNode(posB);

			Edge<dimension>* edge = new Edge<dimension>(*this, a, b);

			edges().add(*edge);

			return *edge;
		}

		Face<dimension>& addFace(const Set<Ref<Edge<dimension>>>& edges) {
			Face<dimension>* face = new Face<dimension>(*this, edges);
			faces().add(*face);
			return *face;
		}

		template<size_t elementDimension>
		MeshElement<elementDimension, dimension>& addElement(const Set<Ref<MeshElement<elementDimension - 1, dimension>>>& boundary) {
			MeshElement<elementDimension, dimension>* element = new MeshElement<elementDimension, dimension>(*this, boundary);
			elements<elementDimension>().add(*element);
			return *element;
		}

		template<size_t elementDimension>
		MeshElement<elementDimension, dimension>& findElementByBoundary(const Set<Ref<MeshElement<elementDimension - 1, dimension>>>& boundary) {
			return findElementByBoundary<elementDimension>(Manifold<elementDimension - 1, dimension>(*this, boundary));
		}

		template<size_t elementDimension>
		MeshElement<elementDimension, dimension>& findElementByBoundary(const Manifold<elementDimension - 1, dimension>& boundary) {
			for (MeshElement<elementDimension, dimension>& element : elements<elementDimension>()) {
				if (element.boundary() == boundary)
					return element;
			}
			throw Exception("Element with given boundary not found");
		}

		template<size_t elementDimension>
		const MeshElement<elementDimension, dimension>& findElementByBoundary(const Set<Ref<MeshElement<elementDimension - 1, dimension>>>& boundary) const {
			return findElementByBoundary<elementDimension>(Manifold<elementDimension - 1, dimension>(*this, boundary));
		}

		template<size_t elementDimension>
		const MeshElement<elementDimension, dimension>& findElementByBoundary(const Manifold<elementDimension - 1, dimension>& boundary) const {
			for (const MeshElement<elementDimension, dimension>& element : elements<elementDimension>()) {
				if (element.boundary() == boundary)
					return element;
			}
			throw Exception("Element with given boundary not found");
		}

		template<size_t elementDimension>
		Manifold<elementDimension, dimension> findElementsByBoundary(MeshElement<elementDimension - 1, dimension>& boundaryElement) {
			Set<Ref<MeshElement<elementDimension, dimension>>> result;
			for (MeshElement<elementDimension, dimension>& element : elements<elementDimension>())
				if (element.boundary().contains(boundaryElement))
					result.insert(element);

			return Manifold<elementDimension, dimension>(*this, result);
		}

		template<size_t elementDimension>
		const Manifold<elementDimension, dimension> findElementsByBoundary(const MeshElement<elementDimension - 1, dimension>& boundaryElement) const {
			Set<Ref<const MeshElement<elementDimension, dimension>>> result;
			for (const MeshElement<elementDimension, dimension>& element : elements<elementDimension>())
				if (element.boundary().contains(boundaryElement))
					result.insert(element);

			return Manifold<elementDimension, dimension>(*this, result);
		}

		Manifold<dimension, dimension> manifold() {
			return Manifold<dimension, dimension>(elements<dimension>());
		}

		Mesh() = default;

		Mesh(Elements<dimension> elements) : elementsVal(elements) {}

		void print(std::ostream& positions = std::cout, std::ostream& edges = std::cout, std::ostream& faces = std::cout) const {
			std::map<const Node<dimension>*, size_t> nodeIndexMap;

			size_t index = 0;

			for (const Node<dimension>& node : nodes()) {
				nodeIndexMap.emplace(&node, index);
				index++;
			}

			index = 0;

			std::map<size_t, const Node<dimension>*> invertedNodeIndexMap;

			for (const std::pair<const Node<dimension>*, size_t>& p : nodeIndexMap) {
				invertedNodeIndexMap.emplace(p.second, p.first);
			}

			for (const std::pair<size_t, const Node<dimension>*>& p : invertedNodeIndexMap) {
				positions << p.second->pos()(0) << " " << p.second->pos()(1) << "\n";
			}

			std::map<const Edge<dimension>*, size_t> edgeIndexMap;

			for (const Edge<dimension>& node : this->edges()) {
				edgeIndexMap.emplace(&node, index);
				index++;
			}

			std::map<size_t, const Edge<dimension>*> invertedEdgeIndexMap;

			for (const std::pair<const Edge<dimension>*, size_t>& p : edgeIndexMap) {
				invertedEdgeIndexMap.emplace(p.second, p.first);
			}

			for (const std::pair<size_t, const Edge<dimension>*>& p : invertedEdgeIndexMap) {
				const auto& [a, b] = p.second->nodes();
				edges << nodeIndexMap[&a] << " " << nodeIndexMap[&b] << "\n";
			}

			for (const Face<dimension>& face : this->faces()) {
				auto boundary = face.boundary();

				for (const Edge<dimension>& edge : boundary)
					faces << edgeIndexMap[&edge] << " ";

				faces << "\n";
			}
		}
	};

	using Mesh2D = Mesh<2>;
	using Mesh3D = Mesh<3>;
}

namespace std {
	template<size_t dimension>
	struct hash<snl::Ref<snl::Mesh<dimension>>> {
		size_t operator()(snl::Ref<snl::Mesh<dimension>> mesh) const noexcept {
			return reinterpret_cast<size_t>(mesh.raw());
		}
	};

	template<size_t dimension>
	struct hash<snl::Ref<const snl::Mesh<dimension>>> {
		size_t operator()(snl::Ref<const snl::Mesh<dimension>> mesh) const noexcept {
			return reinterpret_cast<size_t>(mesh.raw());
		}
	};
}