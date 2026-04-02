#pragma once

#include <format>
#include <exception>
#include <string>
#include <iostream>
#include <algorithm>
#include <array>
#include <sstream>
#include <map>
#include <stack>

#include "../Metaprogramming/StaticString.h"

#ifndef SNLDebugLevel
	#define SNLDebugLevel 0
#endif

#define SNLDebugCall(LEVEL, CALL) if constexpr (::snl::debugLevel >= LEVEL) CALL;


#ifdef _MSC_VER
	#define SNLMSVCCall(CALL) CALL
#else
	#define SNLMSVCCall(CALL) 
#endif

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

	//struct Exception : std::exception {
	//	const std::string msg;
	//
	//	inline Exception(const std::string& msg) : msg("SNL encountered an error: " + msg) {
	//		debug << "SNL exception generated: " + msg << std::endl;
	//	}
	//
	//	const char* what() {
	//		return msg.c_str();
	//	}
	//};
	//
	//template<StaticString exceptionClass>
	//struct ClassedException : Exception {
	//	inline ClassedException(const std::string& msg) : Exception(msg + ", exception class: " + (std::string)exceptionClass) {}
	//};

	enum class ErrorLevel {
		ERROR,
		WARNING
	};

	enum class LogLevel {
		FULL,
		SHORT,
		SILENT
	};

	enum class SuppressionLevel {
		ACTIVE,
		SUPPRESSED
	};

	struct GenericError : std::exception {
		virtual std::string msg() const = 0;
		
		const char* what() const final {
			std::string message = msg();
			char* buf = new char[message.size()];
			std::memcpy(buf, message.c_str(), message.size());
			return buf;
		}
	};

	template<StaticString msgFormat, typename... Ts>
	struct ErrorBase : GenericError {
		std::tuple<Ts...> args;

		ErrorBase() requires (sizeof...(Ts) != 0) = default;
		ErrorBase(Ts... args) : args(args...) {}

		std::string msg() const {
			return std::apply(
				[&](auto&&... args) {
					return std::vformat(msgFormat.str(), std::make_format_args(args...));
				},
				args
			);
		}
	};

	template<StaticString msgFormat, typename... Ts>
	struct Error : ErrorBase<msgFormat, Ts...> {
		using ErrorBase<msgFormat, Ts...>::ErrorBase;

		static constexpr ErrorLevel level = ErrorLevel::ERROR;
	};

	template<StaticString msgFormat, typename... Ts>
	struct Warning : ErrorBase<msgFormat, Ts...> {
		using ErrorBase<msgFormat, Ts...>::ErrorBase;

		static constexpr ErrorLevel level = ErrorLevel::WARNING;
	};

	template<typename ErrorClass>
	concept IsError = ErrorClass::level == ErrorLevel::ERROR;

	template<typename ErrorClass>
	concept IsWarning = ErrorClass::level == ErrorLevel::WARNING;

	class ErrorRegister {
	public:
		struct ErrorSettings {
			LogLevel logStatus;
			SuppressionLevel suppressionStatus;
		};

		template<typename ErrorClass>
		static std::string errorHeader() {
			switch (ErrorClass::level) {
			case ErrorLevel::ERROR:
				return std::string("SNL error"); //+ typeid(ErrorClass).name();
				break;
			case ErrorLevel::WARNING:
				return std::string("SNL warning"); //+ typeid(ErrorClass).name();
				break;
			}
		}

	private:
		std::map<const std::type_info*, ErrorSettings> errorStates;
		std::vector<GenericError*> errors;
		std::stack<GenericError*> toHandle;

	public:
		~ErrorRegister() {
			for (GenericError* error : errors)
				delete error;
		}

		template<typename ErrorClass>
		void registerError(const ErrorClass& error) {
			if (!errorStates.count(&typeid(ErrorClass)))
				errorStates[&typeid(ErrorClass)] = 
				{ LogLevel::FULL, SuppressionLevel::ACTIVE };

			GenericError* copy = new ErrorClass(error);

			errors.push_back(copy);
			toHandle.push(copy);

			switch (errorStates.at(&typeid(ErrorClass)).logStatus)
			{
			case LogLevel::FULL:
				debug << errorHeader<ErrorClass>() << ": " << error.msg() << std::endl;
				break;
			case LogLevel::SHORT:
				debug << errorHeader<ErrorClass>() << std::endl;
				break;
			case LogLevel::SILENT:
				break;
			}

			if constexpr (IsError<ErrorClass>)
				throw error;
		}

		void setSuppression(const std::type_info& errorClass, SuppressionLevel status) {
			if (!errorStates.count(&errorClass))
				errorStates[&errorClass] =
				{ LogLevel::FULL, SuppressionLevel::ACTIVE };

				errorStates[&errorClass].suppressionStatus = status;
		}

		void setLogLevel(const std::type_info& errorClass, LogLevel status) {
			if (!errorStates.count(&errorClass))
				errorStates[&errorClass] =
				{ LogLevel::FULL, SuppressionLevel::ACTIVE };

			errorStates[&errorClass].logStatus = status;
		}

		template<typename ErrorClass>
		void throwError(const ErrorClass& error) {
			if(!errorStates.count(&typeid(ErrorClass)))
				registerError(error);
			else if(errorStates.at(&typeid(ErrorClass)).suppressionStatus == SuppressionLevel::ACTIVE)
				registerError(error);
		}

		template<typename ErrorClass>
		void forceThrow(const ErrorClass& error) {
			registerError(error);
		}

		bool hasError() {
			return !toHandle.empty();
		}

		GenericError& fetchError() {
			GenericError& error = *toHandle.top();
			toHandle.pop();
			return error;
		}
	};

	static ErrorRegister errorRegister;

	template<typename ErrorClass>
	inline void throwError(const ErrorClass& error) {
		errorRegister.throwError<ErrorClass>(error);
	}

	template<IsError ErrorClass>
	[[noreturn]] void throwError(auto... args) {
		errorRegister.throwError<ErrorClass>(ErrorClass(args...));
	}

	template<IsError ErrorClass>
	[[noreturn]] void forceThrow(const ErrorClass& error) {
		errorRegister.forceThrow<ErrorClass>(error);
	}

	template<IsWarning ErrorClass>
	inline void throwError(auto... args) {
		errorRegister.throwError<ErrorClass>(ErrorClass(args...));
	}

	template<IsWarning ErrorClass>
	inline void forceThrow(const ErrorClass& error) {
		errorRegister.forceThrow<ErrorClass>(error);
		throw 0;
	}

	template<typename ErrorClass>
	inline void forceThrow(auto... args) {
		errorRegister.forceThrow<ErrorClass>(ErrorClass(args...));
	}

	template<typename ErrorClass>
	inline void setSuppression(SuppressionLevel level) {
		errorRegister.setSuppression(typeid(ErrorClass), level);
	}

	template<typename ErrorClass>
	inline void setLogLevel(LogLevel level) {
		errorRegister.setLogLevel(typeid(ErrorClass), level);
	}

	template<typename ErrorClass>
	inline void suppressError() {
		setSuppression<ErrorClass>(SuppressionLevel::SUPPRESSED);
	}

	template<typename ErrorClass>
	inline void enableError() {
		setSuppression<ErrorClass>(SuppressionLevel::ACTIVE);
	}

	inline bool hasError() {
		return errorRegister.hasError();
	}

	inline GenericError& fetchError() {
		return errorRegister.fetchError();
	}

	template<typename ErrorClass>
	inline void expect(bool condition, const ErrorClass& error) {
		if (!condition)
			throwError<ErrorClass>(error);
	}

	template<typename ErrorClass>
	inline void expect(bool condition, auto... args) {
		if (!condition)
			throwError<ErrorClass>(args...);
	}

	template<typename ErrorClass>
	class ErrorSuppressor {
		ErrorRegister& errorRegister;
	public:
		ErrorSuppressor() : errorRegister(snl::errorRegister) {
			errorRegister.setSuppression(typeid(ErrorClass), SuppressionLevel::SUPPRESSED);
		}

		ErrorSuppressor(ErrorRegister& errorRegister) : errorRegister(errorRegister) {
			errorRegister.setSuppression(typeid(ErrorClass), SuppressionLevel::SUPPRESSED);
		}

		~ErrorSuppressor() {
			errorRegister.setSuppression(typeid(ErrorClass), SuppressionLevel::ACTIVE);
		}
	};

	template<typename ErrorClass>
	class ErrorSilencer {
		ErrorRegister& errorRegister;
	public:
		ErrorSilencer() : errorRegister(snl::errorRegister) {
			errorRegister.setLogLevel(typeid(ErrorClass), LogLevel::SILENT);
		}

		ErrorSilencer(ErrorRegister& errorRegister) : errorRegister(errorRegister) {
			errorRegister.setLogLevel(typeid(ErrorClass), LogLevel::SILENT);
		}

		~ErrorSilencer() {
			errorRegister.setLogLevel(typeid(ErrorClass), LogLevel::FULL);
		}
	};

	//inline void expect(bool condition, const char* msg) {
	//	if constexpr (debugLevel > 0)
	//		if (!condition)
	//			throw Exception(msg);
	//}
	//
	//template<typename Exception>
	//inline void expect(bool condition, const char* msg) {
	//	if constexpr (debugLevel > 0)
	//		if (!condition)
	//			throw Exception(msg);
	//}
}