#pragma once

#include "GenericSym.h"

namespace snl {
	using InvalidEditError = Error<"SymEdit application is invalid">;

	class SymEdit {
		RelativeSymRef target;
		Ref<const GenericSym> substitute = nullptr;
	public:
		SymEdit(const RelativeSymRef& target, const GenericSym& substitute) :
			target(target), substitute(substitute.heapCopy()) {}

		template<typename T>
		SymEdit(const RelativeSymRef& target, const Sym<T>& substitute) :
			target(target),
			substitute(makeManaged(substitute).as<const GenericSym>()) {}

		void apply(GenericSym& sym) const {
			SNLDebugCall(1, expect<InvalidEditError>(target.use(sym).symType() == substitute.get().symType()))
			target.use(sym) = substitute.get();
		}

		template<typename T>
		void apply(Sym<T>& sym) const {
			SNLDebugCall(1, expect<InvalidEditError>(target.use(sym).symType() == substitute.get().symType()))
			target.use(sym) = substitute.get();
		}
	};

	class EditGroup {
		std::vector<SymEdit> edits;
	public:
		EditGroup(const std::vector<SymEdit>& edits) : edits(edits) {}
		EditGroup() = default;

		EditGroup& add(const SymEdit& edit) {
			edits.push_back(edit);
			return *this;
		}

		void apply(GenericSym& sym) const {
			for (const SymEdit& edit : edits)
				edit.apply(sym);
		}

		template<typename T>
		void apply(Sym<T>& sym) const {
			for (const SymEdit& edit : edits)
				edit.apply(sym);
		}
	};
}