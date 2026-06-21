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

	Nat operator+(const Nat& a, const Nat& b) {
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

	template<typename T>
	struct List : public snl::TypeBase<List<T>> {
		static inline Constructor<List<T>()> empty;
		static inline Constructor<List<T>(T, List<T>)> prepend;

		List() = default;

		List(std::initializer_list<T> elems) {
			*this = empty;

			for (auto it = std::rbegin(elems); it != std::rend(elems); it++)
				*this = prepend(*it, *this);
		}

		T& operator[](size_t index) {	
			match(*this,
				empty >> [&]() { forceThrow<OutOfRangeError>("snl::List"); }
			);

			return match(*this,
				prepend >> [&](T& first, List<T>& rest) -> T& {
					if (index == 0)
						return first;
					else
						return rest[index - 1];
				}
			);
		}

		const T& operator[](size_t index) const {
			match(*this,
				empty >> [&]() { forceThrow<OutOfRangeError>("snl::List"); }
			);

			return match(*this,
				prepend >> [&](const T& first, const List<T>& rest) -> const T& {
					if (index == 0)
						return first;
					else
						return rest[index - 1];
				}
			);
		}

		size_t size() const {
			return match(*this,
				empty >> [&]() { return 0ull; },
				prepend >> [&](T first, List<T> rest) { return 1 + rest.size(); }
			);
		}
	};

	template<typename T>
	List<T> operator+(const List<T>& a, const List<T>& b) {
		return match(a,
			List<T>::empty >> [&]() { return b; },
			List<T>::prepend >> [&](T first, List<T> rest) { return List<T>::prepend(first, rest + b); }
		);
	}
}