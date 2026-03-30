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
		std::map<const void*, std::pair<const std::type_info*, size_t>> objectRegister;
		static std::map<const std::type_info*, std::function<void(const void*)>> destructors;

		std::string formatReferenceCount(const auto* obj) {
			std::stringstream str;

			str << ", reference count: " << objectRegister.at(reinterpret_cast<const void*>(obj)).second;

			return str.str();
		}

		template<typename T>
		static void destructor(const void* obj) {
			reinterpret_cast<const T*>(obj)->~T();
		}

		template<typename T>
		void addDestructor() {
			if (!destructors.count(&typeid(T)))
				destructors[&typeid(T)] = destructor<T>;
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

			objectRegister.at(reinterpret_cast<const void*>(obj)).second++;
		}

		void release(const auto* obj) {
			if constexpr (debugLogging)
				debug << "releasing reference to object at: " << obj << formatReferenceCount(obj) << std::endl;

			auto& objInfo = objectRegister.at(reinterpret_cast<const void*>(obj));

			objInfo.second--;
			if (objInfo.second == 0) {
				if constexpr (debugLogging)
					debug << "deleting object at: " << obj << std::endl;

				destructors.at(objInfo.first)(obj);
				
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

	std::map<const std::type_info*, std::function<void(const void*)>> ObjectManager::destructors;

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