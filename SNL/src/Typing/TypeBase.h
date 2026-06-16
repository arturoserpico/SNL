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
		};
	private:
		struct GenericData {
			GenericConstructor constructor;

			GenericData(GenericConstructor constructor) : constructor(constructor) {}

			virtual operator Derived() const = 0;

			virtual const std::type_info& constructorType() const = 0;

			virtual Ref<GenericData> heapCopy() const = 0;
		};

	public:
		template<typename... Args>
		struct Constructor;

	private:
		template<typename... Args>
		struct Data : GenericData {
			std::tuple<Args...> args;

			Data(const Constructor<Args...>& constructor, Args... args) : 
				GenericData(constructor), args(args...) {}

			operator Derived() const {
				Derived result;
				result.data = makeManaged(*this).as<GenericData>();
				return result;
			}

			const std::type_info& constructorType() const {
				return typeid(Constructor<Args...>);
			}

			Ref<GenericData> heapCopy() const {
				return makeManaged(*this).as<GenericData>();
			}
		};

	public:
		template<typename R, typename... Args>
		struct MatchArm;

		template<typename... Args>
		struct Constructor : public GenericConstructor {
			Derived operator()(Args... args) const {
				return Data<Args...>(*this, args...);
			}

			operator Derived() const requires (sizeof...(Args) == 0) {
				return this->operator()();
			}

			auto operator>>(auto callable) const;
		};

		template<typename R, typename... Args>
		struct MatchArm {
			using Return = R;

			Constructor<Args...> constructor;
			std::function<R(Args...)> f;

			MatchArm(Constructor<Args...> constructor, std::function<R(Args...)> f) :
				constructor(constructor), f(f) {}

			R call(Ref<const GenericData> data) const {
			 	return std::apply(f, data.as<const Data<Args...>>().get().args);
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

	template<typename Derived>
	template<typename... Args>
	auto TypeBase<Derived>::Constructor<Args...>::operator>>(
		auto callable
	) const {
		using Info = FunctionTypeInfo<decltype(&decltype(callable)::operator())>;
		return typename TypeBase<Derived>::template MatchArm<typename Info::Return, Args...>(*this, std::function(callable));
	}

	template<typename Derived>
	auto match(const Derived& val, auto first, auto... rest) {
		if (val.constructorType() == typeid(first.constructor))
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
		TypeAlias<typename TypeBase<Derived>::template Constructor<Args...>> {};

	template<typename F>
	using Constructor = _Constructor<F>::Type;
}