#pragma once
#include <map>
#include <type_traits>
#include "Error.h"

namespace snl {
	void addObjectRef(const auto*);

	template<typename T>
	class Ref {
		bool managed = false;
		T* inner = nullptr;
	public:
		Ref() = default;

		Ref(T& val, bool managed) : inner(&val), managed(managed) {}
		Ref(T* val) : inner(val) {}
		Ref(T& val) : inner(&val) {}

		Ref(const Ref<T>& other) : inner(other.inner), managed(other.managed) {
			if (managed)
				addObjectRef(inner);
		}

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

		void bind(T* val) {
			inner = val;
		}

		void bind(T& val) {
			inner = &val;
		}

		friend bool operator==(Ref<T> a, Ref<T> b) {
			return a.get() == b.get();
		}

		template<typename A>
		Ref<A> as() {
			Ref<A> result;

			result.managed = managed;
			result.inner = reinterpret_cast<A*>(inner);

			if (managed)
				addObjectRef(inner);

			return result;
		}

		friend class Ref<void>;
	};

	template<>
	class Ref<void> {
		bool managed = false;
		void* inner = nullptr;
	public:
		Ref() = default;
		
		Ref(void* val) : inner(val) {}

		template<typename T>
		Ref(const Ref<T>& other) : managed(other.managed), inner(reinterpret_cast<void*>(other.inner)) {
			if (managed)
				addObjectRef(inner);
		}

		~Ref();

		void* raw() const {
			return inner;
		}

		bool empty() const {
			return inner == nullptr;
		}

		void bind(void* val) {
			inner = val;
		}

		template<typename T>
		Ref<T> as() {
			Ref<T> result;

			result.managed = managed;
			result.inner = reinterpret_cast<T*>(inner);

			if (managed)
				addObjectRef(inner);

			return result;
		}
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
