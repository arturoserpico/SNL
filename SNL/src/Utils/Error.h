#pragma once

#include <exception>
#include <string>
#include <iostream>
#include <algorithm>
#include <array>
#include <sstream>

#define fail catch(::std::exception e) { throw e; }
#define ignore catch(::std::exception e) {}
#define ignoreLoudly catch(::std::exception e) { ::snl::debug << "exception ignored: " << e.what() << ::std::endl; }
#define save(VAR) catch(::std::exception e) { VAR = e; }
#define saveMsg(VAR) catch(::std::exception e) { VAR = e.what(); }

namespace snl {
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

	template<size_t strSize>
	struct StaticString {
		std::array<char, strSize> str = {};

		constexpr StaticString(const char(&str)[strSize + 1]) {
			std::ranges::copy_n(str, strSize, this->str.begin());
		}

		constexpr size_t size() const {
			return strSize;
		}

		auto begin() const {
			return str.begin();
		}

		auto end() const {
			return str.end();
		}

		operator std::string() const {
			std::string result;

			for (char c : str)
				result.push_back(c);

			return result;
		}

		char operator[](size_t index) const {
			return str[index];
		}
	};

	template<size_t size>
	std::ostream& operator<<(std::ostream& stream, StaticString<size> str) {
		for (char c : str.str)
			stream << c;

		return stream;
	}

	template<size_t size1, size_t size2>
	constexpr bool compareStaticString(StaticString<size1> s1, StaticString<size2> s2) {
		return 0;
	}

	template<size_t size>
	constexpr bool compareStaticString(StaticString<size> s1, StaticString<size> s2) {
		return s1.str == s2.str;
	}

	template<size_t size1, size_t size2>
	constexpr bool operator==(StaticString<size1> s1, StaticString<size2> s2) {
		return compareStaticString(s1, s2);
	}

	template <size_t size>
	StaticString(const char(&)[size]) -> StaticString<size - 1>;

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

	inline void expect(bool condition, const std::string& msg) {
		if (!condition)
			throw Exception(msg);
	}

	template<typename Exception>
	inline void expect(bool condition, const std::string& msg) {
		if (!condition)
			throw Exception(msg);
	}


}