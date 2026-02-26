#pragma once

#include <optional>
#include <vector>
#include <functional>
#include "../Utils/Ref.h"
#include "../Utils/Error.h"
#include "../Metaprogramming/Concepts.h"
#include "../Metaprogramming/TypeList.h"

namespace snl {
	template<typename T>
	class Sym {
		std::optional<T> value;
		std::function<T(std::vector<Ref<void>>)> fun;
		std::vector<Ref<void>> deps;

		template<typename First, typename... Rest>
		static std::vector<Ref<void>> deconcretizeDeps(Sym<First>& first, Sym<Rest>&... rest) {
			Ref<void> result(&first);

			if constexpr (sizeof...(rest) == 0) {
				return { result };
			}
			else {
				std::vector<Ref<void>> restDeps = deconcretizeDeps<Rest...>(rest...);
				restDeps.push_back(result);
				return restDeps;
			}
		}

		template<typename First, typename... Rest>
		static std::tuple<Sym<First>&, Sym<Rest>&...> concretizeDeps(std::vector<Ref<void>> deps, size_t index = 0) {
			Sym<First>& result = deps[index].as<Sym<First>>();

			if constexpr (sizeof...(Rest) == 0) {
				return std::tuple<Sym<First>&>(result);
			}
			else {
				std::tuple<Sym<Rest>&...> restResults = concretizeDeps<Rest...>(deps, index + 1);
				return std::tuple_cat(std::tuple<Sym<First>&>(result), restResults);
			}
		}

		template<typename First, typename... Rest>
		static std::tuple<First, Rest...> computeDeps(Sym<First>& first, Sym<Rest>&... rest) {
			first.compute();

			if constexpr (sizeof...(rest) == 0) {
				return std::make_tuple(first.get());
			}
			else {
				std::tuple<Rest...> restResults = computeDeps<Rest...>(rest...);
				return std::tuple_cat(std::make_tuple(first.get()), restResults);
			}
		}
	public:
		Sym() = default;

		template<typename... Deps>
		Sym(std::function<T(Deps...)> compute, Sym<Deps>&... deps) {
			this->deps = deconcretizeDeps(deps...);
			fun = [compute](std::vector<Ref<void>> deps) {
					auto concretizedDeps = concretizeDeps<Deps...>(deps);
					auto computedDeps = std::apply(computeDeps<Deps...>, concretizedDeps);
					return std::apply(compute, computedDeps);
				};
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

	template<typename A, typename B> requires requires(A a, B b) { a + b; }
	auto operator+(Sym<A>,  Sym<B>) {}
}