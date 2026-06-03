#pragma once

#include <optional>
#include <vector>
#include <functional>
#include <unordered_set>
#include "../Memory/ObjectManager.h"
#include "../Utils/Any.h"
#include "../Utils/Ref.h"
#include "../Utils/Error.h"
#include "../Utils/HashCombine.h"
#include "../Metaprogramming/Concepts.h"
#include "../Metaprogramming/TypeList.h"
#include "TypeOperations.h"

namespace snl {
	template<typename T>
	class Sym;

	using SymSubstitutionTypeMismatchError =
		Error<"tried substituting snl::Sym with another type">;

	struct GenericSym;

	bool operator==(const GenericSym& a, const GenericSym& b);

	class RelativeSymRef {
		std::vector<size_t> descender;
		Ref<GenericSym> target;
	public:
		RelativeSymRef(std::initializer_list<size_t> list) :
			descender(list), target(nullptr) {}

		RelativeSymRef(GenericSym& target, std::initializer_list<size_t> list) :
			descender(list) 
		{
			ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;
			this->target = Ref(target, Ref(target).realManagmentState());
		}

		RelativeSymRef() = default;

		RelativeSymRef operator[](size_t index) {
			RelativeSymRef result(*this);
			result.descender.push_back(index);
			return result;
		}

		RelativeSymRef& rise() {
			descender.erase(descender.end() - 1);
			return *this;
		}

		const GenericSym& use(const GenericSym& sym) const;
		GenericSym& use(GenericSym& sym) const;

		GenericSym& use() const {
			return use(target.get());
		}

		RelativeSymRef use(RelativeSymRef sym) const {
			RelativeSymRef result;

			for (size_t index : descender)
				result = result[index];

			return result;
		}

		template<typename T>
		GenericSym& use(Sym<T>& sym) const {
			return use(Ref(sym).as<GenericSym>().get());
		}

		template<typename T>
		const GenericSym& use(const Sym<T>& sym) const {
			return use(Ref(sym).as<const GenericSym>().get());
		}

		RelativeSymRef& bind(GenericSym& target) {
			ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;
			this->target = Ref(target, Ref(target).realManagmentState());
			return *this;
		}

		template<typename T>
		RelativeSymRef& bind(Sym<T>& target) {
			ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;
			return bind(Ref(target).as<GenericSym>().get());
		}

		operator GenericSym& () const {
			return use();
		}
	};

	struct GenericSym {
		static ErasedFunction<bool, const GenericSym, 2> erasedComparator;
		static ErasedFunction<std::function<void(GenericSym&)>, const GenericSym> erasedCopyAssignment;

		virtual Ref<GenericSym> heapCopy() const = 0;
		virtual bool isEvaluable() const = 0;
		virtual const std::type_info& nodeType() const = 0;
		virtual const std::type_info& symType() const = 0;
		virtual std::vector<Ref<GenericSym>>& rawDeps() = 0;
		virtual const std::vector<Ref<GenericSym>>& rawDeps() const = 0;
		virtual std::vector<const std::type_info*> getDepsTypes() const = 0;
		virtual Ref<GenericSym> rawDeepCopy() const = 0;
		virtual size_t getHash() const = 0;
		virtual Ref<GenericSym> rawDep() = 0;

		bool isLeaf() const {
			return rawDeps().empty();
		}
		//virtual void virtualCompute() = 0;



		GenericSym& operator=(const GenericSym& other) {
			snl::ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;
			erasedCopyAssignment.call({ &other.symType() }, { Ref(other) })(*this);
			return *this;
		}

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

		void unevaluableLeafCount(std::unordered_set<Ref<const GenericSym>>& traversed) const {
			ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;

			if (isLeaf() && !isEvaluable())
				traversed.insert(Ref(*this));

			for (Ref<const GenericSym> dep : rawDeps())
				dep.get().unevaluableLeafCount(traversed);
		}

		size_t unevaluableLeafCount() const {
			std::unordered_set<Ref<const GenericSym>> traversed;
			unevaluableLeafCount(traversed);
			return traversed.size();
		}

		size_t occurences(Ref<const GenericSym> target) const {
			ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;
			size_t result = 0;

			for (Ref<GenericSym> dep : rawDeps()) {
				if (dep == target)
					result++;
				else
					result += dep.get().occurences(target);
			}

			return result;
		}

		size_t occurences(const GenericSym& target) const {
			ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;
			return occurences(Ref(target));
		}

		template<typename T>
		size_t occurences(const Sym<T>& target) const {
			ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;
			return occurences(Ref(target).as<const GenericSym>());
		}

		void substitute(Ref<const GenericSym> target, Ref<const GenericSym> substitute) {
			if (target.get() == *this) {
				*this = substitute.get();
				return;
			}

			for (size_t i = 0; i < rawDeps().size(); i++) {
				if (target == rawDeps()[i]) {
					SNLDebugCall(1, expect<SymSubstitutionTypeMismatchError>(target.get().symType() == *getDepsTypes()[i]));
					rawDeps()[i] = substitute.get().heapCopy();
				}
				else
					rawDeps()[i].get().substitute(target, substitute);
			}
		}

		void substitute(Ref<const GenericSym> target, Ref<GenericSym> substitute) {
			if (target.get() == *this) {
				*this = substitute.get();
				return;
			}

			for (size_t i = 0; i < rawDeps().size(); i++) {
				if (target == rawDeps()[i]) {
					SNLDebugCall(1, expect<SymSubstitutionTypeMismatchError>(target.get().symType() == *getDepsTypes()[i]));
					rawDeps()[i] = substitute;
				}
				else
					rawDeps()[i].get().substitute(target, substitute);
			}
		}

		void substitute(const GenericSym& target, const GenericSym& substitute) {
			return this->substitute(Ref(target), Ref(substitute));
		}

		void substitute(const GenericSym& target, GenericSym& substitute) {
			return this->substitute(Ref(target), Ref(substitute).asConst());
		}

		template<typename T>
		void substitute(Ref<const Sym<T>> target, Ref<const Sym<T>> substitute) {
			this->substitute(target.as<const GenericSym>(), substitute.as<const GenericSym>());
		}

		template<typename T>
		void substitute(Ref<const Sym<T>> target, Ref<Sym<T>> substitute) {
			this->substitute(target.as<const GenericSym>(), substitute.as<GenericSym>());
		}

		template<typename T>
		void substitute(const Sym<T>& target, Sym<T>& substitute) {
			snl::ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;
			this->substitute(Ref(target), Ref(substitute));
		}

		template<typename T>
		void substitute(const Sym<T>& target, const Sym<T>& substitute) {
			snl::ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;
			this->substitute(Ref(target), Ref(substitute));
		}

		RelativeSymRef operator[](size_t index) {
			return RelativeSymRef(*this, { index });
		}
	};

	const GenericSym& RelativeSymRef::use(const GenericSym& sym) const {
		ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;

		Ref<const GenericSym> current = Ref(sym);
		
		for (size_t index : descender)
			current = current.get().rawDeps()[index];

		return current.get();
	}

	GenericSym& RelativeSymRef::use(GenericSym& sym) const {
		ErrorSuppressor<UnmanagedRefToManagedObjWarning> _;

		Ref<GenericSym> current = Ref(sym);

		for (size_t index : descender)
			current = current.get().rawDeps()[index];

		return current.get();
	}

	ErasedFunction<bool, const GenericSym, 2> GenericSym::erasedComparator;
	ErasedFunction<std::function<void(GenericSym&)>, const GenericSym> GenericSym::erasedCopyAssignment;

	bool operator==(const GenericSym& a, const GenericSym& b) {
		snl::ErrorSuppressor<snl::UnmanagedRefToManagedObjWarning> _;

		if (a.symType() != b.symType())
			return false;

		return GenericSym::erasedComparator.call({ &a.symType(), &b.symType() }, std::array<Ref<const GenericSym>, 2>({ Ref(a), Ref(b) }));
	}
}