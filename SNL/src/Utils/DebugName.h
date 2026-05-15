#pragma once
#include "Error.h"

namespace snl {
	class DebugNamesRegister {
		std::map<const void*, std::string> names;
	public:
		void add(const auto& obj, const std::string& name) {
			names.emplace(static_cast<const void*>(&obj), name);
		}

		void add(const void* obj, const std::string& name) {
			names.emplace(obj, name);
		}

		std::string getName(const auto& obj) {
			return names[static_cast<const void*>(&obj)];
		}

		std::string getName(const void* obj) {
			return names[obj];
		}
	};

	static std::conditional_t<debugLevel >= 1, DebugNamesRegister, Empty> debugNamesRegister;
	
	void addDebugName(const auto& obj, const char* name) {
		SNLDebugCall(1, debugNamesRegister.add(obj, name));
	}

	void addDebugName(const auto* obj, const char* name) {
		SNLDebugCall(1, debugNamesRegister.add(obj, name));
	}

	std::conditional_t<debugLevel >= 1, std::string, Empty> getDebugName(const auto& obj) {
		SNLDebugCall(1, return debugNamesRegister.getName(obj));
		return {};
	}

	std::conditional_t<debugLevel >= 1, std::string, Empty> getDebugName(const auto* obj) {
		SNLDebugCall(1, return debugNamesRegister.getName(obj));
		return {};
	}
}