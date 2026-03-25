#pragma once

#include <optional>
#include <vector>
#include <functional>
#include "../Memory/ObjectManager.h"
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

		virtual R eval(Args...) = 0;
	};

	template<typename T>
	concept IsSymOpType = requires(T t) {
		typename T::R;
		typename T::ArgsList;
		t.eval;
	};

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

	struct GenericSym {
		virtual std::vector<Ref<void>>& rawDeps() = 0;
		virtual const std::vector<Ref<void>>& rawDeps() const = 0;
		virtual std::vector<const std::type_info*> getDepsTypes() const = 0;
		virtual void virtualCompute() = 0;
		
		std::string print() const {
			std::stringstream str;

			str << "{\n";

			for (Ref<void> dep : rawDeps())
				str << '\t' << dep.as<GenericSym>().get().print();

			if (rawDeps().size() == 0)
				str << this << '\n';

			str << "}\n";

			return str.str();
		}

		template<typename T>
		size_t occurences(Ref<Sym<T>> target) {
			size_t result = 0;

			for (Ref<void> dep : rawDeps()) {
				if (dep.raw() == target.as<void>().raw())
					result++;
				else
					result += dep.as<GenericSym>().get().occurences(target);
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
					SNLDebugCall(1, expect(typeid(T) == *getDepsTypes()[i], "substitution target is not of correct type"));
					rawDeps()[i] = substitute.as<void>();
				}
				else
					rawDeps()[i].as<GenericSym>().get().substitute(target, substitute);
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

		virtual Ref<void> rawDeepCopy() const = 0;
	};

	template<typename T>
	class Sym : public GenericSym {
		std::optional<T> value;
		std::function<T(std::vector<Ref<void>>&)> fun;
		const std::type_info* symOpType = nullptr;
		std::conditional_t<(debugLevel > 0), std::vector<const std::type_info*>, Empty> depsTypes;
		std::vector<Ref<void>> deps;
	public:
		std::vector<Ref<void>>& rawDeps() {
			return deps;
		}

		const std::vector<Ref<void>>& rawDeps() const {
			return deps;
		}

		std::vector<const std::type_info*> getDepsTypes() const {
			if constexpr (debugLevel > 0)
				return depsTypes;
			else
				return {};
		}

	private:
		template<typename First, typename... Rest>
		static std::vector<Ref<void>> copyDeps(Ref<Sym<First>> first, Ref<Sym<Rest>>... rest) {
			Ref<void> result;

			if (first.get().rawDeps().size() == 0)
				result = first.as<void>();
			else
				result = makeManaged(first.get().copy()).as<void>();

			if constexpr (sizeof...(rest) == 0) {
				return { result };
			}
			else {
				std::vector<Ref<void>> restDeps = copyDeps<Rest...>(rest...);
				restDeps.insert(restDeps.begin(), result);
				return restDeps;
			}
		}

		template<typename First, typename... Rest>
		static std::vector<Ref<void>> deconcretizeDeps(Ref<Sym<First>> first, Ref<Sym<Rest>>... rest) {
			Ref<void> result = first.as<void>();

			if constexpr (sizeof...(rest) == 0) {
				return { result };
			}
			else {
				std::vector<Ref<void>> restDeps = deconcretizeDeps<Rest...>(rest...);
				restDeps.insert(restDeps.begin(), result);
				return restDeps;
			}
		}

		template<typename First, typename... Rest>
		static std::tuple<Ref<Sym<First>>, Ref<Sym<Rest>>...> concretizeDeps(std::vector<Ref<void>>& deps, size_t index = 0) {
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
				std::get<index>(result) = std::get<index>(deps).get().compute().get();

			return result;
		}

		template<typename First, typename... Rest>
		static std::vector<const std::type_info*> getDepsTypes() {
			const std::type_info* result = &typeid(First);

			if constexpr (sizeof...(Rest) == 0) {
				return { result };
			}
			else {
				std::vector<Ref<void>> rest = getDepsTypes<Rest...>();
				rest.insert(rest.begin(), result);
				return rest;
			}
		}

		Ref<void> rawDeepCopy() const {
			Ref<Sym<T>> copy = makeManaged(*this);

			for (size_t i = 0; i < rawDeps().size(); i++)
				if (rawDeps()[i].as<GenericSym>().get().rawDeps().size() != 0)
					copy.get().rawDeps()[i] = rawDeps()[i].as<GenericSym>().get().rawDeepCopy();

			return copy.as<void>();
		}
	public:
		auto operator()(auto&&...) &;
		auto operator()(auto&&...) &&;

		auto operator|=(auto&&) &;
		auto operator|=(auto&&) &&;

		Sym<T> deepCopy() const {
			return rawDeepCopy().as<Sym<T>>();
		}

		Sym() = default;

		Sym(T val) : value(val) {}

		template<IsSymOpType SymOpType, typename... Deps>
		Sym(SymOpType evalObj, Ref<Sym<Deps>>... deps) : symOpType(&typeid(SymOpType)), depsTypes({ &typeid(Deps)... }) {
			this->deps = deconcretizeDeps(deps...);
			fun = std::function([evalObj](std::vector<Ref<void>>& deps) {
					auto concretizedDeps = concretizeDeps<Deps...>(deps);
					auto computedDeps = computeDeps<0, typename SymOpType::ArgsList, Deps...>(concretizedDeps);
					return std::apply(&SymOpType::eval, std::tuple_cat(std::tuple(evalObj), computedDeps));
				});
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
			value = other.value;
			fun = other.fun;
			symOpType = other.symOpType;
			depsTypes = other.depsTypes;
			deps = other.deps;
		}

		Sym<T>& operator=(const Sym<T>& other) {
			value = other.value;
			fun = other.fun;
			symOpType = other.symOpType;
			depsTypes = other.depsTypes;
			deps = other.deps;

			return *this;
		}

		Sym<T> dep() {
			return Sym<T>(SymIdentity<T>(), Ref(*this));
		}

		template<typename SymOpType>
		bool isOpType() {
			return *symOpType == typeid(SymOpType);
		}

		void virtualCompute() {
			if (fun) {
				value = fun(deps);
			}
		}

		Sym<T>& compute() {
			if (fun) {
				value = fun(deps);
			}

			return *this;
		}

		T& get() {
			SNLDebugCall(1, expect(value.has_value(), "Value not computed or assigned yet"));
			return value.value();
		}

		const T& get() const {
			SNLDebugCall(1, expect(value.has_value(), "Value not computed or assigned yet"));
			return value.value();
		}

		T& computeGet() {
			if (value.has_value()) {
				return get();
			} 
			else
				return compute().get();
		}

		Sym<T>& set() {
			value = T();
			return *this;
		}

		Sym<T>& set(T val) {
			value = val;
			return *this;
		}

		operator T () {
			return compute().get();
		}

		template<typename A>
		Sym<A> cast() {
			if constexpr (std::is_same_v<A, T>)
				return *this;
			else
				return Sym<A>(SymCast<A, T>(), makeManaged(dep().deepCopy()));
		}

		template<typename A>
		operator Sym<A>() {
			return cast<A>();
		}
	};

	template<typename SymOpType, typename... Deps>
	Sym(SymOpType, Ref<Sym<Deps>>...) -> Sym<typename SymOpType::R>;

	template<typename SymOpType, typename... Deps>
	Sym(SymOpType, std::tuple<Ref<Sym<Deps>>...>) -> Sym<typename SymOpType::R>;

	std::ostream& operator<<(std::ostream& stream, IsSym auto val) {
		stream << val.computeGet();
		return stream;
	}
}