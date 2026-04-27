/*
 * ProjectNotFoundException.h
 *
 *  Created on: Nov 30, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_CACHE_PROJECTNOTFOUNDEXCEPTION_H_
#define SMARTCONTRACT_CACHE_PROJECTNOTFOUNDEXCEPTION_H_

#include "base/Exception.h"

using namespace alinous;

namespace codablecash {

class ProjectNotFoundException : public Exception {
public:
	ProjectNotFoundException(const char* srcfile, int srcline) noexcept;
	ProjectNotFoundException(Exception* cause, const char* srcfile, int srcline) noexcept;
	ProjectNotFoundException(const wchar_t* message, const char* srcfile, int srcline) noexcept;
	ProjectNotFoundException(const wchar_t* message, Exception* cause, const char* srcfile, int srcline) noexcept;
	virtual ~ProjectNotFoundException();

	static const wchar_t* defaultMessage;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_CACHE_PROJECTNOTFOUNDEXCEPTION_H_ */
