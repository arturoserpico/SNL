#pragma once
#include <map>
#include <type_traits>
#include "Error.h"

namespace snl {
	template<typename T>
	class ManagedObject;

	template<typename T>
	class Ref {
		bool managed = false;
		T* inner = nullptr;

		Ref(T& val, bool managed) : inner(&val), managed(managed) {}
	public:
		Ref() = default;

		Ref(T* val) : inner(val) {}
		Ref(T& val) : inner(&val) {}
		
		Ref(const Ref<T>&);

		~Ref();

		T* raw() const {
			return inner;
		}

		bool empty() const {
			return inner == nullptr;
		}

		operator T& () const {
			expect(!empty(), "cannot access null snl::Ref");
			return *inner;
		}

		T& get() const {
			expect(!empty(), "cannot access null snl::Ref");
			return *inner;
		}

		void bind(T& val) {
			inner = &val;
		}

		friend bool operator==(Ref<T> a, Ref<T> b) {
			return a.get() == b.get();
		}

		friend class ManagedObject<T>;
	};
}

namespace std {
	template<typename T>
	struct hash<snl::Ref<T>> {
		size_t operator()(snl::Ref<T> ref) const {
			return std::hash<std::remove_cvref_t<T>>{}(ref.get());
		}
	};
}
