#pragma once

#include <cmath>

namespace snl {
	template<typename A, typename B> requires requires(A a, B b) { std::pow(a, b); }
	auto pow(A a, B b) {
		return std::pow(a, b);
	}
}