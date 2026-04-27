/*
 * ModuleSetupException.h
 *
 *  Created on: Dec 1, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_CACHE_MODULESETUPEXCEPTION_H_
#define SMARTCONTRACT_CACHE_MODULESETUPEXCEPTION_H_

#include "base/Exception.h"

using namespace alinous;

namespace codablecash {

class ModuleSetupException : public Exception {
public:
	ModuleSetupException(const char* srcfile, int srcline) noexcept;
	ModuleSetupException(Exception* cause, const char* srcfile, int srcline) noexcept;
	ModuleSetupException(const wchar_t* message, const char* srcfile, int srcline) noexcept;
	ModuleSetupException(const wchar_t* message, Exception* cause, const char* srcfile, int srcline) noexcept;
	virtual ~ModuleSetupException();

	static const wchar_t* defaultMessage;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_CACHE_MODULESETUPEXCEPTION_H_ */
