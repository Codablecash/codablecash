/*
 * DebugDefaultLogger.h
 *
 *  Created on: 2023/01/13
 *      Author: iizuka
 */

#ifndef BC_DEBUGDEFAULTLOGGER_H_
#define BC_DEBUGDEFAULTLOGGER_H_

#include "bc/ISystemLogger.h"

#include "base/RawArrayPrimitive.h"

namespace alinous {
class SysMutex;
}
using namespace alinous;

namespace codablecash {

class DebugDefaultLogger: public ISystemLogger {
public:
	DebugDefaultLogger();
	virtual ~DebugDefaultLogger();

	void addSection(int section) noexcept;

	virtual void logException(const Exception* e) noexcept;
	virtual void log(const UnicodeString* message) noexcept;

	virtual void debugLog(int section, const UnicodeString* message, const char* srcfile, int srcline) noexcept;

	bool hasSection(int section) noexcept;

	SysMutex* getMutex() const noexcept {
		return this->mutex;
	}

private:
	SysMutex* mutex;

	RawArrayPrimitive<int> sections;
};

} /* namespace codablecash */

#endif /* BC_DEBUGDEFAULTLOGGER_H_ */
