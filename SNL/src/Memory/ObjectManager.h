#pragma once

#include <map>
#include <sstream>
#include "../Utils/Error.h"
#include "../Utils/ErasedFunction.h"
#include "../Utils/Ref.h"
#include "../Utils/Error.h"

#ifndef SNLObjectManagerDebugLogging
	#define SNLObjectManagerDebugLogging false
#endif

namespace snl {
	template<typename T>
	class Ref;

	using UnmanagedRefToManagedObjWarning =
		Warning<"unmanaged snl::Ref has been created pointing to managed object at: {}", const void*>;

	class ObjectManager {
		std::map<const void*, std::pair<const std::type_info*, size_t>> objectRegister;

		std::string formatReferenceCount(const auto* obj) {
			std::stringstream str;

			str << ", reference count: " << objectRegister.at(reinterpret_cast<const void*>(obj)).second;

			return str.str();
		}
	public:
		static constexpr bool debugLogging = SNLObjectManagerDebugLogging;

		template<typename T>
		Ref<T> create(const T& val) {
			globalErasedDestructors.addVariant<const T>([](const T& obj) { obj.~T(); });

			T* obj = new T(val);
			objectRegister[obj] = { &typeid(T), 0 };

			if constexpr (debugLogging)
				debug << "creating object at: " << obj << formatReferenceCount(obj) << std::endl;

			return Ref<T>(*obj, true);
		}

		template<typename T, typename... Args>
		Ref<T> create(Args&&... args) {
			globalErasedDestructors.addVariant<const T>([](const T& obj) { obj.~T(); });

			T* obj = new T(std::forward<Args>(args)...);
			objectRegister[obj] = { &typeid(T), 0 };

			if constexpr (debugLogging)
				debug << "creating object at: " << obj << formatReferenceCount(obj) << std::endl;

			return Ref<T>(*obj, true);
		}

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
				ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;

				if constexpr (debugLogging)
					debug << "deleting object at: " << obj << std::endl;

				globalErasedDestructors.call({ objInfo.first }, { Ref<const void>(obj) });
				
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

	template<typename T>
	Ref<T> makeManaged(const T& val) {
		return objManager.create<T>(val);
	}

	template<typename T, typename... Args>
	Ref<T> makeManaged(Args&&... args) {
		return objManager.create<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	void Ref<T>::checkManagmentState() {
		if (objManager.find(inner) && !managed)
			throwError<UnmanagedRefToManagedObjWarning>(inner);
			
	}

	void Ref<void>::checkManagmentState() {
		if (objManager.find(inner) && !managed)
			throwError<UnmanagedRefToManagedObjWarning>(inner);
	}
}