/*
 * AbstractSessionDetector.cpp
 *
 *  Created on: Mar 18, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_index/AbstractSessionDetector.h"
#include "smartcontract_executor_index/InstanceSessionContext.h"

#include "smartcontract_executor/SmartcontractExecContextRegistory.h"
#include "smartcontract_executor/SmartcontractExecContextData.h"

#include "trx/session/base/CdbDatabaseSessionId.h"

#include "base/StackRelease.h"


namespace codablecash {

AbstractSessionDetector::AbstractSessionDetector(SmartcontractExecContextRegistory* contextRegistory) {
	this->lastIdsList = new ArrayList<CdbDatabaseSessionId, CdbDatabaseSessionId::ValueCompare>();
	this->contextRegistory = contextRegistory;
}

AbstractSessionDetector::~AbstractSessionDetector() {
	this->lastIdsList->deleteElements();
	delete this->lastIdsList;
}

void AbstractSessionDetector::visit(const InstanceSessionContext *context) {
	const CdbDatabaseSessionId* sessionId = context->getSessionId();

	SmartcontractExecContextData* data = this->contextRegistory->getExecContext(sessionId); __STP(data);

	// register lasta id
	const CdbDatabaseSessionId* lastTrx = data->getLastTrxId();
	if(lastTrx != nullptr){
		registerLastId(lastTrx);
	}

	const CdbDatabaseSessionId* trxId = context->getSessionId();
	if(!hasLastId(trxId)){
		registerHead(context, data);
	}
}

void AbstractSessionDetector::registerLastId(const CdbDatabaseSessionId *lastTrxId) {
	CdbDatabaseSessionId* cp = dynamic_cast<CdbDatabaseSessionId*>(lastTrxId->copyData());
	this->lastIdsList->addElement(cp);
}

bool AbstractSessionDetector::hasLastId(const CdbDatabaseSessionId *trxId) {
	this->lastIdsList->sort();

	CdbDatabaseSessionId* sessionId = this->lastIdsList->search(trxId);
	return sessionId != nullptr;
}

} /* namespace codablecash */
