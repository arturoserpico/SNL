#pragma once

#include <iostream>
#include "Error.h"
#include "../Metaprogramming/Utils.h"

namespace snl {
	using BoundedInvalidInitializationValueError = 
		Error<"snl::Bounded initialization is out of bounds">;

	template<typename T, T min, T max>
	class Bounded {
		T value;
	public:
		Bounded() requires (min <= T() && T() <= max) = default;
		Bounded(T value) : value(value) {
			SNLDebugCall(1, expect<BoundedInvalidInitializationValueError>(min <= value && value <= max));
		}

		Bounded<T, min, max>& operator=(T value) {
			SNLDebugCall(1, expect<BoundedInvalidInitializationValueError>(min <= value && value <= max));
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
	struct _BoundedType;

	template<typename T, T min, T max>
	struct _BoundedType<Bounded<T, min, max>> : TypeAlias<T> {};

	template<typename T>
	using BoundedType = _BoundedType<T>::Type;

	template<typename T>
	constexpr auto boundedMax = 0; 

	template<typename T, T min, T max>
	constexpr auto boundedMax<Bounded<T, min, max>> = max;

	template<typename T>
	constexpr auto boundedMin = 0;

	template<typename T, T min, T max>
	constexpr auto boundedMin<Bounded<T, min, max>> = min;

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