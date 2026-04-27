/*
 * SmartcontractCallSequenceCursor.cpp
 *
 *  Created on: Mar 26, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_cursor/SmartcontractCallSequenceCursor.h"
#include "smartcontract_executor_cursor/SmartcontractCallSequence.h"
#include "smartcontract_executor_cursor/SessionContextCursorData.h"

namespace codablecash {

SmartcontractCallSequenceCursor::SmartcontractCallSequenceCursor(const SmartcontractCallSequence* sequence, uint64_t start) {
	this->sequence = sequence;
	this->data = nullptr;

	this->pos = 0;

	int maxLoop = sequence->size();
	for(int i = maxLoop - 1; i >= 0; --i){
		SessionContextCursorData* data = sequence->get(i);
		if(data->getCallSerial() == start){
			this->data = sequence->get(i);
			this->pos = i - 1;
			break;
		}
	}
}

SmartcontractCallSequenceCursor::~SmartcontractCallSequenceCursor() {

}

bool SmartcontractCallSequenceCursor::hasNext() {
	bool ret = false;

	if(this->pos >= 0){
		this->data = sequence->get(this->pos--);
		ret = true;
	}

	return ret;
}

const SessionContextCursorData* SmartcontractCallSequenceCursor::next() {
	return this->data;
}

} /* namespace codablecash */
