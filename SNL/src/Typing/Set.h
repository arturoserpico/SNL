#pragma once

#include "TypeBase.h"
#include "../Symbolic/Sym.h"

namespace snl {
	template<typename T>
	struct Set : public TypeBase<Set<T>> {
		static inline const Constructor<Set<T>()> empty;
		static inline const Constructor<Set<T>(std::unordered_set<T>)> finite;
		static inline const Constructor<Set<T>(Set<T>, Set<T>)> unionSet;
		static inline const Constructor<Set<T>(Set<T>, Set<T>)> intersectionSet;
		static inline const Constructor<Set<T>(std::unordered_set<Sym<T>>)> patterned;

		bool contains(const T& val) {
			return match(*this,
				Set::empty >> [&]() { return false; },
				Set::finite >> [&](std::unordered_set<T> vals) { return vals.contains(val); },
				Set::unionSet >> [&](Set<T> a, Set<T> b) { return a.contains(val) || b.contains(val); },
				Set::intersectionSet >> [&](Set<T> a, Set<T> b) { return a.contains(val) && b.contains(val); },
				Set::patterned >> [&](std::unordered_set<Sym<T>> patterns) {
					//for (Sym<T> pattern : patterns)
					//	if (pattern.get().eval() == val)
					//		return true;
					return false;
				}
			);
		}

		Set() = default;

		Set(std::initializer_list<T> vals) {
			*this = finite(std::unordered_set<T>(vals));
		}

		Set(std::initializer_list<Sym<T>> vals) {
			*this = patterned(std::unordered_set<T>(vals));
		}
	};

	template<typename T>
	Set<T> operator|(const Set<T>& a, const Set<T>& b) {
		return Set<T>::unionSet(a, b);
	}

	template<typename T>
	Set<T> operator&(const Set<T>& a, const Set<T>& b) {
		return Set<T>::intersectionSet(a, b);
	}
}