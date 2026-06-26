#pragma once

#include <typeinfo>
#include "../Symbolic/Sym.h"
#include <tuple>
#include <functional>
#include <type_traits>
//#include "../Symbolic/SymNodes.h"

namespace snl {
	using AlgebraicNotComparableError =
		Error<"tried comparing two Algabraics wich have constructors with not comparable types">;

	template<typename Derived>
	class AlgebraicBase {
	public:
		struct GenericConstructor {
			static inline size_t lastId = 0;

			size_t id;

			GenericConstructor() : id(lastId++) {}

			bool operator==(const GenericConstructor& other) const {
				return id == other.id;
			}

			using Type = Derived;
		};
	private:
		struct GenericData {
			virtual size_t hash() const = 0;

			virtual Sym<Derived> sym() const = 0;

			virtual const GenericConstructor& getConstructor() const = 0;

			virtual operator Derived() const = 0;

			virtual const std::type_info& type() const = 0;

			virtual const std::type_info& constructorType() const = 0;

			virtual Ref<GenericData> heapCopy() const = 0;

			virtual bool compare(const GenericData& other) const = 0;
		};

	public:
		template<typename F>
		struct Constructor;

	private:
		template<typename... Args>
		struct Data : GenericData {
			Constructor<Derived(Args...)> constructor;
			std::tuple<Args...> args;

			Data(const Constructor<Derived(Args...)>& constructor, Args... args) : 
				constructor(constructor), args(args...) { }

			size_t hash() const {
				// Use type trait to check if std::hash<Args> is invocable with Args
				constexpr bool hashable = (... && std::is_invocable_r_v<size_t, std::hash<Args>, Args>);

				if constexpr (hashable)
					return std::apply([&](Args... args) -> size_t {
						return hashCombine(constructor.id, std::hash<Args>{}(args)...);
					}, args);
				else
					return 0;
			}

			Sym<Derived> sym() const {
				return std::apply([&](Args... args) -> Sym<Derived> {
					return Sym<Derived>(
						SymCall<decltype(constructor), TypeList<Args...>>(),
						Sym<decltype(constructor)>(constructor),
						Sym(args)...
					);
				}, args);
			}

			const GenericConstructor& getConstructor() const {
				return dynamic_cast<const GenericConstructor&>(constructor);
			}

			operator Derived() const {
				Derived result;
				result.data = makeManaged(*this).as<GenericData>();
				return result;
			}

			const std::type_info& type() const {
				return typeid(Data<Args...>);
			}

			const std::type_info& constructorType() const {
				return typeid(Constructor<Derived(Args...)>);
			}

			Ref<GenericData> heapCopy() const {
				return makeManaged(*this).as<GenericData>();
			}

			bool compare(const GenericData& other) const {
				ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;
				
				constexpr bool isComparable = (... && IsComparable<Args>);

				if (constructorType() != other.constructorType())
					return false;
				
				if constexpr (isComparable)
					return Ref(other).as<const Data<Args...>>().get().args == args;
				else
					forceThrow<AlgebraicNotComparableError>(); 

				SNLMSVCCall(__assume(false));
			}
		};

	public:
		template<typename R, typename... Args>
		struct MatchArm;

		template<typename R, typename... Args> requires std::is_same_v<R, Derived>
		struct Constructor<R(Args...)> : public GenericConstructor {
			Derived operator()(Args... args) const {
				return Data<Args...>(*this, args...);
			}

			operator Derived() const requires (sizeof...(Args) == 0) {
				return this->operator()();
			}

			auto operator>>(auto callable) const;

			bool operator==(const Constructor<R(Args...)>& other) const {
				return this->id == other.id;
			}
		};

		template<typename F, typename... Args> //requires std::is_invocable_v<F*, Args...>
		struct MatchArm {
			using Return = FunctionTypeInfo<F>::Return;

			Constructor<Derived(Args...)> constructor;
			std::function<F> f;

			MatchArm(Constructor<Derived(Args...)> constructor, std::function<F> f) :
				constructor(constructor), f(f) {}

			Return call(Ref<const GenericData> data) const {
			 	return std::apply(f, data.as<const Data<Args...>>().get().args);
			}

			Return call(Ref<GenericData> data) const {
				return std::apply(f, data.as<Data<Args...>>().get().args);
			}
		};

		template<typename T>
		static constexpr bool isMatchArm = false;

		template<typename R, typename... Args>
		static constexpr bool isMatchArm<MatchArm<R, Args...>> = true;
	private:
		Ref<GenericData> data = nullptr;
	public:
		AlgebraicBase() = default;

		AlgebraicBase(const AlgebraicBase<Derived>& other) {
			if (other.data.raw() == nullptr)
				data = nullptr;
			else
				data = other.data.get().heapCopy();
		}

		AlgebraicBase<Derived>& operator=(const AlgebraicBase<Derived>& other) {
			if (other.data.raw() == nullptr)
				data = nullptr;
			else
				data = other.data.get().heapCopy();

			return *this;
		}

		size_t hash() const {
			return data.get().hash();
		}

		Sym<Derived> sym() const {
			return data.get().sym();
		}

		GenericConstructor constructor() const {
			return data.get().getConstructor();
		}

		const std::type_info& constructorType() const {
			return data.get().constructorType();
		}

		Ref<GenericData> getData() {
			return data;
		}

		Ref<const GenericData> getData() const {
			return data.asConst();
		}

		bool is(const GenericConstructor& constructor) {
			return data.get().getConstructor() == constructor;
		}
	};

	template<typename T>
	concept IsAlgebraicSym = IsSym<T> && IsAlgebraic<typename T::Type>;

	template<IsAlgebraic Derived>
	bool operator==(const AlgebraicBase<Derived>& a, const AlgebraicBase<Derived>& b) {
		return a.getData().get().compare(b.getData().get());
	}

	template<IsAlgebraic Derived>
	bool operator==(const Derived& a, const Derived& b) {
		return static_cast<const AlgebraicBase<Derived>&>(a) == static_cast<const AlgebraicBase<Derived>&>(b);
	}

	template<typename Derived>
	template<typename R, typename... Args> requires std::is_same_v<R, Derived>
	auto AlgebraicBase<Derived>::Constructor<R(Args...)>::operator>>(
		auto callable
	) const {
		using Info = FunctionTypeInfo<decltype(&decltype(callable)::operator())>;
		return typename AlgebraicBase<Derived>::template MatchArm<typename Info::Function, Args...>(*this, std::function(callable));
	}

	template<IsAlgebraic Derived>
	decltype(auto) match(Derived& val, auto first, auto... rest) {
		if (val.AlgebraicBase<Derived>::constructor().id == first.constructor.id)
			if constexpr (std::is_same_v<typename decltype(first)::Return, void>)
				first.call(val.getData());
			else
				return first.call(val.getData());
		else
			if constexpr (sizeof...(rest) != 0)
				if constexpr (std::is_same_v<typename decltype(first)::Return, void>)
					match(val, rest...);
				else
					return match(val, rest...);
	}

	template<IsAlgebraic Derived>
	decltype(auto) match(const Derived& val, auto first, auto... rest) {
		if (val.AlgebraicBase<Derived>::constructor().id == first.constructor.id)
			if constexpr (std::is_same_v<typename decltype(first)::Return, void>)
				first.call(val.getData());
			else
				return first.call(val.getData());
		else
			if constexpr (sizeof...(rest) != 0)
				if constexpr (std::is_same_v<typename decltype(first)::Return, void>)
					match(val, rest...);
				else
					return match(val, rest...);
	}

	template<typename F>
	struct _Constructor;

	template<typename Derived, typename... Args>
	struct _Constructor<Derived(Args...)> : 
		TypeAlias<typename AlgebraicBase<Derived>::template Constructor<Derived(Args...)>> {};

	template<typename F>
	using Constructor = _Constructor<F>::Type;

	//template<IsAlgebraic T, typename... Args>
	//struct SymAlgebraicConstruction : SymOpType<T(Constructor<T(Args...)>, Args...)> {
	//	T eval(const Constructor<T(Args...)>& constructor, Args... args) {
	//		return static_cast<T>(constructor(args...));
	//	}
	//};
	//
	//template<IsAlgebraic T, typename... Args>
	//bool operator==(SymAlgebraicConstruction<T, Args...> a, SymAlgebraicConstruction<T, Args...> b) {
	//	return true;
	//}

	template<typename T>
	Sym<T>::Sym(T value) requires IsAlgebraic<T> : Sym<T>(value.sym()) {}
}

namespace std {
	template<snl::IsAlgebraic T>
	struct hash<T> {
		size_t operator()(const T& val) const {
			return val.hash();
		}
	};
}