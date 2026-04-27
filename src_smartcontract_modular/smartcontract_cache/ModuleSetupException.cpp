/*
 * ModuleSetupException.cpp
 *
 *  Created on: Dec 1, 2025
 *      Author: iizuka
 */

#include "smartcontract_cache/ModuleSetupException.h"

#include "base/UnicodeString.h"

namespace codablecash {

const wchar_t* ModuleSetupException::defaultMessage = L"Smartcontract Project Setup Error.";

ModuleSetupException::ModuleSetupException(const char* srcfile, int srcline) noexcept : Exception(srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
}
ModuleSetupException::ModuleSetupException(Exception* cause, const char* srcfile, int srcline) noexcept : Exception(cause, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
}
ModuleSetupException::ModuleSetupException(const wchar_t* message, const char* srcfile, int srcline) noexcept : Exception(message, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
	this->message->append(message);
}
ModuleSetupException::ModuleSetupException(const wchar_t* message, Exception* cause, const char* srcfile, int srcline) noexcept : Exception(message, cause, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
	this->message->append(message);
}

ModuleSetupException::~ModuleSetupException() {
}

} /* namespace codablecash */
