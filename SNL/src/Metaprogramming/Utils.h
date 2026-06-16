#pragma once

#include <functional>
#include <type_traits>

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

	template<auto value>
	using Static = ValueAlias<value>;

	template<auto value>
	auto makeStatic = Static<value>{};

	template<bool condition, auto then, auto otherwise>
	constexpr auto staticIf = 0;

	template<auto then, auto otherwise>
	constexpr auto staticIf<true, then, otherwise> = then;

	template<auto then, auto otherwise>
	constexpr auto staticIf<false, then, otherwise> = otherwise;

	template<bool condition, typename Then, typename Else>
	struct _If;

	template<typename Then, typename Else>
	struct _If<true, Then, Else> : TypeAlias<Then> {};

	template<typename Then, typename Else>
	struct _If<false, Then, Else> : TypeAlias<Else> {};

	template<bool condition, typename Then, typename Else>
	using If = _If<condition, Then, Else>::Type;

	struct StaticError {
		constexpr StaticError() {}

		template<typename T>
		constexpr operator T() {
			return T();
		}
	};


	
	template<typename T>
	struct FunctionTypeInfo {
		using Fail = ErrorType<>;
		using Return = Fail;
		using ArgsList = Fail;
	};

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

	template<typename Obj, typename R, typename... Args>
	struct FunctionTypeInfo<R(Obj::*)(Args...)> {
		using Return = R;
		using ArgsList = TypeList<Args...>;
		using ObjectType = Obj;
	};

	template<typename Obj, typename R, typename... Args>
	struct FunctionTypeInfo<R(Obj::*)(Args...) const> {
		using Return = R;
		using ArgsList = TypeList<Args...>;
		using ObjectType = Obj;
	};

	template<typename R, typename... Args>
	struct FunctionTypeInfo<std::function<R(Args...)>> {
		using Return = R;
		using ArgsList = TypeList<Args...>;
	};



	template<typename T, auto vals>
	using IdenticalTypePackFromValues = T;

	template<typename T, typename Ts>
	using IdenticalTypePackFromTs = T;
}