/*
 * MoveTestSessionHeadDitector.cpp
 *
 *  Created on: Mar 19, 2026
 *      Author: iizuka
 */

#include "MoveTestSessionHeadDitector.h"

#include "smartcontract_executor_index/InstanceSessionContext.h"


namespace codablecash {

MoveTestSessionHeadDitector::MoveTestSessionHeadDitector(SmartcontractExecContextRegistory* contextRegistory)
		: AbstractSessionDetector(contextRegistory) {
	this->heads = new ArrayList<InstanceSessionContext>();
}

MoveTestSessionHeadDitector::~MoveTestSessionHeadDitector() {
	this->heads->deleteElements();
	delete this->heads;
}

void MoveTestSessionHeadDitector::registerHead(const InstanceSessionContext *context, const SmartcontractExecContextData *data) {
	InstanceSessionContext* copiedContext = dynamic_cast<InstanceSessionContext*>(context->copyData());

	this->heads->addElement(copiedContext);
}

} /* namespace codablecash */
