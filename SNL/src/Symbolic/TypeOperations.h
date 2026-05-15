#pragma once

namespace snl {
	template<typename T>
	struct TypeObject {
		using Type = T;
	};

	template<typename TypeA, typename TypeB>
	consteval TypeObject<std::tuple<typename TypeA::Type, typename TypeB::Type>> operator*(TypeA, TypeB) {
		return {};
	}

	template<typename... TypesA, typename TypeB>
	consteval TypeObject<std::tuple<typename TypesA..., typename TypeB::Type>> operator*(TypeObject<std::tuple<TypesA...>>, TypeB) {
		return {};
	}

	template<typename TypeA, typename... TypesB>
	consteval TypeObject<std::tuple<typename TypeA::Type, TypesB...>> operator*(TypeA, TypeObject<std::tuple<TypesB...>>) {
		return {};
	}

	template<typename... TypesA, typename... TypesB>
	consteval TypeObject<std::tuple<TypesA..., TypesB...>> operator*(TypeObject<std::tuple<TypesA...>>, TypeObject<std::tuple<TypesB...>>) {
		return {};
	}

	template<auto type, size_t n>
	auto cartesianPow = type * cartesianPow<type, n - 1>;

	template<auto type>
	auto cartesianPow<type, 1> = type;
}