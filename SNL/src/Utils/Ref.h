#pragma once
#include <map>
#include <type_traits>
#include "../Metaprogramming/Utils.h"
//#include "../Memory/ObjectManager.h"
#include "Error.h"

namespace snl {
	void addObjectRef(const auto*);
	void removeObjectRef(const auto*);
	
	using NullRefError = Error<"tried derefercing null snl::Ref">;

	template<typename T>
	class Ref {
		bool managed;
		T* inner;

		void checkManagmentState() const;
	public:
		bool realManagmentState() const;

		Ref() = default;

		template<typename U = T> requires (!std::is_void_v<T>&& std::is_convertible_v<U*, T*>)
		Ref(U& val, bool managed = false) : inner(&val), managed(managed) {
			SNLDebugCall(2, checkManagmentState());

			if (managed)
				addObjectRef(inner);
		}
		
		Ref(T* val, bool managed = false) : inner(val), managed(managed) {
			SNLDebugCall(2, checkManagmentState());

			if (managed)
				addObjectRef(inner);
		}

		Ref(const Ref<T>& other) : inner(other.inner), managed(other.managed) {
			SNLDebugCall(2, checkManagmentState());

			if (managed)
				addObjectRef(inner);
		}

		template<typename A> requires std::is_void_v<T>
		Ref(const Ref<A>& other) : Ref<A>(other.as<A>()) {}

		~Ref() {
			if (managed)
				removeObjectRef(inner);

			inner = nullptr;
		}

		Ref<T>& operator=(const Ref<T>& other) {
			if (inner != nullptr && managed)
				removeObjectRef(inner);

			managed = other.managed;

			if (managed)
				addObjectRef(other.inner);

			inner = other.inner;

			SNLDebugCall(2, checkManagmentState());

			return *this;
		}

		T* raw() const {
			return inner;
		}

		bool empty() const {
			return inner == nullptr;
		}

		//template<typename U = T> requires !std::is_void_v<T>
		//operator U&() const {
		//	SNLDebugCall(1, expect<NullRefError>(!empty()));
		//	return *inner;
		//}

		template<typename U = T> requires !std::is_void_v<T>
		U& get() const {
			SNLDebugCall(1, expect<NullRefError>(!empty()));
			return *inner;
		}

		bool isManaged() const {
			return managed;
		}

		template<typename A>
		Ref<A> as() const {
			return Ref<A>(std::launder(reinterpret_cast<A*const>(inner)), managed);
		}

		template<typename A> requires std::is_void_v<A>
		Ref<A> as() const {
			return Ref<A>(reinterpret_cast<A*const>(inner), managed);
		}

		template<typename A>
		Ref<A> dyn() const {
			return Ref<A>(dynamic_cast<A*>(inner), managed);
		}

		void release() {
			removeObjectRef(inner);
			managed = false;
			inner = nullptr;
		}

		operator Ref<const T>() const {
			return Ref<const T>(inner, managed);
		}

		Ref<const T> asConst() const {
			return Ref<const T>(inner, managed);
		}
	};

	template<typename T>
	Ref(T&) -> Ref<T>;

	template<typename T>
	Ref(T&, bool) -> Ref<T>;

	template<typename A, typename B> requires requires(A a, B b) { { a == b } -> std::same_as<bool>; }
	bool operator==(Ref<A> a, Ref<B> b) {
		return a.get() == b.get();
	}

	template<typename T>
	struct _RemRef;

	template<typename T>
	struct _RemRef<Ref<T>> : TypeAlias<T> {};

	template<typename T>
	using RemRef = _RemRef<T>::Type;

	template<typename T>
	struct _SafeRemRef : TypeAlias<void> {};

	template<typename T>
	struct _SafeRemRef<Ref<T>> : TypeAlias<T> {};

	template<typename T>
	using SafeRemRef = _SafeRemRef<T>::Type;
}

namespace std {
	template<typename T>
	struct hash<snl::Ref<T>> {
		size_t operator()(snl::Ref<T> ref) const {
			return std::hash<std::remove_cvref_t<T>>{}(ref.get());
		}
	};
}
