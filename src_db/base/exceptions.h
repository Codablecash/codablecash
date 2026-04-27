/*
 * exceptions.h
 *
 *  Created on: 2018/04/15
 *      Author: iizuka
 */

#ifndef BASE_EXCEPTIONS_H_
#define BASE_EXCEPTIONS_H_

#include "base/Exception.h"

namespace alinous {

class NumberFormatException : public Exception{
public:
	NumberFormatException(const char* srcfile, int srcline) noexcept;
	NumberFormatException(Exception* cause, const char* srcfile, int srcline) noexcept;
	NumberFormatException(const wchar_t* message, const char* srcfile, int srcline) noexcept;
	NumberFormatException(const wchar_t* message, Exception* cause, const char* srcfile, int srcline) noexcept;
	virtual ~NumberFormatException();
	static const wchar_t* defaultMessage;
};

class IllegalArgumentException : public Exception{
public:
	IllegalArgumentException(const char* srcfile, int srcline) noexcept;
	IllegalArgumentException(Exception* cause, const char* srcfile, int srcline) noexcept;
	IllegalArgumentException(const wchar_t* message, const char* srcfile, int srcline) noexcept;
	IllegalArgumentException(const wchar_t* message, Exception* cause, const char* srcfile, int srcline) noexcept;
	virtual ~IllegalArgumentException();
	static const wchar_t* defaultMessage;
};

class UnsupportedFunctionException : public Exception{
public:
	UnsupportedFunctionException(const char* srcfile, int srcline) noexcept;
	UnsupportedFunctionException(Exception* cause, const char* srcfile, int srcline) noexcept;
	UnsupportedFunctionException(const wchar_t* message, const char* srcfile, int srcline) noexcept;
	UnsupportedFunctionException(const wchar_t* message, Exception* cause, const char* srcfile, int srcline) noexcept;
	virtual ~UnsupportedFunctionException();
	static const wchar_t* defaultMessage;
};

class CompilantUnitAnalyzeException : public Exception{
public:
	CompilantUnitAnalyzeException(const char* srcfile, int srcline) noexcept;
	CompilantUnitAnalyzeException(Exception* cause, const char* srcfile, int srcline) noexcept;
	CompilantUnitAnalyzeException(const wchar_t* message, const char* srcfile, int srcline) noexcept;
	CompilantUnitAnalyzeException(const wchar_t* message, Exception* cause, const char* srcfile, int srcline) noexcept;
	virtual ~CompilantUnitAnalyzeException();
	static const wchar_t* defaultMessage;
};


} /* namespace alinous */

#endif /* BASE_EXCEPTIONS_H_ */
