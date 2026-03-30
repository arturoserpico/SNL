#pragma once

#include <typeinfo>
#include "../Metaprogramming/Utils.h"
#include "Ref.h"
#include "../Memory/ObjectManager.h"
#include "../Utils/ErasedFunction.h"
#include <initializer_list>

namespace snl {
	template<bool enableTypeInfo = true, size_t maxStack = 8>
	class Any {
		using DefaultString = std::string;

		template<typename T>
		using DefaultList = std::vector<T>;

		std::conditional_t<enableTypeInfo, const std::type_info*, Empty> type;

		enum { SMALL, BIG } storageType;

		union {
			std::byte smallStorage[maxStack];
			Ref<void> bigStorage;
		};
	public:
		Any() = default;

		Any(const Any& other) : type(other.type), storageType(other.storageType) {
			switch (storageType)
			{
			case SMALL:
				std::memcpy(smallStorage, other.smallStorage, maxStack);
				break;
			case BIG:
				bigStorage = other.bigStorage;
				break;
			}
		}

		Any(const char* str) : Any(DefaultString(str)) {}
		
		template<typename T>
		Any(std::initializer_list<T> list) : Any(DefaultList<T>(list)) {}

		Any& operator=(const Any& other) {
			type = other.type;
			storageType = other.storageType;

			switch (storageType)
			{
			case SMALL:
				std::memcpy(smallStorage, other.smallStorage, maxStack);
				break;
			case BIG:
				bigStorage = other.bigStorage;
				break;
			}

			return *this;
		}

		template<typename T>
		T& get() {
			if constexpr (enableTypeInfo)
				SNLDebugCall(1, expect(typeid(T) == *type, "casted snl::Any to invalid type"));

			if constexpr (sizeof(T) <= 8)
				return Ref(smallStorage).as<T>().get();
			else
				return bigStorage.as<T>().get();
		}

		template<typename T>
		const T& get() const {
			if constexpr (enableTypeInfo)
				SNLDebugCall(1, expect(typeid(T) == *type, "casted snl::Any to invalid type"));

			if constexpr (sizeof(T) <= 8)
				return Ref(smallStorage).as<const T>().get();
			else
				return bigStorage.as<const T>().get();
		}

		const std::type_info& getType() const requires enableTypeInfo {
			return *type;
		}

		Ref<const void> raw() const {
			return storageType == SMALL ? Ref(smallStorage).as<const void>() : bigStorage.as<const void>();
		}

		template<typename T>
		Any(const T& value) : type(&typeid(T)) {
			if constexpr (enableTypeInfo) {
				globalErasedDestructors.addVariant<const T>([](const T& obj) { obj.~T(); });
				globalErasedComparators.addVariant<const T, const T>([](const T& a, const T& b) { return a == b; });
			}

			if constexpr (sizeof(T) <= 8) {
				storageType = SMALL;
				new(smallStorage) T(value);
			}
			else {
				storageType = BIG;
				bigStorage = makeManaged<T>(value).as<void>();
			}
		}

		~Any() {
			globalErasedDestructors.call({ type }, { raw() });
		}

		friend bool operator==(const Any& a, const Any& b) requires enableTypeInfo {
			if (a.getType() != b.getType())
				return false;

			return globalErasedComparators.call({ &a.getType(), &b.getType() }, { a.raw(), b.raw() });
		}
	};
}