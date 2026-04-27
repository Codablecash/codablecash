/*
 * SmartcontractCallSequenceBuilder.cpp
 *
 *  Created on: Mar 23, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_cursor/SmartcontractCallSequenceBuilder.h"
#include "smartcontract_executor_cursor/SessionContextCursor.h"
#include "smartcontract_executor_cursor/SmartcontractCallSequence.h"
#include "smartcontract_executor_cursor/SessionContextCursorData.h"

#include "base/StackRelease.h"


namespace codablecash {

SmartcontractCallSequenceBuilder::SmartcontractCallSequenceBuilder(SmartcontractExecContextRegistory* contextRegistory) {
	this->contextRegistory = contextRegistory;
	this->sequence = nullptr;
}

SmartcontractCallSequenceBuilder::~SmartcontractCallSequenceBuilder() {
	this->contextRegistory = nullptr;
	delete this->sequence;
}

void SmartcontractCallSequenceBuilder::build(const CdbDatabaseSessionId *trxId) {
	SmartcontractCallSequence* sequence = new SmartcontractCallSequence(); __STP(sequence);

	SessionContextCursor cursor(this->contextRegistory);

	cursor.setPosition(trxId);
	{
		SessionContextCursorData* data = cursor.getData(); __STP(data);
		sequence->addFirst(data);
	}

	while(cursor.hasPrevious()){
		cursor.previous();
		SessionContextCursorData* data = cursor.getData(); __STP(data);

		sequence->addFirst(data);
	}

	delete this->sequence, this->sequence = nullptr;
	this->sequence = __STP_MV(sequence);
}

} /* namespace codablecash */
