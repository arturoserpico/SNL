#pragma once

#include "TypeBase.h"
#include "../Metaprogramming/Concepts.h"

namespace snl {
	struct Nat : public snl::TypeBase<Nat> {
		static inline Constructor<Nat()> zero;
		static inline Constructor<Nat(Nat)> succ;

		Nat() = default;

		Nat(size_t n) {
			*this = zero;

			for (size_t i = 0; i < n; i++)
				*this = succ(*this);
		}

		size_t toInt() const {
			return snl::match(*this,
				Nat::zero >> []() { return 0ull; },
				Nat::succ >> [](Nat n) { return n.toInt() + 1; }
			);
		}
	};

	std::ostream& operator<<(std::ostream& stream, const Nat& n) {
		return stream << n.toInt();
	}

	namespace literals {
		Nat operator""_nat(size_t n) {
			return Nat(n);
		}
	}

	Nat operator+(Nat a, Nat b) {
		return match(b,
			Nat::zero >> [&]() { return a; },
			Nat::succ >> [&](Nat n) { return Nat::succ(a) + n; }
		);
	}

	template<typename... Ts>
	struct Tuple : public snl::TypeBase<Tuple<Ts...>> {
		static inline Constructor<Tuple(Ts...)> tuple;
		
		Tuple() = default;
		
		Tuple(Ts... args) {
			*this = tuple(args...);
		}
	};

	template<typename A, typename B>
	using Pair = Tuple<A, B>;

	template<typename... Ts>
	class Union : public snl::TypeBase<Union<Ts...>> {
		static const inline std::tuple<Constructor<Union(Ts)>...> constructors;
		
	public:
		template<IsOneOf<TypeList<Ts...>> T>
		static inline const Constructor<Union(T)>& constructor = 
			std::get<find<TypeList<Ts...>, T>>(constructors);
		
		Union() = default;

		template<IsOneOf<TypeList<Ts...>> T>
		Union(T val) {
			*this = constructor<T>()(val);
		}

		auto match(auto... callables) const {
			using R = decltype(snl::match(*this, (constructor<Ts> >> callables)...));

			if constexpr (std::is_same_v<R, void>) {
				snl::match(*this, (constructor<Ts> >> callables)...);
			}
			else {
				return snl::match(*this, (constructor<Ts> >> callables)...);
			}
		}
	};
}