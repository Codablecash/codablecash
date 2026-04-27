/*
 * ModuleNotFoundException.cpp
 *
 *  Created on: Mar 2, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_operations/ModuleNotFoundException.h"

#include "base/UnicodeString.h"

namespace codablecash {

const wchar_t* ModuleNotFoundException::defaultMessage = L"The module does not exists.";

ModuleNotFoundException::ModuleNotFoundException(const char* srcfile, int srcline) noexcept : Exception(srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
}
ModuleNotFoundException::ModuleNotFoundException(Exception* cause, const char* srcfile, int srcline) noexcept : Exception(cause, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
}
ModuleNotFoundException::ModuleNotFoundException(const wchar_t* message, const char* srcfile, int srcline) noexcept : Exception(message, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
	this->message->append(message);
}
ModuleNotFoundException::ModuleNotFoundException(const wchar_t* message, Exception* cause, const char* srcfile, int srcline) noexcept : Exception(message, cause, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
	this->message->append(message);
}
ModuleNotFoundException::~ModuleNotFoundException() {
}


} /* namespace codablecash */
