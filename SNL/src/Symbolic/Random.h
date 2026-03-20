#pragma once

#include <random>
#include <chrono>
#include "ExprOperators.h"

namespace snl {
	class RNG {
		std::mt19937_64 rng;
		std::uniform_real_distribution<double> unif;
	public:
		RNG() : unif(0, 1) {
			uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
			std::seed_seq ss{ uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed >> 32) };
			rng.seed(ss);
		}

		double get() {
			return unif(rng);
		}
	};

	static RNG rng;

	template<typename T>
	T _random(T min, T max);

	template<>
	double _random<double>(double min, double max) {
		return rng.get() * (max - min) + min;
	}

	template<>
	float _random<float>(float min, float max) {
		return rng.get() * (max - min) + min;
	}

	template<>
	int _random<int>(int min, int max) {
		return (int)_random<float>(min, max);
	}

	template<>
	size_t _random<size_t>(size_t min, size_t max) {
		return (size_t)_random<float>(min, max);
	}

	template<typename T>
	struct SymRandomGen : SymOpType<T(T, T)> {
		T eval(T min, T max) {
			return _random<T>(min, max);
		}
	};

	auto random(auto&& _min, auto&& _max) {
		auto [min, max] = convertArgsToSymRef(std::forward<decltype(_min)>(_min), std::forward<decltype(_max)>(_max));
		using Min = RemSym<RemRef<decltype(min)>>;
		using Max = RemSym<RemRef<decltype(min)>>;
		return Sym(SymRandomGen<Min>(), min, makeManaged(max.get().cast<Min>()));
	}
		 
}