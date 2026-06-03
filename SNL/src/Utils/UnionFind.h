#pragma once

#include <unordered_map>
#include "Error.h"

namespace snl {
	template<typename T>
	class UnionFind {
		std::unordered_map<T, T> parent;

		UnionFind& fix(const T& elem) {
			parent[elem] = find(parent[elem]);
			return *this;
		}
	public:
		UnionFind() = default;
		UnionFind(const std::unordered_map<T, T>& graph) : parent(graph) {}

		size_t size() {
			return parent.size();
		}

		UnionFind& add(const T& elem, const T& parent) {
			this->parent.emplace(elem, parent);
			fix(elem);
			return *this;
		}

		UnionFind& add(const T& elem) {
			return add(elem, elem);
		}

		const T& find(const T& elem) const {
			SNLDebugCall(1, expect<OutOfRangeError>(parent.find(elem) != parent.end(), "UnionFind"));
			if (parent.at(elem) == elem) return elem;
			else return find(parent.at(elem));
		}

		UnionFind& merge(const T& a, const T& b) {
			parent[find(a)] = find(b);
			fix(a);
			fix(b);
			return *this;
		}
	};
}