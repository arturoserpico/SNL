#pragma once

#include <typeinfo>
#include "../Utils/Ref.h"
#include "../Memory/ObjectManager.h"
#include "../Metaprogramming/Utils.h"
#include <tuple>

namespace snl {
	template<typename Derived>
	class TypeBase {
	public:
		struct GenericConstructor {
			static inline size_t lastId = 0;

			size_t id;

			GenericConstructor() : id(lastId++) {}

			bool operator==(const GenericConstructor& other) const {
				return id == other.id;
			}
		};
	private:
		struct GenericData {
			GenericConstructor constructor;

			GenericData(GenericConstructor constructor) : constructor(constructor) {}

			virtual operator Derived() const = 0;

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
			std::tuple<Args...> args;

			Data(const Constructor<Derived(Args...)>& constructor, Args... args) : 
				GenericData(constructor), args(args...) {}

			operator Derived() const {
				Derived result;
				result.data = makeManaged(*this).as<GenericData>();
				return result;
			}

			const std::type_info& constructorType() const {
				return typeid(Constructor<Derived(Args...)>);
			}

			Ref<GenericData> heapCopy() const {
				return makeManaged(*this).as<GenericData>();
			}

			bool compare(const GenericData& other) const {
				ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;
				
				if (constructorType() != other.constructorType())
					return false;

				return Ref(other).as<const Data<Args...>>().get().args == args;
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
		TypeBase() = default;

		TypeBase(const TypeBase<Derived>& other) {
			if (other.data.raw() == nullptr)
				data = nullptr;
			else
				data = other.data.get().heapCopy();
		}

		TypeBase<Derived>& operator=(const TypeBase<Derived>& other) {
			if (other.data.raw() == nullptr)
				data = nullptr;
			else
				data = other.data.get().heapCopy();

			return *this;
		}

		GenericConstructor constructor() const {
			return data.get().constructor;
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
	};

	template<typename Derived> requires std::derived_from<Derived, TypeBase<Derived>>
	bool operator==(const TypeBase<Derived>& a, const TypeBase<Derived>& b) {
		return a.getData().get().compare(b.getData().get());
	}

	template<typename Derived> requires std::derived_from<Derived, TypeBase<Derived>>
	bool operator==(const Derived& a, const Derived& b) {
		return static_cast<const TypeBase<Derived>&>(a) == static_cast<const TypeBase<Derived>&>(b);
	}

	template<typename Derived>
	template<typename R, typename... Args> requires std::is_same_v<R, Derived>
	auto TypeBase<Derived>::Constructor<R(Args...)>::operator>>(
		auto callable
	) const {
		using Info = FunctionTypeInfo<decltype(&decltype(callable)::operator())>;
		return typename TypeBase<Derived>::template MatchArm<typename Info::Function, Args...>(*this, std::function(callable));
	}

	template<typename Derived>
	decltype(auto) match(Derived& val, auto first, auto... rest) {
		if (val.TypeBase<Derived>::constructor().id == first.constructor.id)
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

	template<typename Derived>
	decltype(auto) match(const Derived& val, auto first, auto... rest) {
		if (val.TypeBase<Derived>::constructor().id == first.constructor.id)
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
		TypeAlias<typename TypeBase<Derived>::template Constructor<Derived(Args...)>> {};

	template<typename F>
	using Constructor = _Constructor<F>::Type;
}