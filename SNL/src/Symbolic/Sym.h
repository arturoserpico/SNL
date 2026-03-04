#pragma once

#include <optional>
#include <vector>
#include <functional>
#include "../Memory/ObjectManager.h"
#include "../Utils/Ref.h"
#include "../Utils/Error.h"
#include "../Metaprogramming/Concepts.h"
#include "../Metaprogramming/TypeList.h"
#include "SymNodes.h"

namespace snl {
	template<typename T>
	class Sym;

	struct GenericSym {
		virtual std::vector<Ref<void>>& rawDeps() = 0;
		virtual std::vector<const std::type_info*> getDepsTypes() = 0;
		template<typename T>
		void substitute(Ref<Sym<T>> target, Ref<Sym<T>> substitute) {
			for (size_t i = 0; i < rawDeps().size(); i++) {
				if (target.raw() == rawDeps()[i].as<Sym<T>>().raw()) {
					expect(typeid(T) == *getDepsTypes()[i], "substitution target is not of correct type");
					rawDeps()[i] = substitute.as<void>();
				}
				else
					rawDeps()[i].as<GenericSym>().get().substitute(target, substitute);
			}
		}

		template<typename T>
		void substitute(Sym<T>& target, Sym<T>& substitute) {
			this->substitute(Ref(target), Ref(substitute));
		}

		template<typename T>
		void substitute(Sym<T>& target, const Sym<T>& substitute) {
			this->substitute(Ref(target), makeManaged(substitute));
		}
	};

	template<typename T>
	class Sym : public GenericSym {
		std::optional<T> value;
		std::function<T(std::vector<Ref<void>>)> fun;
		const std::type_info* symOpType = nullptr;
		std::vector<const std::type_info*> depsTypes;
		std::vector<Ref<void>> deps;

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
		static std::tuple<Ref<Sym<First>>, Ref<Sym<Rest>>...> concretizeDeps(std::vector<Ref<void>> deps, size_t index = 0) {
			Ref<Sym<First>> result = deps[index].as<Sym<First>>();

			if constexpr (sizeof...(Rest) == 0) {
				return std::tuple<Ref<Sym<First>>>(result);
			}
			else {
				std::tuple<Ref<Sym<Rest>>...> restResults = concretizeDeps<Rest...>(deps, index + 1);
				return std::tuple_cat(std::tuple<Ref<Sym<First>>>(result), restResults);
			}
		}

		template<typename First, typename... Rest>
		static std::tuple<First, Rest...> computeDeps(Ref<Sym<First>> first, Ref<Sym<Rest>>... rest) {
			first.get().compute();

			if constexpr (sizeof...(rest) == 0) {
				return std::make_tuple(first.get().get());
			}
			else {
				std::tuple<Rest...> restResults = computeDeps<Rest...>(rest...);
				return std::tuple_cat(std::make_tuple(first.get().get()), restResults);
			}
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
	public:
		Sym() = default;

		Sym(T val) : value(val) {}

		template<typename SymOpType, typename... Deps>
		Sym(SymOpType evalObj, Ref<Sym<Deps>>... deps) : symOpType(&typeid(SymOpType)), depsTypes({ &typeid(Deps)... }) {
			this->deps = deconcretizeDeps(deps...);
			fun = [evalObj](std::vector<Ref<void>> deps) {
					auto concretizedDeps = concretizeDeps<Deps...>(deps);
					auto computedDeps = std::apply(computeDeps<Deps...>, concretizedDeps);
					return std::apply(&SymOpType::eval, std::tuple_cat(std::tuple(evalObj), computedDeps));
				};
		}

		std::vector<Ref<void>>& rawDeps() {
			return deps;
		}
		
		std::vector<const std::type_info*> getDepsTypes() {
			return depsTypes;
		}

		Sym<T> dep() {
			return Sym<T>(SymIdentity<T>(), Ref(*this));
		}

		template<typename SymOpType>
		bool isOpType() {
			return *symOpType == typeid(SymOpType);
		}

		void compute() {
			if (fun) {
				value = fun(deps);
			}
		}

		T get() {
			expect(value.has_value(), "Value not computed or assigned yet");
			return value.value();
		}

		void set(T val) {
			value = val;
		}
	};

	template<typename T>
	constexpr bool isSym = false;

	template<typename T>
	constexpr bool isSym<Sym<T>> = true;

	template<typename T>
	concept IsSym = isSym<T>;

	template<typename T>
	concept IsSymMaybeConst = isSym<std::remove_const_t<T>>;

	template<typename T>
	concept IsRefIgnoreCVRef = isSym<std::remove_cvref_t<T>>;

	template<typename T>
	struct _SymT;

	template<typename T>
	struct _SymT<Sym<T>> : TypeAlias<T> {};

	template<typename T>
	using SymT = _SymT<std::remove_cvref_t<T>>::Type;
}