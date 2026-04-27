/*
 * ClassNotFoundException.h
 *
 *  Created on: Dec 2, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_INSTANCE_CLASSNOTFOUNDEXCEPTION_H_
#define SMARTCONTRACT_INSTANCE_CLASSNOTFOUNDEXCEPTION_H_

#include "base/Exception.h"

using namespace alinous;

namespace codablecash {

class ClassNotFoundException : public alinous::Exception {
public:
	ClassNotFoundException(const char* srcfile, int srcline) noexcept;
	ClassNotFoundException(Exception* cause, const char* srcfile, int srcline) noexcept;
	ClassNotFoundException(const wchar_t* message, const char* srcfile, int srcline) noexcept;
	ClassNotFoundException(const wchar_t* message, Exception* cause, const char* srcfile, int srcline) noexcept;
	virtual ~ClassNotFoundException();
	static const wchar_t* defaultMessage;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_INSTANCE_CLASSNOTFOUNDEXCEPTION_H_ */
