/*
 * ProjectNotFoundException.cpp
 *
 *  Created on: Nov 30, 2025
 *      Author: iizuka
 */

#include "smartcontract_cache/ProjectNotFoundException.h"

#include "base/UnicodeString.h"


namespace codablecash {

const wchar_t* ProjectNotFoundException::defaultMessage = L"Smartcontract Project Error.";

ProjectNotFoundException::ProjectNotFoundException(const char* srcfile, int srcline) noexcept : Exception(srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
}
ProjectNotFoundException::ProjectNotFoundException(Exception* cause, const char* srcfile, int srcline) noexcept : Exception(cause, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
}
ProjectNotFoundException::ProjectNotFoundException(const wchar_t* message, const char* srcfile, int srcline) noexcept : Exception(message, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
	this->message->append(message);
}
ProjectNotFoundException::ProjectNotFoundException(const wchar_t* message, Exception* cause, const char* srcfile, int srcline) noexcept : Exception(message, cause, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
	this->message->append(message);
}

ProjectNotFoundException::~ProjectNotFoundException() {
}

} /* namespace codablecash */
