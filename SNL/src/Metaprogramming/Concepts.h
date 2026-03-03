#pragma once

#include <type_traits>
#include "TypeList.h"

namespace snl {
	template<typename T, typename Target>
	concept IsTypeIgnoreCVRef = std::is_same_v<std::remove_cvref_t<T>, std::remove_cvref_t<Target>>;

	template<typename T, typename List>
	concept IsOneOf = contains<List, T>;

	template<typename T, typename Target>
	concept IsRefOrConstRef = IsOneOf<T, TypeList<Target&, const Target&>>;
}