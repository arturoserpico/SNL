#pragma once

#include <iostream>
#include "Error.h"
#include "../Metaprogramming/Utils.h"

namespace snl {
	template<typename T, T min, T max>
	class Bounded {
		T value;
	public:
		Bounded() requires (min <= T() && T() <= max) = default;
		Bounded(T value) : value(value) {
			expect(min <= value && value <= max, "value is out of bounds");
		}

		Bounded<T, min, max>& operator=(T value) {
			expect(min <= value && value <= max, "value is out of bounds");
			this->value = value;
			return *this;
		}

		T get() const {
			return value;
		}

		operator T() const {
			return value;
		}
	};

	template<typename T>
	constexpr bool isBounded = false;

	template<typename T, T min, T max>
	constexpr bool isBounded<Bounded<T, min, max>> = true;

	template<typename T>
	concept IsBounded = isBounded<T>;

	template<typename T>
	struct BoundedType;

	template<typename T, T min, T max>
	struct BoundedType<Bounded<T, min, max>> : TypeAlias<T> {};

	template<typename T>
	auto convertBoundedToValue(T first, auto... rest) {
		if constexpr (sizeof...(rest) != 0) {
			auto restResult = convertBoundedToValue(rest...);
			return std::tuple_cat(std::tuple(first), restResult);
		}
		else
			return std::tuple(first);
	}

	template<typename T> requires IsBounded<std::remove_cvref_t<T>>
	auto convertBoundedToValue(T first, auto... rest) {
		if constexpr (sizeof...(rest) != 0) {
			auto restResult = convertBoundedToValue(rest...);
			return std::tuple_cat(std::tuple(first.get()), restResult);
		}
		else
			return std::tuple(first.get());
	}

	std::ostream& operator<<(std::ostream& stream, IsBounded auto val) {
		stream << val.get();
		return stream;
	}

	auto operator+(auto _a, auto _b) 
		requires ((IsBounded<decltype(_a)> || IsBounded<decltype(_b)>) && requires() { convertBoundedToValue(_a, _b); }) 
	{
		auto [a, b] = convertBoundedToValue(_a, _b);
		return a + b;
	}

	decltype(auto) operator*(auto _a, auto _b)
		requires ((IsBounded<decltype(_a)> || IsBounded<decltype(_b)>) && requires() { convertBoundedToValue(_a, _b); })
	{
		auto [a, b] = convertBoundedToValue(_a, _b);
		return a * b;
	}

	auto operator-(auto _a, auto _b)
		requires ((IsBounded<decltype(_a)> || IsBounded<decltype(_b)>) && requires() { convertBoundedToValue(_a, _b); })
	{
		auto [a, b] = convertBoundedToValue(_a, _b);
		return a - b;
	}

	auto operator/(auto _a, auto _b) 
		requires ((IsBounded<decltype(_a)> || IsBounded<decltype(_b)>) && requires() { convertBoundedToValue(_a, _b); })
	{
		auto [a, b] = convertBoundedToValue(_a, _b);
		return a / b;
	}
}