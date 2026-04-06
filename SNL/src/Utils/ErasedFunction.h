#pragma once

#include <typeinfo>
#include <map>
#include <functional>
#include "../Metaprogramming/Utils.h"
#include "../Metaprogramming/TypeList.h"
#include "Ref.h"

namespace snl {
	template<bool enableTypeInfo, size_t maxStack>
	class Any;

	template<typename T>
	constexpr bool isAny = false;

	template<bool enableTypeInfo, size_t maxStack>
	constexpr bool isAny<Any<enableTypeInfo, maxStack>> = true;

	template<typename T>
	concept IsAny = isAny<T>;

	template<typename R, typename Erased = void, size_t argsCount = 1>
	class ErasedFunction {
		std::map<std::array<const std::type_info*, argsCount>, std::function<R(std::array<Ref<Erased>, argsCount>)>> functions;
	
		template<typename... Args> requires (sizeof...(Args) == argsCount)
		static std::tuple<Args&...> caster(const std::array<Ref<Erased>, argsCount>& args)
		{
			return[&]<std::size_t... Is>(std::index_sequence<Is...>) {
				return std::tuple<Args&...>{
					(args[Is].template as<Args>().get())...
				};
			}(std::make_index_sequence<sizeof...(Args)>{});
		}
	public:
		template<typename... Args> requires (sizeof...(Args) == argsCount)
		void addVariant(std::function<R(Args&...)> variant) {
			std::array types{ &typeid(Args)... };

			if (!functions.count(types))
				functions[types] = [variant](const std::array<Ref<Erased>, argsCount>& args) {
				auto tuple = caster<Args...>(args);
				return std::apply(variant, tuple);
				};
		}

		template<typename... Args, typename... FunArgs> 
			requires (sizeof...(Args) == argsCount && sizeof...(FunArgs) == argsCount)
		void addVariant(std::function<R(FunArgs&...)> variant) {
			std::array types{ &typeid(Args)... };
			
			if (!functions.count(types))
				functions[types] = [variant](const std::array<Ref<Erased>, argsCount>& args) {
					auto tuple = caster<FunArgs...>(args);
					return std::apply(variant, tuple);
				};
		}

		R call(
			const std::array<const std::type_info*, argsCount>& type,
			const std::array<Ref<Erased>, argsCount>& args
		) {
			return functions.at(type)(args);
		}

		template<typename... Args> requires (sizeof...(Args) == argsCount)
		R call(Args&&... args) {
			return call({ &typeid(Args)... }, { Ref(args).as<Erased>()... });
		}

		template<IsAny... Args> requires (sizeof...(Args) == argsCount)
		R call(Args&&... args);
	};

	static ErasedFunction<std::function<void(void*)>, const void> globalErasedSizeof;
	static ErasedFunction<std::function<void(void*)>, const void> globalErasedCopyAssignment;
	static ErasedFunction<std::function<void(void*)>, const void> globalErasedCopyConstructor;
	static ErasedFunction<void, const void> globalErasedDestructors;
	static ErasedFunction<bool, const void, 2> globalErasedComparators;
}