#pragma once
#include <map>
#include <type_traits>
#include "../Metaprogramming/Utils.h"
#include "Error.h"

namespace snl {
	void addObjectRef(const auto*);

	template<typename T>
	class Ref {
		bool managed = false;
		T* inner = nullptr;
	public:
		Ref() = default;

		Ref(T& val, bool managed = false) : inner(&val), managed(managed) {
			if (managed)
				addObjectRef(inner);
		}
		
		Ref(T* val, bool managed = false) : inner(val), managed(managed) {
			if (managed)
				addObjectRef(inner);
		}

		Ref(const Ref<T>& other) : inner(other.inner), managed(other.managed) {
			if (managed)
				addObjectRef(inner);
		}

		~Ref();

		Ref<T>& operator=(const Ref<T>& other) {
			inner = other.inner;
			managed = other.managed;

			if (managed)
				addObjectRef(inner);

			return *this;
		}

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

		bool isManaged() {
			return managed;
		}

		friend bool operator==(Ref<T> a, Ref<T> b) {
			return a.get() == b.get();
		}

		template<typename A>
		Ref<A> as() {
			return Ref<A>(std::launder(reinterpret_cast<T*>(inner)), managed);
		}

		template<typename A>
		Ref<const A> as() const {
			return Ref<const A>(std::launder(reinterpret_cast<const A*>(inner)), managed);
		}

		template<typename A>
		Ref<A> dyn() {
			return Ref<A>(dynamic_cast<A*>(inner), managed);
		}

		template<typename A>
		Ref<const A> dyn() const {
			return Ref<const A>(dynamic_cast<const A*>(inner), managed);
		}
	};

	template<>
	class Ref<void> {
		bool managed = false;
		void* inner = nullptr;
	public:
		Ref() = default;
		
		Ref(void* val, bool managed = false) : managed(managed), inner(val) {
			if (managed)
				addObjectRef(inner);
		}

		template<typename T>
		Ref(const Ref<T>& other) : managed(other.managed), inner(reinterpret_cast<void*>(other.inner)) {
			if (managed)
				addObjectRef(inner);
		}

		Ref(const Ref<void>& other) : managed(other.managed), inner(other.inner) {
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

		bool isManaged() {
			return managed;
		}

		template<typename T>
		Ref<T> as() {
			return Ref<T>(std::launder(reinterpret_cast<T*>(inner)), managed);
		}

		template<typename T>
		Ref<const T> as() const {
			return Ref<const T>(std::launder(reinterpret_cast<const T*>(inner)), managed);
		}
	};

	template<typename T>
	struct _RemRef;

	template<typename T>
	struct _RemRef<Ref<T>> : TypeAlias<T> {};

	template<typename T>
	using RemRef = _RemRef<T>::Type;
}

namespace std {
	template<typename T>
	struct hash<snl::Ref<T>> {
		size_t operator()(snl::Ref<T> ref) const {
			return std::hash<std::remove_cvref_t<T>>{}(ref.get());
		}
	};
}
