#pragma once

#include "AlgebraicBase.h"
#include "../Symbolic/Sym.h"
#include "../Symbolic/MathContext.h"

namespace snl {
	template<typename T>
	struct Set : public AlgebraicBase<Set<T>> {
		static inline const Constructor<Set<T>()> empty;
		static inline const Constructor<Set<T>(std::unordered_set<T>)> finite;
		static inline const Constructor<Set<T>(Set<T>, Set<T>)> unionSet;
		static inline const Constructor<Set<T>(Set<T>, Set<T>)> intersectionSet;
		static inline const Constructor<Set<T>(std::unordered_set<Sym<T>>)> patterned;

		bool contains(const Sym<T>& sym, MatchResult& result) const {
			return match(*this,
				Set::empty >> [&]() { return false; },
				Set::finite >> [&](std::unordered_set<T> vals) {
					for (T val : vals)
						if (symMatch(sym, Sym(val), result))
							return true;

					return false;
				},
				Set::unionSet >> [&](Set<T> a, Set<T> b) { return a.contains(sym, result) || b.contains(sym, result); },
				Set::intersectionSet >> [&](Set<T> a, Set<T> b) { return a.contains(sym, result) && b.contains(sym, result); },
				Set::patterned >> [&](std::unordered_set<Sym<T>> patterns) {
					for (Sym<T> pattern : patterns)
						if (symMatch(pattern, sym, result))
							return true;

					return false;
				}
			);
		}

		bool contains(const T& val, MatchResult& result) const {
			return match(*this,
				Set::empty >> [&]() { return false; },
				Set::finite >> [&](std::unordered_set<T> vals) { return vals.contains(val); },
				Set::unionSet >> [&](Set<T> a, Set<T> b) { return a.contains(val, result) || b.contains(val, result); },
				Set::intersectionSet >> [&](Set<T> a, Set<T> b) { return a.contains(val, result) && b.contains(val, result); },
				Set::patterned >> [&](std::unordered_set<Sym<T>> patterns) {
					for (Sym<T> pattern : patterns)
						if (symMatch(pattern, Sym(val), result))
							return true;

					return false;
				}
			);
		}

		bool contains(const Sym<T>& sym) const {
			MatchResult result;
			return contains(sym, result);
		}

		bool contains(const T& val) const {
			MatchResult result;
			return contains(val, result);
		}

		Set() = default;

		Set(IsTypeObject auto) : Set() {}

		Set(std::initializer_list<T> vals) {
			*this = finite(std::unordered_set<T>(vals));
		}

		Set(std::initializer_list<Sym<T>> vals) {
			*this = patterned(std::unordered_set<Sym<T>>(vals));
		}
	};

	template<IsTypeObject TypeObject>
	Set(TypeObject) -> Set<typename TypeObject::Type>;

	template<typename T>
	Set<T> operator|(const Set<T>& a, const Set<T>& b) {
		return Set<T>::unionSet(a, b);
	}

	template<typename T>
	Set<T> operator&(const Set<T>& a, const Set<T>& b) {
		return Set<T>::intersectionSet(a, b);
	}
}