#pragma once

#include <typeinfo>
#include <map>
#include <functional>
#include "../Metaprogramming/Utils.h"
#include "../Metaprogramming/TypeList.h"
#include "Ref.h"

namespace snl {
	template<typename R, size_t argsCount = 1>
	class ErasedFunction {
		std::map<std::array<const std::type_info*, argsCount>, std::function<R(std::array<Ref<void>, argsCount>)>> functions;
	
		template<typename... Args> requires (sizeof...(Args) == argsCount)
		static std::tuple<Args&...> caster(const std::array<Ref<void>, argsCount>& args)
		{
			return[&]<std::size_t... Is>(std::index_sequence<Is...>) {
				return std::tuple<Args&...>{
					(args[Is].template as<Args>().get())...
				};
			}(std::make_index_sequence<sizeof...(Args)>{});
		}
	public:
		template<typename... Args> requires (sizeof...(Args) == argsCount)
		void addVariant(std::function<R(Args...)> variant) {
			std::array types{ &typeid(Args)... };
			
			if (!functions.count(types))
				functions[types] = [variant](const std::array<Ref<void>, argsCount>& args) {
					auto tuple = caster<Args...>(args);
					return std::apply(variant, tuple);
				};
		}

		R call(
			const std::array<const std::type_info*, argsCount>& type,
			const std::array<Ref<void>, argsCount>& args
		) {
			return functions.at(type)(args);
		}
	};
}