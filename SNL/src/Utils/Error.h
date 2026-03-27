#pragma once

#include <exception>
#include <string>
#include <iostream>
#include <algorithm>
#include <array>
#include <sstream>

#include "../Metaprogramming/StaticString.h"

#define SNLDebugLevel 2

#define SNLDebugCall(LEVEL, CALL) if constexpr (::snl::debugLevel >= LEVEL) CALL;

#define fail catch(::std::exception e) { throw e; }
#define ignore catch(::std::exception e) {}
#define ignoreLoudly catch(::std::exception e) { ::snl::debug << "exception ignored: " << e.what() << ::std::endl; }
#define save(VAR) catch(::std::exception e) { VAR = e; }
#define saveMsg(VAR) catch(::std::exception e) { VAR = e.what(); }

namespace snl {
	constexpr size_t debugLevel = SNLDebugLevel;

	struct DebugLogger : public std::ostream {
		static const bool enabled = true;

		struct DebugBuf : public std::stringbuf {
			int sync() {
				if (enabled) {
					std::cout << "SNL debug: " << str();
					str("");
					return std::cout ? 0 : -1;
				}

				return 0;
			}
		};

		DebugLogger() : std::ostream(new DebugBuf()) {}

		~DebugLogger() {
			delete rdbuf();
		}
	};

	DebugLogger debug;

	struct Exception : std::exception {
		const std::string msg;

		inline Exception(const std::string& msg) : msg("SNL encountered an error: " + msg) {
			debug << "SNL exception generated: " + msg << std::endl;
		}

		const char* what() {
			return msg.c_str();
		}
	};

	template<StaticString exceptionClass>
	struct ClassedException : Exception {
		inline ClassedException(const std::string& msg) : Exception(msg + ", exception class: " + (std::string)exceptionClass) {}
	};

	inline void expect(bool condition, const char* msg) {
		if constexpr (debugLevel > 0)
			if (!condition)
				throw Exception(msg);
	}

	template<typename Exception>
	inline void expect(bool condition, const char* msg) {
		if constexpr (debugLevel > 0)
			if (!condition)
				throw Exception(msg);
	}
}