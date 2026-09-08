/*
 * SimnetConfigException.cpp
 *
 *  Created on: Sep 7, 2026
 *      Author: iizuka
 */

#include "sim/SimnetConfigException.h"

#include "base/UnicodeString.h"

namespace codablecash {

const wchar_t* SimnetConfigException::defaultMessage = L"Simulartor config error. ";

SimnetConfigException::SimnetConfigException(const char* srcfile, int srcline) noexcept : Exception(srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
}
SimnetConfigException::SimnetConfigException(Exception* cause, const char* srcfile, int srcline) noexcept : Exception(cause, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
}
SimnetConfigException::SimnetConfigException(const wchar_t* message, const char* srcfile, int srcline) noexcept : Exception(message, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
	this->message->append(message);
}
SimnetConfigException::SimnetConfigException(const wchar_t* message, Exception* cause, const char* srcfile, int srcline) noexcept : Exception(message, cause, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
	this->message->append(message);
}
SimnetConfigException::~SimnetConfigException() {
}

} /* namespace codablecash */
