/*
 * ModuleNotFoundException.h
 *
 *  Created on: Mar 2, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_OPERATIONS_MODULENOTFOUNDEXCEPTION_H_
#define SMARTCONTRACT_EXECUTOR_OPERATIONS_MODULENOTFOUNDEXCEPTION_H_

#include "base/Exception.h"

using namespace alinous;

namespace codablecash {

class ModuleNotFoundException : public alinous::Exception {
public:
	ModuleNotFoundException(const char* srcfile, int srcline) noexcept;
	ModuleNotFoundException(Exception* cause, const char* srcfile, int srcline) noexcept;
	ModuleNotFoundException(const wchar_t* message, const char* srcfile, int srcline) noexcept;
	ModuleNotFoundException(const wchar_t* message, Exception* cause, const char* srcfile, int srcline) noexcept;
	virtual ~ModuleNotFoundException();
	static const wchar_t* defaultMessage;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_OPERATIONS_MODULENOTFOUNDEXCEPTION_H_ */
