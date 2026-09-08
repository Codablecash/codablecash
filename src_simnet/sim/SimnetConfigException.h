/*
 * SimnetConfigException.h
 *
 *  Created on: Sep 7, 2026
 *      Author: iizuka
 */

#ifndef SIM_SIMNETCONFIGEXCEPTION_H_
#define SIM_SIMNETCONFIGEXCEPTION_H_

#include "base/Exception.h"

using namespace alinous;

namespace codablecash {

class SimnetConfigException : public Exception {
public:
	SimnetConfigException(const char* srcfile, int srcline) noexcept;
	SimnetConfigException(Exception* cause, const char* srcfile, int srcline) noexcept;
	SimnetConfigException(const wchar_t* message, const char* srcfile, int srcline) noexcept;
	SimnetConfigException(const wchar_t* message, Exception* cause, const char* srcfile, int srcline) noexcept;
	virtual ~SimnetConfigException();
	static const wchar_t* defaultMessage;
};

} /* namespace codablecash */

#endif /* SIM_SIMNETCONFIGEXCEPTION_H_ */
