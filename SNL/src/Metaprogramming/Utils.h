#pragma once

namespace snl {
	struct Empty {
		Empty() = default;
		Empty(auto&&...) {}
	};

	template<typename... Ts>
	struct TypeList;

	template<auto... info>
	struct ErrorType {};

	template<typename T>
	struct TypeAlias {
		using Type = T;
	};

	template<auto _value>
	struct ValueAlias {
		static constexpr auto value = _value;
	};



	template<bool condition, auto then, auto otherwise>
	constexpr auto staticIf = 0;

	template<auto then, auto otherwise>
	constexpr auto staticIf<true, then, otherwise> = then;

	template<auto then, auto otherwise>
	constexpr auto staticIf<false, then, otherwise> = otherwise;



	struct StaticError {
		constexpr StaticError() {}

		template<typename T>
		constexpr operator T() {
			return T();
		}
	};


	
	template<typename F>
	struct FunctionTypeInfo;

	template<typename R, typename... Args>
	struct FunctionTypeInfo<R(Args...)> {
		using Return = R;
		using ArgsList = TypeList<Args...>;
	};

	template<typename R, typename... Args>
	struct FunctionTypeInfo<R(*)(Args...)> {
		using Return = R;
		using ArgsList = TypeList<Args...>;
	};

	template<typename R, typename... Args>
	struct FunctionTypeInfo<std::function<R(Args...)>> {
		using Return = R;
		using ArgsList = TypeList<Args...>;
	};
}