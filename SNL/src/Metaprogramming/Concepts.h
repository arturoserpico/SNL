#pragma once

#include <type_traits>

namespace snl {
	template<typename T, typename Target>
	concept IsTypeIgnoreCVRef = std::is_same_v<std::remove_cvref_t<T>, std::remove_cvref_t<Target>>;
}