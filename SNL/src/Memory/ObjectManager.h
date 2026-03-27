#pragma once

#include <map>
#include <sstream>
#include "../Utils/Error.h"

//#include "../Utils/Ref.h"

#ifndef SNLObjectManagerDebugLogging
	#define SNLObjectManagerDebugLogging false
#endif

namespace snl {
	template<typename T>
	class Ref;

	class ObjectManager {
		std::map<const void*, size_t> objectRegister;

		std::string formatReferenceCount(const auto* obj) {
			std::stringstream str;

			str << ", reference count: " << objectRegister.at(reinterpret_cast<const void*>(obj));

			return str.str();
		}
	public:
		static constexpr bool debugLogging = SNLObjectManagerDebugLogging;

		template<typename T>
		Ref<T> create(const T&);

		template<typename T, typename... Args>
		Ref<T> create(Args&&...);

		void addRef(const auto* obj) {
			if constexpr (debugLogging)
				debug << "adding reference to object at: " << obj << formatReferenceCount(obj) << std::endl;

			objectRegister.at(reinterpret_cast<const void*>(obj))++;
		}

		void release(const auto* obj) {
			if constexpr (debugLogging)
				debug << "releasing reference to object at: " << obj << formatReferenceCount(obj) << std::endl;

			objectRegister.at(reinterpret_cast<const void*>(obj))--;
			if (objectRegister.at(reinterpret_cast<const void*>(obj)) == 0) {
				if constexpr (debugLogging)
					debug << "deleting object at: " << obj << std::endl;

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

		auto getObjects() {
			return objectRegister;
		}
	};

	static ObjectManager objManager;
	
	void removeObjectRef(const auto* obj) {
		objManager.release(obj);
	}

	void addObjectRef(const auto* obj) {
		objManager.addRef(obj);
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