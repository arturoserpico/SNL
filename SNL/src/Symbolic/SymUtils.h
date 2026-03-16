#pragma once

#include "Sym.h"

namespace snl {
	template<typename T, size_t nCovariant, size_t nContravariant, size_t... sizes>
		requires (sizeof...(sizes) == nCovariant + nContravariant)
	class TensorIndexingProxy;

	template<typename T>
	constexpr bool isTensorIndexingProxy = false;

	template<typename T, size_t nCovariant, size_t nContravariant, size_t... sizes>
	constexpr bool isTensorIndexingProxy<TensorIndexingProxy<T, nCovariant, nContravariant, sizes...>> = true;

	template<typename T>
	concept IsTensorIndexingProxy = isTensorIndexingProxy<T>;

	template<typename R, typename... Args>
	class FunctionCallProxy;

	template<typename T>
	constexpr bool isFunctionCallProxy = false;

	template<typename... Ts>
	constexpr bool isFunctionCallProxy<FunctionCallProxy<Ts...>> = true;

	template<typename T>
	concept IsFunctionCallProxy = isFunctionCallProxy<T>;

	template<typename TargetList = TypeList<>, size_t index = 0, typename T>
		requires
		(staticIf<std::is_same_v<TargetList, TypeList<>>, true, std::is_convertible_v<T, SafeGet<TargetList, index>>>
		&& !IsFunctionCallProxy<T>
		&& !IsTensorIndexingProxy<T>)
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

	template<typename T>
	struct _TensorIndexingProxyType;

	template<typename T, size_t nCovariant, size_t nContravariant, size_t... sizes>
	struct _TensorIndexingProxyType<TensorIndexingProxy<T, nCovariant, nContravariant, sizes...>> : TypeAlias<T> {};

	template<typename T>
	using TensorIndexingProxyType = _TensorIndexingProxyType<std::remove_cvref_t<T>>::Type;

	template<typename TargetList = TypeList<>, size_t index = 0>
	auto convertArgsToSymRef(IsTensorIndexingProxy auto first, auto&&... rest)
		requires staticIf<std::is_same_v<TargetList, TypeList<>>, true, std::is_same_v<TensorIndexingProxyType<decltype(first)>, SafeGet<TargetList, index>>>
	{
		using T = TensorIndexingProxyType<decltype(first)>;
		Ref<Sym<T>> result = makeManaged<Sym<T>>(first.sym());

		if constexpr (sizeof...(rest) == 0) {
			return std::tuple(result);
		}
		else {
			return std::tuple_cat(std::tuple(result), convertArgsToSymRef<TargetList, index + 1>(std::forward<decltype(rest)>(rest)...));
		}
	}

	template<typename T>
	struct _FunctionCallProxyReturnT;

	template<typename R, typename... Args>
	struct _FunctionCallProxyReturnT<FunctionCallProxy<R, Args...>> : TypeAlias<R> {};

	template<typename T>
	using FunctionCallProxyReturnT = _FunctionCallProxyReturnT<std::remove_cvref_t<T>>::Type;

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

	auto witchSymbolic(auto&&... args) 
	{
		std::array<bool, sizeof...(args)> result{ IsSymIgnoreCVRef<decltype(args)>... };
		return result;
	}
}