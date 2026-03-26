#pragma once

#include <map>
#include "../Utils/Set.h"
#include "Sym.h"
#include "Function.h"


namespace snl {
	class MathRule {
		Set<Ref<GenericSym>> variables;
		size_t specificity;
		Ref<GenericSym> original, substitute;
	public:
		template<typename T, typename... Vars>
		MathRule(Ref<Sym<T>> original, Ref<Sym<T>> substitute, Ref<Sym<Vars>>... variables) {

		}
	};

	class MathScope {
		
	};

	MathScope currentMathScope;
}