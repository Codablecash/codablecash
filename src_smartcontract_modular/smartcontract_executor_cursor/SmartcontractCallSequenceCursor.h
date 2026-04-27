/*
 * SmartcontractCallSequenceCursor.h
 *
 *  Created on: Mar 26, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_CURSOR_SMARTCONTRACTCALLSEQUENCECURSOR_H_
#define SMARTCONTRACT_EXECUTOR_CURSOR_SMARTCONTRACTCALLSEQUENCECURSOR_H_
#include <cstdint>

namespace codablecash {

class SmartcontractCallSequence;
class SessionContextCursorData;

class SmartcontractCallSequenceCursor {
public:
	SmartcontractCallSequenceCursor(const SmartcontractCallSequence* sequence, uint64_t start);
	virtual ~SmartcontractCallSequenceCursor();

	bool hasNext();
	const SessionContextCursorData* next();


private:
	const SmartcontractCallSequence* sequence;
	uint64_t pos;

	SessionContextCursorData* data;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_CURSOR_SMARTCONTRACTCALLSEQUENCECURSOR_H_ */
