/*
 * SmartcontractCallSequence.cpp
 *
 *  Created on: Mar 22, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_cursor/SmartcontractCallSequence.h"
#include "smartcontract_executor_cursor/SessionContextCursorData.h"

#include "trx/session/base/CdbDatabaseSessionId.h"

#include "smartcontract_executor_cursor/SmartcontractCallSequenceCursor.h"

#include "bc/ISystemLogger.h"

#include "base/Long.h"

#include "base/StackRelease.h"

using alinous::Long;
namespace codablecash {

SmartcontractCallSequence::SmartcontractCallSequence() {
	this->list = new ArrayList<SessionContextCursorData>();
}

SmartcontractCallSequence::~SmartcontractCallSequence() {
	this->list->deleteElements();
	delete this->list;
}

void SmartcontractCallSequence::addFirst(const SessionContextCursorData *data) {
	SessionContextCursorData* d = dynamic_cast<SessionContextCursorData*>(data->copyData());

	this->list->addElement(d, 0);
}

int SmartcontractCallSequence::size() const noexcept {
	return this->list->size();
}

SessionContextCursorData* SmartcontractCallSequence::get(int pos) const noexcept {
	return this->list->get(pos);
}

bool SmartcontractCallSequence::isSamechain(const SmartcontractCallSequence *other) const noexcept {
	uint64_t max = getMaxCallSerial();
	uint64_t omax = other->getMaxCallSerial();

	uint64_t start = max > omax ? omax : max;

	SessionContextCursorData* thisData = getDataAtCallSerial(start);
	SessionContextCursorData* otherData = other->getDataAtCallSerial(start);

	CdbDatabaseSessionId* id = thisData->getSessionId();
	CdbDatabaseSessionId* otherid = otherData->getSessionId();

	return id->equals(otherid);
}

const SessionContextCursorData* SmartcontractCallSequence::getCommonPoint(const SmartcontractCallSequence *other) const noexcept {
	const SessionContextCursorData* ret = nullptr;

	uint64_t max = getMaxCallSerial();
	uint64_t omax = other->getMaxCallSerial();

	uint64_t start = max > omax ? omax : max;

	SmartcontractCallSequenceCursor cursor(this, start);
	SmartcontractCallSequenceCursor ocursor(other, start);

	{
		const SessionContextCursorData* data = cursor.next();
		const SessionContextCursorData* odata = ocursor.next();
		assert(data->getCallSerial() == odata->getCallSerial());

		CdbDatabaseSessionId* trxId = data->getSessionId();
		CdbDatabaseSessionId* otrxId = odata->getSessionId();

		if(trxId->equals(otrxId)){
			ret = data;
		}
	}

	while(ret == nullptr && cursor.hasNext() && ocursor.hasNext()){
		const SessionContextCursorData* data = cursor.next();
		const SessionContextCursorData* odata = ocursor.next();
		assert(data->getCallSerial() == odata->getCallSerial());

		CdbDatabaseSessionId* trxId = data->getSessionId();
		CdbDatabaseSessionId* otrxId = odata->getSessionId();
		if(trxId->equals(otrxId)){
			ret = data;
		}
	}

	return ret;
}

uint64_t SmartcontractCallSequence::getMaxCallSerial() const noexcept {
	int pos = this->list->size() - 1;
	SessionContextCursorData* data = this->list->get(pos);
	return data->getCallSerial();
}

SessionContextCursorData* SmartcontractCallSequence::getDataAtCallSerial(uint64_t serial) const noexcept {
	SessionContextCursorData* ret = nullptr;

	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		SessionContextCursorData* data = this->list->get(i);
		uint64_t callSerial = data->getCallSerial();
		if(callSerial == serial){
			ret = data;
			break;
		}
	}

	return ret;
}

int SmartcontractCallSequence::getPosition(const CdbDatabaseSessionId *sessionId) const noexcept {
	int pos = -1;

	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		SessionContextCursorData* data = this->list->get(i);

		CdbDatabaseSessionId* id = data->getSessionId();
		if(sessionId->equals(id)){
			pos = i;
			break;
		}
	}

	return pos;
}

void SmartcontractCallSequence::logSequence(ISystemLogger *logger) const {
	UnicodeString logstr(L"");

	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		SessionContextCursorData* data = this->list->get(i);

		uint64_t serial = data->getCallSerial();
		CdbDatabaseSessionId* sessionId = data->getSessionId();

		UnicodeString* sstr = Long::toString(serial, 10); __STP(sstr);
		UnicodeString* idstr = sessionId->toString(); __STP(idstr);

		logstr.append(sstr);
		logstr.append(L" ");
		logstr.append(idstr);
		logstr.append(L"\n");
	}

	logger->debugLog(ISystemLogger::DEBUG_DBSESSION_INFO, &logstr, __FILE__, __LINE__);
}

} /* namespace codablecash */
