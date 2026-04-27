/*
 * SmartcontractExecutionException.cpp
 *
 *  Created on: Dec 2, 2025
 *      Author: iizuka
 */

#include "smartcontract_instance/SmartcontractExecutionException.h"

#include "base/UnicodeString.h"


namespace codablecash {

const wchar_t* SmartcontractExecutionException::defaultMessage = L"Smartcontract Project Error.";

SmartcontractExecutionException::SmartcontractExecutionException(const char* srcfile, int srcline) noexcept : Exception(srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
}
SmartcontractExecutionException::SmartcontractExecutionException(Exception* cause, const char* srcfile, int srcline) noexcept : Exception(cause, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
}
SmartcontractExecutionException::SmartcontractExecutionException(const wchar_t* message, const char* srcfile, int srcline) noexcept : Exception(message, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
	this->message->append(message);
}
SmartcontractExecutionException::SmartcontractExecutionException(const wchar_t* message, Exception* cause, const char* srcfile, int srcline) noexcept : Exception(message, cause, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
	this->message->append(message);
}

SmartcontractExecutionException::~SmartcontractExecutionException() {
}

} /* namespace codablecash */
