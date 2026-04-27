/*
 * SmartcontractCallSequence.h
 *
 *  Created on: Mar 22, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_CURSOR_SMARTCONTRACTCALLSEQUENCE_H_
#define SMARTCONTRACT_EXECUTOR_CURSOR_SMARTCONTRACTCALLSEQUENCE_H_

#include "base/ArrayList.h"
#include <cstdint>

using namespace alinous;

namespace codablecash {

class SessionContextCursorData;
class CdbDatabaseSessionId;
class ISystemLogger;

class SmartcontractCallSequence {
public:
	SmartcontractCallSequence();
	virtual ~SmartcontractCallSequence();

	void addFirst(const SessionContextCursorData* data);

	int size() const noexcept;
	SessionContextCursorData* get(int pos) const noexcept;
	int getPosition(const CdbDatabaseSessionId* sessionId) const noexcept;

	bool isSamechain(const SmartcontractCallSequence* other) const noexcept;
	const SessionContextCursorData* getCommonPoint(const SmartcontractCallSequence* other) const noexcept;

	uint64_t getMaxCallSerial() const noexcept;
	SessionContextCursorData* getDataAtCallSerial(uint64_t serial) const noexcept;

	void logSequence(ISystemLogger* logger) const;

private:
	ArrayList<SessionContextCursorData>* list;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_CURSOR_SMARTCONTRACTCALLSEQUENCE_H_ */
