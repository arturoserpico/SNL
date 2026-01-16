#pragma once
#include "Sym.h"

namespace snl {
	template<typename F>
	class Function;

	template<typename R, typename... Args>
	class Function<R(Args...)> {
		Sym<R, Args...> expr;
		std::tuple<Sym<Args>> variables;
	};
}