#pragma once
#include "Sym.h"

namespace snl {
	template<typename F>
	class Function;

	template<typename R, typename... Args>
	class FunctionCallProxy {
		Ref<Sym<R>> expr;
		std::tuple<Ref<Sym<Args>>...> callVars;
		Ref<std::tuple<Sym<Args>...>> funArgs;

		template<size_t index = sizeof...(Args) - 1>
		void substituteAll(Sym<R>& target) const {
			target.substitute(std::get<index>(callVars).get(), std::get<index>(funArgs.get()));

			if constexpr (index != 0)
				substituteAll<index - 1>(target);
		}

		template<size_t index = sizeof...(Args) - 1>
		void inverseSubstituteAll(Sym<R>& target) const {
			target.substitute(std::get<index>(funArgs.get()), std::get<index>(callVars).get());

			if constexpr (index != 0)
				inverseSubstituteAll<index - 1>(target);
		}
	public:
		FunctionCallProxy(Sym<R>& expr, std::tuple<Ref<Sym<Args>>...> declVars, std::tuple<Sym<Args>...>& funArgs) :
			expr(expr), callVars(declVars), funArgs(funArgs) {}

		FunctionCallProxy& operator=(const Sym<R>& expr) {
			this->expr.get() = expr.deepCopy();

			substituteAll<>(this->expr);

			return *this;
		}

		Sym<R> sym() const {
			Sym<R> result = expr.get().deepCopy();
			inverseSubstituteAll<>(result);
			return result;
		}

		operator Sym<R> () const {
			return sym();
		}
	};
	
	template<typename T>
	constexpr bool isFunctionCallProxy = false;

	template<typename... Ts>
	constexpr bool isFunctionCallProxy<FunctionCallProxy<Ts...>> = true;

	template<typename T>
	concept IsFunctionCallProxy = isFunctionCallProxy<T>;

	template<typename R, typename... Args>
	class Function<R(Args...)> {
		snl::Sym<R> expr;
		std::tuple<Sym<Args>...> variables;

		template<size_t index = sizeof...(Args) - 1>
		void setVariables(std::tuple<Args...> args) {
			std::get<index>(variables).set(std::get<index>(args));

			if constexpr (index != 0)
				setVariables<index - 1>(args);
		}
	public:
		FunctionCallProxy<R, Args...> operator()(Sym<Args>&... declVars) {
			return FunctionCallProxy<R, Args...>(Ref(expr), { Ref(declVars)... }, Ref(variables));
		}

		R operator()(Args... args) {
			setVariables({ args... });

			expr.compute();

			return expr.get();
		}
	};
}