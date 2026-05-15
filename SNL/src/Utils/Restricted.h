#pragma once
#include "Error.h"
#include <type_traits>

namespace snl {
	using InvalidRestrictedError = Error<"Tried to set a Restricted to a not allowed value">;

	template<typename T, auto condition, typename Error = InvalidRestrictedError> 
	class Restricted {
	protected:
		T value;
	public:
		using Type = T;

		void check() const {
			expect<Error>(condition(value));
		}

		Restricted() : value() {
			check();
		};

		Restricted(T value) : value(value) {
			check();
		}

		Restricted(const Restricted<T, condition>& other) : value(other.value) {}

		Restricted<T, condition>& operator=(T value) {
			this->value = value;
			check();
			return *this;
		}

		T get() {
			return value;
		}

		operator T() {
			return get();
		}
	};
}