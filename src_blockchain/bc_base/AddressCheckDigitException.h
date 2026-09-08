/*
 * AddressCheckDigitException.h
 *
 *  Created on: Aug 6, 2026
 *      Author: iizuka
 */

#ifndef BC_BASE_ADDRESSCHECKDIGITEXCEPTION_H_
#define BC_BASE_ADDRESSCHECKDIGITEXCEPTION_H_

#include "base/Exception.h"

using namespace alinous;

namespace codablecash {

class AddressCheckDigitException : public Exception {
public:
	AddressCheckDigitException(const char* srcfile, int srcline) noexcept;
	AddressCheckDigitException(Exception* cause, const char* srcfile, int srcline) noexcept;
	AddressCheckDigitException(const wchar_t* message, const char* srcfile, int srcline) noexcept;
	AddressCheckDigitException(const wchar_t* message, Exception* cause, const char* srcfile, int srcline) noexcept;
	virtual ~AddressCheckDigitException();
	static const wchar_t* defaultMessage;
};

} /* namespace codablecash */

#endif /* BC_BASE_ADDRESSCHECKDIGITEXCEPTION_H_ */
