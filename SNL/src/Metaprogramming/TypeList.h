#pragma once

#include <type_traits>
#include "Utils.h"
#include "StaticString.h"

namespace snl {
	template<typename... Ts>
	struct TypeList {};



	template<typename List>
	constexpr size_t lenght = 0;

	template<>
	constexpr size_t lenght<TypeList<>> = 0;

	template<typename First, typename... Rest>
	constexpr size_t lenght<TypeList<First, Rest...>> = lenght<TypeList<Rest...>> + 1;



	template<typename List, size_t index>
	struct _Get;

	template<typename First, typename... Rest, size_t index>
	struct _Get<TypeList<First, Rest...>, index> : _Get<TypeList<Rest...>, index - 1> {};

	template<size_t index>
	struct _Get<TypeList<>, index> {
		static_assert(false, "index out of bounds when indexing TypeList");
	};

	template<typename First, typename... Rest>
	struct _Get<TypeList<First, Rest...>, 0> : TypeAlias<First> {};

	template<typename List, size_t index>
	using Get = _Get<List, index>::Type;



	template<typename Element, typename List>
	struct _Prepend;

	template<typename Element, typename... Ts>
	struct _Prepend<Element, TypeList<Ts...>> : TypeAlias<TypeList<Element, Ts...>> {};

	template<typename Element, typename List>
	using Prepend = typename _Prepend<Element, List>::Type;




}