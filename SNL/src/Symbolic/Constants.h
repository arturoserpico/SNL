#pragma once

namespace snl {
	template<typename T>
 	constexpr size_t decimalPrecision;

	template<>
	constexpr size_t decimalPrecision<int> = 0;

	template<>
	constexpr size_t decimalPrecision<float> = 8;

	template<>
	constexpr size_t decimalPrecision<double> = 16;

	template<typename T, size_t precision = decimalPrecision<T>> requires (precision <= decimalPrecision<T>)
	T epsilon;

	template<size_t precision> requires (precision <= decimalPrecision<double>)
	double epsilon<double, precision> = std::pow(10, -int(precision));
}