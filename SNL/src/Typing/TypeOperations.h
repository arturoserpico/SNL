#pragma once

#include "AlgebraicTypes.h"

namespace snl {
	template<typename T>
	struct TypeObject {
		using Type = T;
	};

	template<IsTypeObject TypeA, IsTypeObject TypeB>
	consteval TypeObject<Tuple<typename TypeA::Type, typename TypeB::Type>> operator*(TypeA, TypeB) {
		return {};
	}

	template<typename... TypesA, IsTypeObject TypeB>
	consteval TypeObject<Tuple<typename TypesA..., typename TypeB::Type>> operator*(TypeObject<Tuple<TypesA...>>, TypeB) {
		return {};
	}

	template<IsTypeObject TypeA, typename... TypesB>
	consteval TypeObject<Tuple<typename TypeA::Type, TypesB...>> operator*(TypeA, TypeObject<Tuple<TypesB...>>) {
		return {};
	}

	template<typename... TypesA, typename... TypesB>
	consteval TypeObject<Tuple<TypesA..., TypesB...>> operator*(TypeObject<Tuple<TypesA...>>, TypeObject<Tuple<TypesB...>>) {
		return {};
	}

	template<auto type, size_t n>
	auto cartesianPow = type * cartesianPow<type, n - 1>;

	template<auto type>
	auto cartesianPow<type, 1> = type;

	TypeObject<Nat> nat;
	TypeObject<int> cInt;
}