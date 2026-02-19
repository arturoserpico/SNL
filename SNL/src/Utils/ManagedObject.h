#pragma once

#include "Ref.h"

namespace snl {
	//Should only be allocated on heap, as stack allocation causes undefined behaviur
	template<typename T>
	class ManagedObject {
		T inner;
		size_t refCount = 0;

		static std::map<T*, ManagedObject<T>*> objectRegister;
	public:
		ManagedObject() : inner() {
			objectRegister.emplace(&inner, this);
		}

		template<typename... Args>
		ManagedObject(Args&&... args) : inner(std::forward<Args>(args)...) {
			objectRegister.emplace(&inner, this);
		}

		~ManagedObject() {
			objectRegister.erase(&inner);
		}

		Ref<T> ref() {
			refCount++;
			return Ref<T>(inner, true);
		}

		void release() {
			refCount--;
			if (refCount == 0)
				delete this;
		}

		friend class Ref<T>;
	};

	template<typename T>
	std::map<T*, ManagedObject<T>*> ManagedObject<T>::objectRegister = {};

	template<typename T>
	Ref<T> makeManaged(const T& val) {
		auto obj = new ManagedObject<T>(val);
		return obj->ref();
	}

	template<typename T, typename... Args>
	Ref<T> makeManaged(Args&&... args) {
		auto obj = new ManagedObject<T>(std::forward<Args>(args)...);
		return obj->ref();
	}

	template<typename T>
	Ref<T>::~Ref() {
		if (managed)
			ManagedObject<T>::objectRegister.at(inner)->release();
	}

	template<typename T>
	Ref<T>::Ref(const Ref<T>& other) : inner(other.inner), managed(other.managed) {
		if (managed)
			ManagedObject<T>::objectRegister.at(inner)->refCount++;
	}
}