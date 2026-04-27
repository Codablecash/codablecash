/*
 * ClassNotFoundException.cpp
 *
 *  Created on: Dec 2, 2025
 *      Author: iizuka
 */

#include "smartcontract_instance/ClassNotFoundException.h"

#include "base/UnicodeString.h"

namespace codablecash {

const wchar_t* ClassNotFoundException::defaultMessage = L"Class does not exists.";

ClassNotFoundException::ClassNotFoundException(const char* srcfile, int srcline) noexcept : Exception(srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
}
ClassNotFoundException::ClassNotFoundException(Exception* cause, const char* srcfile, int srcline) noexcept : Exception(cause, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
}
ClassNotFoundException::ClassNotFoundException(const wchar_t* message, const char* srcfile, int srcline) noexcept : Exception(message, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
	this->message->append(message);
}
ClassNotFoundException::ClassNotFoundException(const wchar_t* message, Exception* cause, const char* srcfile, int srcline) noexcept : Exception(message, cause, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
	this->message->append(message);
}
ClassNotFoundException::~ClassNotFoundException() {
}

} /* namespace codablecash */
