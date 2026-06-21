#pragma once

#include <optional>
#include <vector>
#include <functional>
#include "../Memory/ObjectManager.h"
#include "../Utils/Any.h"
#include "../Utils/Ref.h"
#include "../Utils/Error.h"
#include "../Utils/HashCombine.h"
#include "../Metaprogramming/Concepts.h"
#include "../Metaprogramming/TypeList.h"
#include "TypeOperations.h"
#include "GenericSym.h"

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

	using UnevaulableSymEvalError = Error<"tried to evaluate unevaluable snl::Sym">;

	template<typename T>
	class Sym : public GenericSym {
	private:
		std::function<T(bool, Any<>&, std::vector<Ref<GenericSym>>&)> evalFun;
	public:
		bool isEvaluable() const {
			for (Ref<GenericSym> dep : deps)
				if (!dep.get().isEvaluable())
					return false;
			
			return isDefined;
		}

		const std::type_info& symType() const {
			return typeid(T);
		}

		Ref<GenericSym> rawDeepCopy() const {
			Ref<Sym<T>> copy = makeManaged(*this);

			std::vector<Ref<GenericSym>> deps = rawDeps();

			for (size_t i = 0; i < deps.size(); i++)
				if (deps[i].get().rawDeps().size() != 0)
					copy.get().rawDeps()[i] = deps[i].get().rawDeepCopy();

			return copy.as<GenericSym>();
		}

		auto operator()(auto&&...) &;
		auto operator()(auto&&...) &&;

		auto operator-() &;
		auto operator-() &&;

		//auto operator|=(auto&&) &;
		//auto operator|=(auto&&) &&;

		Sym<T> deepCopy() const {
			return rawDeepCopy().as<Sym<T>>().get();
		}

		//Sym(auto) : Sym() {};

		template<IsSymOpType SymOpType>
		Sym(SymOpType evalObj) : GenericSym(evalObj) {
			erasedComparator.addVariant<T, T>(std::function([](const Sym<T>& a, const Sym<T>& b) -> bool {
					return a == b;
				}));

			erasedCopyAssignment.addVariant<T>(std::function([](const Sym<T>& other) {
					return std::function([other](GenericSym& target) {
							Ref(target).as<Sym<T>>().get() = other;
						});
				}));
			
			constexpr bool isDefined = requires(SymOpType evalObj) {
				evalObj.eval();
			};

			this->isDefined = isDefined;

			evalFun = std::function([](bool isEvaluable, Any<>& _evalObj, std::vector<Ref<GenericSym>>& deps) -> T {
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
		Sym(SymOpType evalObj, Ref<Sym<Deps>>... deps) : GenericSym(evalObj, { &typeid(Deps)... }) {			
			erasedComparator.addVariant<T, T>(std::function([](const Sym<T>& a, const Sym<T>& b) -> bool {
					return a == b;
				}));
			
			using InvokeTypes = FunctionTypeInfo<decltype(&SymOpType::eval)>::ArgsList;

			constexpr bool isDefined = requires(SymOpType evalObj, TypeListToTuple<InvokeTypes> args) {
				std::apply(&SymOpType::eval, std::tuple_cat(std::tuple(evalObj), args));
			};

			this->isDefined = isDefined;
			
			this->deps = deconcretizeDeps(deps...);
			evalFun = std::function([](bool isEvaluable, Any<>& _evalObj, std::vector<Ref<GenericSym>>& deps) -> T {
					if constexpr (isDefined) {
						if (isEvaluable) {
							auto& evalObj = _evalObj.get<SymOpType>();
							auto concretizedDeps = concretizeDeps<Deps...>(deps);
							auto computedDeps = computeDeps<typename SymOpType::ArgsList, Deps...>(concretizedDeps);
							return std::apply(&SymOpType::eval, std::tuple_cat(std::tuple(evalObj), computedDeps));
						}
					}

					forceThrow<UnevaulableSymEvalError>();
					SNLMSVCCall(__assume(false));
				});
		}

		Sym() : Sym<T>(SymLabel<T>()) {}
		Sym(T value) : Sym<T>(SymConstant<T>(value)) {}

		//Sym<T>& operator=(T value) {
		//	*this = Sym<T>(value);
		//	return *this;
		//}
		Ref<GenericSym> heapCopy() const {
			return makeManaged<Sym<T>>(deepCopy()).as<GenericSym>();
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
			evalFun = other.evalFun;
			depsTypes = other.depsTypes;
			deps = other.deps;
		}

		Sym<T>& operator=(const Sym<T>& other) {
			isDefined = other.isDefined;
			evalObj = other.evalObj;
			evalFun = other.evalFun;
			depsTypes = other.depsTypes;
			deps = other.deps;

			return *this;
		}

		template<typename A> requires (!std::is_same_v<T, A> && std::convertible_to<A, T>)
		Sym<T>& operator=(const Sym<A>& other) {
			*this = Sym<T>(SymCast<T, A>(), other.heapCopy().as<Sym<A>>());
			return *this;
		}

		Sym<T> dep() {
			return Sym<T>(SymIdentity<T>(), Ref(*this, Ref(*this).realManagmentState()));
		}

		Ref<GenericSym> rawDep() {
			return makeManaged<Sym<T>>(dep()).as<GenericSym>();
		}

		template<typename SymOpType>
		bool isOpType() {
			return evalObj.getType() == typeid(SymOpType);
		}

		T eval();

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

		size_t getHash() const;
 
		template<typename T>
		friend bool operator==(const snl::Sym<T>& a, const snl::Sym<T>& b);
	
		friend struct std::hash<snl::Sym<T>>;
	};

	template<typename TypeObject>
	Sym(TypeObject) -> Sym<typename TypeObject::Type>;

	template<typename F> requires std::is_function_v<F>
	using Function = Sym<F*>;

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

namespace std {
	template<>
	struct hash<snl::GenericSym> {
		size_t operator()(const snl::GenericSym& sym) const {
			return sym.getHash();
		}
	};

	template<typename T>
	struct hash<snl::Sym<T>> {
		size_t operator()(const snl::Sym<T>& sym) const {
			size_t seed = 0;

			for (snl::Ref<const snl::GenericSym> dep : sym.deps)
				seed = snl::hashCombine(seed, hash<snl::GenericSym>{}(dep.get()));

			return snl::hashCombine(std::hash<snl::Any<>>{}(sym.evalObj), seed);
		}
	};

	template<typename T>
	struct hash<snl::SymLabel<T>> {
		size_t operator()(const snl::SymLabel<T>& obj) {
			return obj.labelId;
		}
	};

	template<snl::IsSymOpType SymOpType>
	struct hash<SymOpType> {
		size_t operator()(const SymOpType&) {
			return 0;
		}
	};
}

namespace snl {
	template<typename T>
	size_t Sym<T>::getHash() const {
		return std::hash<Sym<T>>{}(*this);
	}
}