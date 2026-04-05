#pragma once

#include <optional>
#include <vector>
#include <functional>
#include "../Memory/ObjectManager.h"
#include "../Utils/Any.h"
#include "../Utils/Ref.h"
#include "../Utils/Error.h"
#include "../Metaprogramming/Concepts.h"
#include "../Metaprogramming/TypeList.h"
//#include "SymNodes.h"

namespace snl {
	template<typename Fn>
	struct SymOpType;

	template<typename _R, typename... Args>
	struct SymOpType<_R(Args...)> {
		using R = _R;
		using ArgsList = TypeList<Args...>;

		//virtual R eval(Args...) = 0;
	};

	template<typename T>
	concept IsSymOpType = requires(T t) {
		typename T::R;
		typename T::ArgsList;
		t.eval;
	};

	template<typename T>
	struct SymLabel : SymOpType<T()> {
		static size_t lastLabelId;
		size_t labelId = lastLabelId++;
		T eval() = delete;
	};

	template<typename T>
	size_t SymLabel<T>::lastLabelId = 0;

	template<typename T>
	bool operator==(const SymLabel<T>& a, const SymLabel<T>& b) {
		return a.labelId == b.labelId;
	}

	template<typename T>
	struct SymConstant : SymOpType<T()> {
		T value;

		SymConstant() = default;
		SymConstant(T value) : value(value) {}

		T eval() {
			return value;
		}
	};

	template<typename T>
	bool operator==(const SymConstant<T>& a, const SymConstant<T>& b) {
		return a.value == b.value;
	}

	template<typename T>
	class Sym;

	template<typename T>
	constexpr bool isSym = false;

	template<typename T>
	constexpr bool isSym<Sym<T>> = true;

	template<typename T>
	concept IsSym = isSym<T>;

	template<typename T>
	concept IsSymMaybeConst = isSym<std::remove_const_t<T>>;

	template<typename T>
	concept IsSymIgnoreCVRef = isSym<std::remove_cvref_t<T>>;

	template<typename T>
	struct _RemSym;

	template<typename T>
	struct _RemSym<Sym<T>> : TypeAlias<T> {};

	template<typename T>
	using RemSym = _RemSym<std::remove_cvref_t<T>>::Type;

	template<typename T>
	struct _SafeRemSym : TypeAlias<void> {};

	template<typename T>
	struct _SafeRemSym<Sym<T>> : TypeAlias<T> {};

	template<typename T>
	using SafeRemSym = _SafeRemSym<std::remove_cvref_t<T>>::Type;

	using SymSubstitutionTypeMismatchError =
		Error<"tried substituting snl::Sym with another type">;

	struct GenericSym {
		static ErasedFunction<bool, const GenericSym, 2> comparators;

		virtual bool isEvaluable() const = 0;
		virtual const std::type_info& nodeType() const = 0;
		virtual const std::type_info& symType() const = 0;
		virtual std::vector<Ref<GenericSym>>& rawDeps() = 0;
		virtual const std::vector<Ref<GenericSym>>& rawDeps() const = 0;
		virtual std::vector<const std::type_info*> getDepsTypes() const = 0;
		//virtual void virtualCompute() = 0;
		
		std::string print() const {
			std::stringstream str;

			str << "{\n";

			for (Ref<GenericSym> dep : rawDeps())
				str << '\t' << dep.get().print();

			if (rawDeps().size() == 0)
				str << this << '\n';

			str << "}\n";

			return str.str();
		}

		template<typename T>
		size_t occurences(Ref<Sym<T>> target) {
			size_t result = 0;

			for (Ref<GenericSym> dep : rawDeps()) {
				if (dep.raw() == target.as<void>().raw())
					result++;
				else
					result += dep.get().occurences(target);
			}

			return result;
		}

		template<typename T>
		size_t occurences(Sym<T>& target) {
			return occurences(Ref(target));
		}

		template<typename T>
		void substitute(Ref<const Sym<T>> target, Ref<Sym<T>> substitute) {
			for (size_t i = 0; i < rawDeps().size(); i++) {
				if (target.raw() == rawDeps()[i].as<Sym<T>>().raw()) {
					SNLDebugCall(1, expect<SymSubstitutionTypeMismatchError>(typeid(T) == *getDepsTypes()[i]));
					rawDeps()[i] = substitute.as<GenericSym>();
				}
				else
					rawDeps()[i].get().substitute(target, substitute);
			}
		}

		template<typename T>
		void substitute(Ref<Sym<T>> target, Ref<Sym<T>> substitute) {
			this->substitute(static_cast<Ref<const Sym<T>>>(target), substitute);
		}

		template<typename T>
		void substitute(const Sym<T>& target, Sym<T>& substitute) {
			this->substitute(Ref(target), Ref(substitute));
		}

		template<typename T>
		void substitute(const Sym<T>& target, const Sym<T>& substitute) {
			this->substitute(Ref(target), makeManaged(substitute));
		}

		virtual Ref<GenericSym> rawDeepCopy() const = 0;
	};

	ErasedFunction<bool, const GenericSym, 2> GenericSym::comparators;

	bool operator==(const GenericSym& a, const GenericSym& b) {
		if (a.symType() != b.symType())
			return false;

		return GenericSym::comparators.call({ &a.symType(), &b.symType() }, std::array<Ref<const GenericSym>, 2>({ Ref(a), Ref(b) }));
	}

	using UnevaulableSymEvalError = Error<"tried to evaluate unevaluable snl::Sym">;

	template<typename T>
	class Sym : public GenericSym {
		bool isDefined = false;
		Any<> evalObj;
		//std::optional<T> value;
		std::function<T(bool, Any<>&, std::vector<Ref<GenericSym>>&)> fun;
		std::conditional_t<(debugLevel > 0), std::vector<const std::type_info*>, Empty> depsTypes;
		std::vector<Ref<GenericSym>> deps;
	public:
		bool isEvaluable() const {
			bool depsEvaluable = true;
			
			for (Ref<GenericSym> dep : deps)
				depsEvaluable = depsEvaluable && dep.get().isEvaluable();

			return isDefined; //&& depsEvaluable;
		}

		std::vector<Ref<GenericSym>>& rawDeps() {
			return deps;
		}

		const std::vector<Ref<GenericSym>>& rawDeps() const {
			return deps;
		}

		std::vector<const std::type_info*> getDepsTypes() const {
			if constexpr (debugLevel > 0)
				return depsTypes;
			else
				return {};
		}

		const std::type_info& nodeType() const {
			return evalObj.getType();
		}

		const std::type_info& symType() const {
			return typeid(T);
		}
	private:
		template<typename First, typename... Rest>
		static std::vector<Ref<GenericSym>> copyDeps(Ref<Sym<First>> first, Ref<Sym<Rest>>... rest) {
			Ref<GenericSym> result;

			if (first.get().rawDeps().size() == 0)
				result = first.as<void>();
			else
				result = makeManaged(first.get().copy()).as<GenericSym>();

			if constexpr (sizeof...(rest) == 0) {
				return { result };
			}
			else {
				std::vector<Ref<GenericSym>> restDeps = copyDeps<Rest...>(rest...);
				restDeps.insert(restDeps.begin(), result);
				return restDeps;
			}
		}

		template<typename First, typename... Rest>
		static std::vector<Ref<GenericSym>> deconcretizeDeps(Ref<Sym<First>> first, Ref<Sym<Rest>>... rest) {
			Ref<GenericSym> result = first.as<GenericSym>();

			if constexpr (sizeof...(rest) == 0) {
				return { result };
			}
			else {
				std::vector<Ref<GenericSym>> restDeps = deconcretizeDeps<Rest...>(rest...);
				restDeps.insert(restDeps.begin(), result);
				return restDeps;
			}
		}

		template<typename First, typename... Rest>
		static std::tuple<Ref<Sym<First>>, Ref<Sym<Rest>>...> concretizeDeps(std::vector<Ref<GenericSym>>& deps, size_t index = 0) {
			Ref<Sym<First>> result = deps[index].as<Sym<First>>();

			if constexpr (sizeof...(Rest) == 0) {
				return std::tuple<Ref<Sym<First>>>(result);
			}
			else {
				std::tuple<Ref<Sym<Rest>>...> restResults = concretizeDeps<Rest...>(deps, index + 1);
				return std::tuple_cat(std::tuple<Ref<Sym<First>>>(result), restResults);
			}
		}

		template<size_t index, typename TargetList, typename... Deps>
		static TypeListToTuple<TargetList> computeDeps(const std::tuple<Ref<Sym<Deps>>...>& deps) {
			TypeListToTuple<TargetList> result;

			if constexpr (index != sizeof...(Deps) - 1)
				result = computeDeps<index + 1, TargetList, Deps...>(deps);
			 
			if constexpr (IsSym<SafeRemRef<Get<TargetList, index>>>)
				std::get<index>(result) = std::get<index>(deps);
			else
				std::get<index>(result) = std::get<index>(deps).get().eval();

			return result;
		}

		template<typename First, typename... Rest>
		static std::vector<const std::type_info*> getDepsTypes() {
			const std::type_info* result = &typeid(First);

			if constexpr (sizeof...(Rest) == 0) {
				return { result };
			}
			else {
				std::vector<Ref<GenericSym>> rest = getDepsTypes<Rest...>();
				rest.insert(rest.begin(), result);
				return rest;
			}
		}

		Ref<GenericSym> rawDeepCopy() const {
			Ref<Sym<T>> copy = makeManaged(*this);

			std::vector<Ref<GenericSym>> deps = rawDeps();

			for (size_t i = 0; i < deps.size(); i++)
				if (deps[i].get().rawDeps().size() != 0)
					copy.get().rawDeps()[i] = deps[i].get().rawDeepCopy();

			return copy.as<GenericSym>();
		}
	public:
		auto operator()(auto&&...) &;
		auto operator()(auto&&...) &&;

		//auto operator|=(auto&&) &;
		//auto operator|=(auto&&) &&;

		Sym<T> deepCopy() const {
			return rawDeepCopy().as<Sym<T>>();
		}

		template<IsSymOpType SymOpType>
		Sym(SymOpType evalObj) : evalObj(evalObj) {
			comparators.addVariant<T, T>(std::function([](const Sym<T>& a, const Sym<T>& b) -> bool {
					return a == b;
				}));
			
			constexpr bool isDefined = requires(SymOpType evalObj) {
				evalObj.eval();
			};

			this->isDefined = isDefined;

			fun = std::function([](bool isEvaluable, Any<>& _evalObj, std::vector<Ref<GenericSym>>& deps) -> T {
					if constexpr (isDefined) {
						if (isEvaluable) {
							auto& evalObj = _evalObj.get<SymOpType>();
							return std::apply(&SymOpType::eval, std::tuple(evalObj));
						}
					}
					
					forceThrow<UnevaulableSymEvalError>();
					SNLMSVCCall(__assume(false));
				});
		}

		template<IsSymOpType SymOpType, typename... Deps>
		Sym(SymOpType evalObj, Ref<Sym<Deps>>... deps) : evalObj(evalObj), depsTypes({ &typeid(Deps)... }) {			
			comparators.addVariant<T, T>(std::function([](const Sym<T>& a, const Sym<T>& b) -> bool {
					return a == b;
				}));
			
			using InvokeTypes = FunctionTypeInfo<decltype(&SymOpType::eval)>::ArgsList;

			constexpr bool isDefined = requires(SymOpType evalObj, TypeListToTuple<InvokeTypes> args) {
				std::apply(&SymOpType::eval, std::tuple_cat(std::tuple(evalObj), args));
			};

			this->isDefined = isDefined;
			
			this->deps = deconcretizeDeps(deps...);
			fun = std::function([](bool isEvaluable, Any<>& _evalObj, std::vector<Ref<GenericSym>>& deps) -> T {
					if constexpr (isDefined) {
						if (isEvaluable) {
							auto& evalObj = _evalObj.get<SymOpType>();
							auto concretizedDeps = concretizeDeps<Deps...>(deps);
							auto computedDeps = computeDeps<0, typename SymOpType::ArgsList, Deps...>(concretizedDeps);
							return std::apply(&SymOpType::eval, std::tuple_cat(std::tuple(evalObj), computedDeps));
						}
					}

					forceThrow<UnevaulableSymEvalError>();
					SNLMSVCCall(__assume(false));
				});
		}

		Sym() : Sym<T>(SymLabel<T>()) {}
		Sym(T value) : Sym<T>(SymConstant<T>(value)) {}

		Sym<T>& operator=(T value) {
			*this = Sym<T>(value);
			return *this;
		}
	private:
		template<int... seq, typename SymOpType, typename... Deps>
		static Sym<T> constructFromTuple(
			std::index_sequence<seq...> sequence,
			SymOpType evalObj,
			std::tuple<Ref<Sym<Deps>>...> deps
		) {
			return Sym<T>(evalObj, std::get<seq>(deps)...);
		}
	public:
		template<typename SymOpType, typename... Deps>
		Sym(SymOpType evalObj, std::tuple<Ref<Sym<Deps>>...> deps) {
			*this = constructFromTuple(std::make_index_sequence<sizeof...(Deps)>(), evalObj, deps);
		}

		Sym(const Sym<T>& other) {
			isDefined = other.isDefined;
			evalObj = other.evalObj;
			fun = other.fun;
			depsTypes = other.depsTypes;
			deps = other.deps;
		}

		Sym<T>& operator=(const Sym<T>& other) {
			isDefined = other.isDefined;
			evalObj = other.evalObj;
			fun = other.fun;
			depsTypes = other.depsTypes;
			deps = other.deps;

			return *this;
		}

		Sym<T> dep() {
			return Sym<T>(SymIdentity<T>(), Ref(*this));
		}

		template<typename SymOpType>
		bool isOpType() {
			return evalObj.getType() == typeid(SymOpType);
		}

		T eval() {
			if (fun) { 
				return fun(isEvaluable(), evalObj, deps);
			}
		}

		//T& get() {
		//	SNLDebugCall(1, expect(value.has_value(), "Value not computed or assigned yet"));
		//	return value.value();
		//}
		//
		//const T& get() const {
		//	SNLDebugCall(1, expect(value.has_value(), "Value not computed or assigned yet"));
		//	return value.value();
		//}

		//T& computeGet() {
		//	if (value.has_value()) {
		//		return get();
		//	} 
		//	else
		//		return compute().get();
		//}
		//
		//Sym<T>& set() {
		//	value = T();
		//	return *this;
		//}
		//
		//Sym<T>& set(T val) {
		//	value = val;
		//	return *this;
		//}

		operator T () {
			return eval();
		}

		template<typename A>
		Sym<A> cast() {
			if constexpr (std::is_same_v<A, T>)
				return *this;
			else
				return Sym<A>(SymCast<A, T>(), makeManaged(dep()));
		}

		template<typename A>
		operator Sym<A>() {
			return cast<A>();
		}

		template<typename T>
		friend bool operator==(const snl::Sym<T>& a, const snl::Sym<T>& b);
	};

	template<typename T>
	bool operator==(const snl::Sym<T>& a, const snl::Sym<T>& b) {
		return
			a.evalObj == b.evalObj &&
			a.depsTypes == b.depsTypes &&
			a.deps == b.deps;
	}

	template<typename SymOpType, typename... Deps>
	Sym(SymOpType, Ref<Sym<Deps>>...) -> Sym<typename SymOpType::R>;

	template<typename SymOpType, typename... Deps>
	Sym(SymOpType, std::tuple<Ref<Sym<Deps>>...>) -> Sym<typename SymOpType::R>;

	std::ostream& operator<<(std::ostream& stream, IsSym auto val) {
		stream << val.eval();
		return stream;
	}
}