/*
 * DebugDefaultLogger.cpp
 *
 *  Created on: 2023/01/13
 *      Author: iizuka
 */

#include "bc/DebugDefaultLogger.h"

#include "base_thread/SysMutex.h"
#include "base_thread/StackUnlocker.h"

#include "base/Exception.h"
#include "base/StackRelease.h"
#include "base/UnicodeString.h"

namespace codablecash {

DebugDefaultLogger::DebugDefaultLogger() : sections(0) {
	this->mutex = new SysMutex();
}

DebugDefaultLogger::~DebugDefaultLogger() {
	delete this->mutex;
}

void DebugDefaultLogger::logException(const Exception *e) noexcept {
	StackUnlocker __lock(this->mutex, __FILE__, __LINE__);

	UnicodeString* fileInfo = e->getFileInfo(); __STP(fileInfo);
	const UnicodeString* message = e->getMessage();

	UnicodeString str(message);
	str.append(L" at ");
	str.append(fileInfo);

	const char* cstr = str.toCString();
	::printf("%s\n", cstr);
	::fflush(stdout);

	delete [] cstr;
}

void DebugDefaultLogger::log(const UnicodeString *message) noexcept {
	StackUnlocker __lock(this->mutex, __FILE__, __LINE__);

	const char* str = message->toCString();
	::printf("%s\n", str);
	::fflush(stdout);

	delete [] str;
}

void DebugDefaultLogger::addSection(int section) noexcept {
	this->sections.addElement(section);
}

void DebugDefaultLogger::debugLog(int section, const UnicodeString *message, const char* srcfile, int srcline) noexcept {
	StackUnlocker __lock(this->mutex, __FILE__, __LINE__);

	if(hasSection(section)){
		const char* str = message->toCString();
		::printf("%s at file: %s, line : %d\n", str, srcfile, srcline);
		::fflush(stdout);

		delete [] str;
	}

}

bool DebugDefaultLogger::hasSection(int section) noexcept {
	bool res = false;

	int maxLoop = this->sections.size();
	for(int i = 0; i != maxLoop; ++i){
		int sec = this->sections.get(i);

		if(sec == section){
			res = true;
		}
	}

	return res;
}

} /* namespace codablecash */
