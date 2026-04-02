#pragma once

#include <array>
#include <iostream>

namespace snl {
	template<size_t strSize>
	struct StaticString {
		std::array<char, strSize> data = {};

		constexpr StaticString(const char(&str)[strSize + 1]) {
			std::ranges::copy_n(str, strSize, this->data.begin());
		}

		constexpr size_t size() const {
			return strSize;
		}

		auto begin() const {
			return data.begin();
		}

		auto end() const {
			return data.end();
		}

		std::string str() const {
			std::string result;

			for (char c : data)
				result.push_back(c);

			return result;
		}

		operator std::string() const {
			return str();
		}

		char operator[](size_t index) const {
			return data[index];
		}
	};

	template<size_t size>
	std::ostream& operator<<(std::ostream& stream, StaticString<size> str) {
		for (char c : str.data)
			stream << c;

		return stream;
	}

	template<size_t size1, size_t size2>
	constexpr bool compareStaticString(StaticString<size1> s1, StaticString<size2> s2) {
		return 0;
	}

	template<size_t size>
	constexpr bool compareStaticString(StaticString<size> s1, StaticString<size> s2) {
		return s1.data == s2.data;
	}

	template<size_t size1, size_t size2>
	constexpr bool operator==(StaticString<size1> s1, StaticString<size2> s2) {
		return compareStaticString(s1, s2);
	}

	template <size_t size>
	StaticString(const char(&)[size]) -> StaticString<size - 1>;
}