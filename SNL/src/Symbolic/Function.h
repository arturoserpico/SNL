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
			target.substitute(Ref(std::get<index>(funArgs.get())), std::get<index>(callVars));

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

		operator R () const {
			return sym();
		}
	};
	
	template<typename T>
	constexpr bool isFunctionCallProxy = false;

	template<typename... Ts>
	constexpr bool isFunctionCallProxy<FunctionCallProxy<Ts...>> = true;

	template<typename T>
	concept IsFunctionCallProxy = isFunctionCallProxy<T>;

	template<typename T>
	struct _FunctionCallProxyReturnT;

	template<typename R, typename... Args>
	struct _FunctionCallProxyReturnT<FunctionCallProxy<R, Args...>> : TypeAlias<R> {};

	template<typename T>
	using FunctionCallProxyReturnT = _FunctionCallProxyReturnT<std::remove_cvref_t<T>>::Type;

	template<typename TargetList = TypeList<>, size_t index = 0, typename T>
		requires staticIf<std::is_same_v<TargetList, TypeList<>>, true, std::is_convertible_v<T, SafeGet<TargetList, index>>> 
		&& !IsFunctionCallProxy<T>
	auto convertArgsToSymRef(T first, auto&&... rest) {
		Ref<Sym<std::conditional_t<std::is_same_v<TargetList, TypeList<>>, T, SafeGet<TargetList, index>>>> result;

		if constexpr (std::is_same_v<TargetList, TypeList<>>)
			result = makeManaged<Sym<T>>(first);
		else
			result = makeManaged<Sym<Get<TargetList, index>>>(static_cast<Get<TargetList, index>>(first));

		if constexpr (sizeof...(rest) == 0) {
			return std::tuple(result);
		}
		else {
			return std::tuple_cat(std::tuple(result), convertArgsToSymRef<TargetList, index + 1>(std::forward<decltype(rest)>(rest)...));
		}
	}

	template<typename TargetList = TypeList<>, size_t index = 0, typename T> 
		requires staticIf<std::is_same_v<TargetList, TypeList<>>, true, std::is_same_v<T, SafeGet<TargetList, index>>>
	auto convertArgsToSymRef(Sym<T>& first, auto&&... rest) {
		if constexpr (sizeof...(rest) == 0) {
			return std::tuple(Ref(first));
		}
		else {
			return std::tuple_cat(std::tuple(Ref(first)), convertArgsToSymRef<TargetList, index + 1>(std::forward<decltype(rest)>(rest)...));
		}
	}

	template<typename TargetList = TypeList<>, size_t index = 0, typename T>
		requires staticIf<std::is_same_v<TargetList, TypeList<>>, true, std::is_same_v<T, SafeGet<TargetList, index>>>
	auto convertArgsToSymRef(const Sym<T>& first, auto&&... rest) {
		Ref<Sym<T>> result = makeManaged<Sym<T>>(first);

		if constexpr (sizeof...(rest) == 0) {
			return std::tuple(result);
		}
		else {
			return std::tuple_cat(std::tuple(result), convertArgsToSymRef<TargetList, index + 1>(std::forward<decltype(rest)>(rest)...));
		}
	}

	template<typename TargetList = TypeList<>, size_t index = 0>
	auto convertArgsToSymRef(IsFunctionCallProxy auto first, auto&&... rest) 
		requires staticIf<std::is_same_v<TargetList, TypeList<>>, true, std::is_same_v<FunctionCallProxyReturnT<decltype(first)>, SafeGet<TargetList, index>>>
	{
		using T = FunctionCallProxyReturnT<decltype(first)>;
		Ref<Sym<T>> result = makeManaged<Sym<T>>(first.sym());

		if constexpr (sizeof...(rest) == 0) {
			return std::tuple(result);
		}
		else {
			return std::tuple_cat(std::tuple(result), convertArgsToSymRef<TargetList, index + 1>(std::forward<decltype(rest)>(rest)...));
		}
	}

	template<typename... Args>
	struct CheckValidFunCall {
		template<size_t index, typename... CallArgs>
		struct Inner {
			using ArgsList = TypeList<Args...>; 
			using CallArgsList = TypeList<CallArgs...>;

			static constexpr bool value = staticIf<isSym<std::remove_cvref_t<Get<CallArgsList, index>>>,
				std::is_same_v<SafeRemSym<Get<CallArgsList, index>>, Get<ArgsList, index>>,
				std::is_convertible_v<std::remove_cvref_t<Get<CallArgsList, index>>, Get<ArgsList, index>>> && Inner<index - 1, CallArgs...>::value;
		};

		template<typename... CallArgs>
		struct Inner<-1, CallArgs...> {
			static constexpr bool value = true;
		};
	};

	template<typename R, typename... Args>
	class Function<R(Args...)> {
		using ArgsList = TypeList<Args...>;

		snl::Sym<R> expr;
		std::tuple<Sym<Args>...> variables;

		template<size_t index = sizeof...(Args) - 1>
		void setVariables(std::tuple<Args...> args) {
			std::get<index>(variables).set(std::get<index>(args));

			if constexpr (index != 0)
				setVariables<index - 1>(args);
		}
	public:
		//FunctionCallProxy<R, Args...> operator()(Sym<Args>&... vars) {
		//	return FunctionCallProxy<R, Args...>(Ref(expr), { Ref(vars)... }, Ref(variables));
		//}

		auto operator()(auto&&... vars) 
			requires CheckValidFunCall<Args...>::template Inner<sizeof...(Args) - 1, decltype(vars)...>::value
		{
			auto tuple = convertArgsToSymRef<ArgsList>(std::forward<decltype(vars)>(vars)...);
			return FunctionCallProxy<R, Args...>(expr, tuple, variables);
		}
	};
}