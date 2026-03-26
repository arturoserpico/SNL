#pragma once

#include "../Utils/Ref.h"

namespace snl {
	class ObjectManager {
		std::map<const void*, size_t> objectRegister;
	public:
		template<typename T>
		Ref<T> create(const T& val) {
			T* obj = new T(val);
			objectRegister[obj] = 0;
			return Ref<T>(*obj, true);
		}

		template<typename T, typename... Args>
		Ref<T> create(Args&&... args) {
			T* obj = new T(std::forward<Args>(args)...);
			objectRegister[obj] = 0;
			return Ref<T>(*obj, true);
		}

		void addRef(const auto* obj) {
			objectRegister.at(reinterpret_cast<const void*>(obj))++;
		}

		void release(const auto* obj) {
			objectRegister.at(reinterpret_cast<const void*>(obj))--;
			if (objectRegister.at(reinterpret_cast<const void*>(obj)) == 0) {
				objectRegister.erase(reinterpret_cast<const void*>(obj));
				delete obj;
			}
		}

		bool find(const auto* obj) {
			return objectRegister.count(reinterpret_cast<const void*>(obj));
		}

		size_t count() {
			return objectRegister.size();
		}
	};

	static ObjectManager objManager;
	
	void removeObjectRef(const auto* obj) {
		objManager.release(obj);
	}

	void addObjectRef(const auto* obj) {
		objManager.addRef(obj);
	}

	template<typename T>
	Ref<T> makeManaged(const T& val) {
		return objManager.create<T>(val);
	}

	template<typename T, typename... Args>
	Ref<T> makeManaged(Args&&... args) {
		return objManager.create<T>(std::forward<Args>(args)...);
	}

	//template<typename T>
	//Ref<T>::~Ref() {
	//	if (managed)
	//		objManager.release(inner);
	//}
	//
	//Ref<void>::~Ref() {
	//	if (managed)
	//		objManager.release(inner);
	//}
}