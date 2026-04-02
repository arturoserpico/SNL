#pragma once
#include <unordered_map>
#include "Sym.h"
//#include "SymOperators.h"
#include "Constants.h"
#include "SymUtils.h"

namespace snl {
	enum class FunctionType {
		Numeric,
		Analytic
	};

	enum class CallType {
		Numeric,
		Symbolic,
		Mixed
	};

	CallType combineFunCallTypes(CallType a, CallType b) {
		if (a == b)
			return a;
		else
			return CallType::Mixed;
	}

	template<typename F>
	class Function;

	template<size_t index, typename... Ts>
	auto _distance_impl(std::tuple<Ts...> a, std::tuple<Ts...> b) {
		if constexpr(index == 0)
			return std::pow(std::get<index>(a) - std::get<index>(b), 2);
		else
			return std::sqrt(std::pow(std::get<index>(a) - std::get<index>(b), 2) + 
			std::pow(_distance_impl<index - 1, Ts...>(a, b), 2));
	}

	template<typename... Ts>
	auto distance(std::tuple<Ts...> a, std::tuple<Ts...> b) {
		return _distance_impl<sizeof...(Ts) - 1, Ts...>(a, b);
	}

	template<typename... Ts>
	using TupleDistanceT = decltype(distance<Ts...>(std::tuple<Ts...>(), std::tuple<Ts...>()));

	template<typename R, typename... Args>
	R idwlerp(std::unordered_map<std::tuple<Args...>, R> points, std::tuple<Args...> sample) {
		using Distance = TupleDistanceT<Args...>;
		
		R result = 0;
		
		size_t pointUsed = 0;

		Distance total = 0;

		std::unordered_map<Distance, R> distanceMap;

		for (const auto& [point, value] : points)
			distanceMap.emplace(distance<Args...>(sample, point), value);

		for (const auto& [distance, value] : distanceMap) {
			if (pointUsed < sizeof...(Args) + 1) {
				pointUsed++;

				Distance w = 1/(std::pow(distance, 2) + epsilon<Distance>);

				total += w;
				result += w*value;
			}
		}

		result /= total;

		return result;
	}

	template<typename R, typename... Args>
	class InterpolatedFunCall : SymOpType<R(Args...)> {
		std::unordered_map<std::tuple<Args...>, R> points;
	public:
		InterpolatedFunCall(std::unordered_map<std::tuple<Args...>, R> points) : points(points) {}

		R eval(Args... args) {
			return idwlerp<R, Args...>(points, std::tuple(args...));
		}
	};

	using InvalidTypesForNumericFunctionError =
		Error<"snl::Function cannot be numeric with given argument types">;

	template<typename R, typename... Args>
	class FunctionCallProxy {
		static constexpr bool canNumeric() {
			return requires() { std::hash<std::tuple<Args...>>{}; };
		}

		CallType callType;
		Ref<FunctionType> type;
		Ref<Sym<R>> expr;
		std::conditional_t<canNumeric(), Ref<std::unordered_map<std::tuple<Args...>, R>>, Empty> numeric;
		std::conditional_t<canNumeric(), std::function<R(std::unordered_map<std::tuple<Args...>, R>, std::tuple<Args...>)>, Empty> interpolation;
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

		template<size_t index = sizeof...(Args) - 1>
		void _computeCallVars(std::tuple<Args...>& out) {
			std::get<index>(out) = std::get<index>(callVars).get().compute().get();
			if constexpr (index != 0)
				_computeCallVars<index - 1>(out);
		}

		std::tuple<Args...> computeCallVars() {
			std::tuple<Args...> result;
			_computeCallVars(result);
			return result;
		}
	public:
		FunctionCallProxy() = default;

		FunctionCallProxy(
			CallType callType,
			FunctionType& type, 
			Sym<R>& expr, 
			Ref<std::unordered_map<std::tuple<Args...>, R>> numeric,
			std::function<R(std::unordered_map<std::tuple<Args...>, R>, std::tuple<Args...>)> interpolation,
			std::tuple<Ref<Sym<Args>>...> declVars, 
			std::tuple<Sym<Args>...>& funArgs
		) :
			callType(callType),
			type(type), 
			expr(expr), 
			numeric(numeric), 
			interpolation(interpolation), 
			callVars(declVars), 
			funArgs(funArgs) 
		{}

		CallType getCallType() const {
			return callType;
		}

		FunctionCallProxy& operator|=(auto&& _expr) {
			auto [expr] = convertArgsToSymRef<TypeList<R>>(std::forward<decltype(_expr)>(_expr));
			switch (callType)
			{
			case snl::CallType::Numeric: {
				if constexpr (canNumeric()) {
					auto point = computeCallVars();

					numeric.get()[point] = expr.get().deepCopy().compute().get();

					type.get() = FunctionType::Numeric;

					return *this;
				} else
					throwError<InvalidTypesForNumericFunctionError>();
			}
			case snl::CallType::Symbolic: {
				this->expr.get() = expr.get().deepCopy();

				substituteAll<>(this->expr);

				type.get() = FunctionType::Analytic;

				return *this;
			}
			case snl::CallType::Mixed:
				//throw Exception("snl::Function declaration cannot be mixed");
				break;
			}
		}

		//FunctionCallProxy& operator=(R value) {
		//	*this = Sym<R>(value);
		//	return *this;
		//}

		Sym<R> sym() const {
			if (type == FunctionType::Analytic) {
				Sym<R> result = expr.get().deepCopy();
				inverseSubstituteAll<>(result);
				return result;
			}
			else {
				if constexpr (canNumeric())
					return Sym<R>(InterpolatedFunCall<R, Args...>(numeric), callVars);
				else
					throwError<InvalidTypesForNumericFunctionError>();
			}
		}

		operator Sym<R> () const {
			return sym();
		}

		operator R () const {
			return sym();
		}
	};

	template<typename T> 
		requires (not isFunctionCallProxy<T>)
	CallType getFunCallType(T first, auto&&... rest) {
		if constexpr (sizeof...(rest) != 0) {
			CallType typeRest = getFunCallType(std::forward<decltype(rest)>(rest)...);
			return combineFunCallTypes(CallType::Numeric, typeRest);
		}
		else
			return CallType::Numeric;
	}

	template<typename T>
	CallType getFunCallType(Sym<T>& first, auto&&... rest) {
		if constexpr (sizeof...(rest) != 0) {
			CallType typeRest = getFunCallType(std::forward<decltype(rest)>(rest)...);
			return combineFunCallTypes(CallType::Symbolic, typeRest);
		}
		else
			return CallType::Symbolic;
	}

	template<typename T>
	CallType getFunCallType(const Sym<T>& first, auto&&... rest) {
		if constexpr (sizeof...(rest) != 0) {
			CallType typeRest = getFunCallType(std::forward<decltype(rest)>(rest)...);
			return combineFunCallTypes(CallType::Symbolic, typeRest);
		}
		else
			return CallType::Symbolic;
	}

	CallType getFunCallType(const IsFunctionCallProxy auto& first, auto&&... rest) {
		if constexpr (sizeof...(rest) != 0) {
			CallType typeRest = getFunCallType(std::forward<decltype(rest)>(rest)...);
			return combineFunCallTypes(first.getCallType(), typeRest);
		}
		else
			return first.getCallType();
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

		static constexpr bool canNumeric() {
			return requires() { std::hash<std::tuple<Args...>>{}; };
		}

		FunctionType type;
		snl::Sym<R> expr;
		std::conditional_t<canNumeric(), std::unordered_map<std::tuple<Args...>, R>, Empty> numeric;
		std::conditional_t<canNumeric(), std::function<R(std::unordered_map<std::tuple<Args...>, R>, std::tuple<Args...>)>, Empty> interpolation;
		std::tuple<Sym<Args>...> variables;

		template<size_t index = sizeof...(Args) - 1>
		void substituteAll(const std::tuple<Sym<Args>...>& oldVariables) {
			expr.substitute(std::get<index>(oldVariables), std::get<index>(variables));

			if constexpr (index != 0)
				substituteAll<index - 1>(oldVariables);
		}

		template<size_t index = sizeof...(Args) - 1>
		void setVariables(std::tuple<Args...> args) {
			std::get<index>(variables).set(std::get<index>(args));

			if constexpr (index != 0)
				setVariables<index - 1>(args);
		}
	public:
		Function() = default;
		
		Function(std::unordered_map<std::tuple<Args...>, R> points) : type(FunctionType::Numeric), numeric(points) {}

		Function(const Function& other) {
			type = other.type;
			expr = other.expr.deepCopy();
			numeric = other.numeric;
			interpolation = other.interpolation;

			substituteAll(other.variables);
		}

		auto operator()(auto&&... vars) 
			//requires CheckValidFunCall<Args...>::template Inner<sizeof...(Args) - 1, decltype(vars)...>::value
		{
			auto tuple = convertArgsToSymRef<ArgsList>(std::forward<decltype(vars)>(vars)...);
			
			if constexpr (canNumeric()) 
				return FunctionCallProxy<R, Args...>(
					getFunCallType(std::forward<decltype(vars)>(vars)...), 
					type, expr, numeric, interpolation, tuple, variables);
			else
				return FunctionCallProxy<R, Args...>(
					getFunCallType(std::forward<decltype(vars)>(vars)...),
					type, expr, nullptr, {}, tuple, variables);
		}

		friend struct std::hash<Function<R(Args...)>>;
	};
}

//namespace std {
//	template<typename F>
//	struct hash<snl::Function<F>> {
//		size_t operator()(const snl::Function<F>& f) const noexcept {
//			//size_t seed = 0;
//			//
//			//auto hashCombine = [&seed](size_t v) {
//			//	seed += v * 0x9e3779b97f4a7c15ULL;
//			//	};
//
//			const unsigned char* data = reinterpret_cast<const unsigned char*>(&f);
//
//			size_t h = 0;
//			
//			for (size_t i = 0; i < sizeof(T); ++i)
//				h = h * 31 + data[i];
//			
//			return h;
//		}
//	};
//};