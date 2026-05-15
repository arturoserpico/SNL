#pragma once

#include

namespace snl {
	using InvalidEditError = Error<"SymEdit initialization is invalid">;

	class SymEdit {
		Ref<GenericSym> target = nullptr;
		Ref<const GenericSym> substitute = nullptr;
	public:
		SymEdit(GenericSym& target, const GenericSym& substitute) :
			target(makeManaged(target)), substitute(makeManaged(substitute))
		{
			SNLDebugCall(expect<InvalidEditError>(target.symType() == substitute.symType()));
		}

		template<typename T>
		SymEdit(Sym<T>& target, const Sym<T>& substitute) :
			target(makeManaged(target).as<GenericSym>()),
			substitute(makeManaged(substitute).as<const GenericSym>()) {
		}


	};
}