#pragma once

namespace snl {
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
	auto staticIf = 0;

	template<auto then, auto otherwise>
	auto staticIf<true, then, otherwise> = then;

	template<auto then, auto otherwise>
	auto staticIf<false, then, otherwise> = otherwise;
}