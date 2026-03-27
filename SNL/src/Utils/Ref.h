#pragma once
#include <map>
#include <type_traits>
#include "../Metaprogramming/Utils.h"
#include "../Memory/ObjectManager.h"
#include "Error.h"

namespace snl {
	void addObjectRef(const auto*);
	void removeObjectRef(const auto*);

	template<typename T>
	class Ref;

	template<>
	class Ref<void> {
		bool managed;
		void* inner;

		void checkManagmentState() {
			if (objManager.find(inner) && !managed)
				debug << "unmanaged snl::Ref has been created pointing to managed object at: " << inner << std::endl;
		}
	public:
		Ref() = default;

		Ref(void* val, bool managed = false) : managed(managed), inner(val) {
			SNLDebugCall(2, checkManagmentState());

			if (managed)
				addObjectRef(inner);
		}

		Ref(const Ref<void>& other) : managed(other.managed), inner(other.inner) {
			SNLDebugCall(2, checkManagmentState());

			if (managed)
				addObjectRef(inner);
		}

		Ref<void>& operator=(const Ref<void>& other) {
			if (inner != nullptr && managed)
				removeObjectRef(inner);

			managed = other.managed;

			if (managed)
				addObjectRef(other.inner);

			inner = other.inner;

			SNLDebugCall(2, checkManagmentState());

			return *this;
		}

		~Ref() {
			if (managed)
				removeObjectRef(inner);
		}

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
		Ref<T> as() const {
			return Ref<T>(std::launder(reinterpret_cast<T*>(inner)), managed);
		}

		template<>
		Ref<void> as<void>() const {
			return Ref<void>(reinterpret_cast<void*>(inner), managed);
		}

		template<typename T>
		Ref(const Ref<T>& other) : Ref<void>(other.as<void>()) {}
	};

	template<typename T>
	class Ref {
		bool managed;
		T* inner;

		void checkManagmentState() {
			if (objManager.find(inner) && !managed)
				debug << "unmanaged snl::Ref has been created pointing to managed object at: " << inner << std::endl;
		}
	public:
		Ref() = default;

		Ref(T& val, bool managed = false) : inner(&val), managed(managed) {
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

		~Ref() {
			if (managed)
				removeObjectRef(inner);
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

		operator T& () const {
			SNLDebugCall(1, expect(!empty(), "cannot access null snl::Ref"));
			return *inner;
		}

		T& get() const {
			SNLDebugCall(1, expect(!empty(), "cannot access null snl::Ref"));
			return *inner;
		}

		//void bind(T* val) {
		//	inner = val;
		//}
		//
		//void bind(T& val) {
		//	inner = &val;
		//}

		bool isManaged() {
			return managed;
		}

		friend bool operator==(Ref<T> a, Ref<T> b) {
			return a.get() == b.get();
		}

		template<typename A>
		Ref<A> as() const {
			return Ref<A>(std::launder(reinterpret_cast<A*>(inner)), managed);
		}

		template<>
		Ref<void> as<void>() const {
			return Ref<void>(reinterpret_cast<void*>(inner), managed);
		}

		template<typename A>
		Ref<A> dyn() const {
			return Ref<A>(dynamic_cast<A*>(inner), managed);
		}

		operator Ref<const T>() const {
			return Ref<const T>(inner, managed);
		}

		friend class Ref<void>;
	};

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

	template<typename T>
	Ref<T> makeManaged(const T& val) {
		return objManager.create<T>(val);
	}

	template<typename T, typename... Args>
	Ref<T> makeManaged(Args&&... args) {
		return objManager.create<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	Ref<T> ObjectManager::create(const T& val) {
		T* obj = new T(val);
		objectRegister[obj] = 0;

		if constexpr (debugLogging)
			debug << "creating object at: " << obj << formatReferenceCount(obj) << std::endl;

		return Ref<T>(*obj, true);
	}

	template<typename T, typename... Args>
	Ref<T> ObjectManager::create(Args&&... args) {
		T* obj = new T(std::forward<Args>(args)...);
		objectRegister[obj] = 0;

		if constexpr (debugLogging)
			debug << "creating object at: " << obj << formatReferenceCount(obj) << std::endl;

		return Ref<T>(*obj, true);
	}
}

namespace std {
	template<typename T>
	struct hash<snl::Ref<T>> {
		size_t operator()(snl::Ref<T> ref) const {
			return std::hash<std::remove_cvref_t<T>>{}(ref.get());
		}
	};
}
