#pragma once

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

		operator T() const {
			return value;
		}
	};
}