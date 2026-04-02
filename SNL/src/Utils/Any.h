#pragma once

#include <typeinfo>
#include "../Metaprogramming/Utils.h"
#include "Ref.h"
#include "../Memory/ObjectManager.h"

namespace snl {
	using AnyInvalidCastError = Error<"Casted snl::Any to invalid type">;

	template<bool enableTypeInfo = true, size_t maxStack = 8>
	class Any {
		static std::map<const std::type_info*, std::function<bool(const Any&, const Any&)>> comparators;

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

		~Any() {
			switch (storageType)
			{
			case SMALL:
				break;
			case BIG:
				bigStorage.~Ref();
				break;
			}
		}

		template<typename T>
		T& get() {
			if constexpr (enableTypeInfo)
				SNLDebugCall(1, expect<AnyInvalidCastError>(typeid(T) == *type));

			if constexpr (sizeof(T) <= 8)
				return Ref(smallStorage).as<T>().get();
			else
				return bigStorage.as<T>().get();
		}

		template<typename T>
		const T& get() const {
			if constexpr (enableTypeInfo)
				SNLDebugCall(1, expect<AnyInvalidCastError>(typeid(T) == *type));

			if constexpr (sizeof(T) <= 8)
				return Ref(smallStorage).as<const T>().get();
			else
				return bigStorage.as<const T>().get();
		}

		const std::type_info& getType() const requires enableTypeInfo {
			return *type;
		}

	private:
		template<typename T>
		static bool comparator(const Any& a, const Any& b) {
			return a.get<T>() == b.get<T>();
		}
	public:
		template<typename T>
		Any(const T& value) : type(&typeid(T)) {
			if constexpr (enableTypeInfo)
				if (!comparators.count(&typeid(T)))
					comparators[&typeid(T)] = comparator<T>;

			if constexpr (sizeof(T) <= 8) {
				storageType = SMALL;
				new(smallStorage) T(value);
			}
			else {
				storageType = BIG;
				bigStorage = makeManaged<T>(value).as<void>();
			}
		}

		friend bool operator==(const Any& a, const Any& b) requires enableTypeInfo {
			if (a.getType() != b.getType())
				return false;

			return comparators.at(&a.getType())(a, b);
		}
	};

	template<bool enableTypeInfo, size_t maxStack>
	std::map<const std::type_info*, std::function<bool(const Any<enableTypeInfo, maxStack>&, const Any<enableTypeInfo, maxStack>&)>> 
		Any<enableTypeInfo, maxStack>::comparators = {};
}