/*
 * AddressCheckDigitException.cpp
 *
 *  Created on: Aug 6, 2026
 *      Author: iizuka
 */

#include "bc_base/AddressCheckDigitException.h"

#include "base/UnicodeString.h"


namespace codablecash {

const wchar_t* AddressCheckDigitException::defaultMessage = L"Checkdigit of the address is wrong. ";

AddressCheckDigitException::AddressCheckDigitException(const char* srcfile, int srcline) noexcept : Exception(srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
}
AddressCheckDigitException::AddressCheckDigitException(Exception* cause, const char* srcfile, int srcline) noexcept : Exception(cause, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
}
AddressCheckDigitException::AddressCheckDigitException(const wchar_t* message, const char* srcfile, int srcline) noexcept : Exception(message, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
	this->message->append(message);
}
AddressCheckDigitException::AddressCheckDigitException(const wchar_t* message, Exception* cause, const char* srcfile, int srcline) noexcept : Exception(message, cause, srcfile, srcline) {
	this->message = new UnicodeString(defaultMessage);
	this->message->append(message);
}
AddressCheckDigitException::~AddressCheckDigitException() {
}

} /* namespace codablecash */
