#pragma once

#include <map>
#include <sstream>
#include "../Utils/Error.h"
#include "../Utils/ErasedFunction.h"
#include "../Utils/Ref.h"
#include "../Utils/Error.h"
#include <set>

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
		Debug<1, std::map<const void*, std::string>> tracked;

		std::string formatReferenceCount(const auto* obj) {
			std::stringstream str;

			str << ", reference count: " << objectRegister.at(reinterpret_cast<const void*>(obj)).second;

			return str.str();
		}

		bool isTracked(const void* obj) {
			if constexpr (debugLevel >= 1)
				return tracked.count(obj);
			else
				return false;
		}

		std::string trackedStr(const auto* obj) {
			if (isTracked(obj))
				return std::format("[tracked ref: {}] ", tracked.at(obj));
			else
				return "";
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

		Ref<void> createErased(const std::type_info& type, size_t size) {
			void* obj = operator new(size);
			objectRegister[obj] = { &type, 0 };
			if constexpr (debugLogging)
				debug << "creating object at: " << obj << formatReferenceCount(obj) << std::endl;
			return Ref<void>(obj, true);
		}

		void addRef(const auto* obj) {
			objectRegister.at(reinterpret_cast<const void*>(obj)).second++;

			if (debugLogging || isTracked(obj))
				debug << trackedStr(obj) << "adding reference to object at: " << obj << formatReferenceCount(obj) << std::endl;
		}

		void release(const auto* obj) {
			auto& objInfo = objectRegister.at(reinterpret_cast<const void*>(obj));

			objInfo.second--;

			if (debugLogging || isTracked(obj))
				debug << trackedStr(obj) << "releasing reference to object at: " << obj << formatReferenceCount(obj) << std::endl;

			if (objInfo.second == 0) {
				ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;

				if (debugLogging || isTracked(obj))
					debug << trackedStr(obj) << "deleting object at: " << obj << std::endl;

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

		void track(const auto* obj, const std::string& name) {
			if constexpr (debugLevel >= 1)
				tracked[obj] = name;
		}
	};

	static ObjectManager objManager;
	
	template<typename T>
	Ref<T> track(Ref<T> ref, const std::string& name) {
		objManager.track(ref.raw(), name);
		return ref;
	}

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

	Ref<void> makeManagedErased(const std::type_info& type, size_t size) {
		return objManager.createErased(type, size);
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