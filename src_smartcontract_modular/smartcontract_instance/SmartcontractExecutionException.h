/*
 * SmartcontractExecutionException.h
 *
 *  Created on: Dec 2, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_INSTANCE_SMARTCONTRACTEXECUTIONEXCEPTION_H_
#define SMARTCONTRACT_INSTANCE_SMARTCONTRACTEXECUTIONEXCEPTION_H_

#include "base/Exception.h"

using namespace alinous;

namespace codablecash {

class SmartcontractExecutionException: public alinous::Exception {
public:
	SmartcontractExecutionException(const char* srcfile, int srcline) noexcept;
	SmartcontractExecutionException(Exception* cause, const char* srcfile, int srcline) noexcept;
	SmartcontractExecutionException(const wchar_t* message, const char* srcfile, int srcline) noexcept;
	SmartcontractExecutionException(const wchar_t* message, Exception* cause, const char* srcfile, int srcline) noexcept;
	virtual ~SmartcontractExecutionException();

	static const wchar_t* defaultMessage;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_INSTANCE_SMARTCONTRACTEXECUTIONEXCEPTION_H_ */
