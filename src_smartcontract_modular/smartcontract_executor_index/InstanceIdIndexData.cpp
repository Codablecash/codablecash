/*
 * InstanceIdIndexData.cpp
 *
 *  Created on: Nov 28, 2025
 *      Author: iizuka
 */

#include "smartcontract_executor_index/InstanceIdIndexData.h"
#include "smartcontract_executor_index/InstanceSessionContext.h"

#include "trx/session/base/CdbDatabaseSessionId.h"

#include "bc_base/BinaryUtils.h"

#include "base/StackRelease.h"



namespace codablecash {

InstanceIdIndexData::InstanceIdIndexData(const InstanceIdIndexData &inst) {
	this->trxIdList = new ArrayList<InstanceSessionContext, SessionContextReverseCompare>();

	int maxLoop = inst.trxIdList->size();
	for(int i = 0; i != maxLoop; ++i){
		InstanceSessionContext* sessionId = inst.trxIdList->get(i);
		this->trxIdList->addElement(dynamic_cast<InstanceSessionContext*>(sessionId->copyData()));
	}
}

InstanceIdIndexData::InstanceIdIndexData() {
	this->trxIdList = new ArrayList<InstanceSessionContext, SessionContextReverseCompare>();
}

InstanceIdIndexData::~InstanceIdIndexData() {
	this->trxIdList->deleteElements();
	delete this->trxIdList;
}

int InstanceIdIndexData::binarySize() const {
	int total = sizeof(uint16_t);

	int maxLoop = this->trxIdList->size();
	for(int i = 0; i != maxLoop; ++i){
		InstanceSessionContext* sessionId = this->trxIdList->get(i);

		total += sessionId->binarySize();
	}

	return total;
}

void InstanceIdIndexData::toBinary(ByteBuffer *out) const {
	int maxLoop = this->trxIdList->size();
	out->putShort(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		InstanceSessionContext* sessionId = this->trxIdList->get(i);
		sessionId->toBinary(out);
	}
}

InstanceIdIndexData* InstanceIdIndexData::createFromBinary(ByteBuffer *in) {
	InstanceIdIndexData* data = new InstanceIdIndexData(); __STP(data);

	int maxLoop = in->getShort();
	for(int i = 0; i != maxLoop; ++i){
		InstanceSessionContext* id = InstanceSessionContext::createFromBinary(in);

		data->addSessionContext(id);
	}

	return __STP_MV(data);
}

IBlockObject* InstanceIdIndexData::copyData() const noexcept {
	return new InstanceIdIndexData(*this);
}

void InstanceIdIndexData::addSessionContext(InstanceSessionContext *sessionId) {
	this->trxIdList->addElement(sessionId);
}

int InstanceIdIndexData::size() const noexcept {
	return this->trxIdList->size();
}

const InstanceSessionContext* InstanceIdIndexData::get(int i) const noexcept {
	return this->trxIdList->get(i);
}

void InstanceIdIndexData::removeSessionId(const CdbDatabaseSessionId *sessionId) {
	int index = -1;

	int maxLoop = this->trxIdList->size();
	for(int i = 0; i != maxLoop; ++i){
		InstanceSessionContext* ctx = this->trxIdList->get(i);
		const CdbDatabaseSessionId* id = ctx->getSessionId();

		if(sessionId->equals(id)){
			index = i;
			break;
		}
	}

	if(index > -1){
		delete this->trxIdList->remove(index);
	}
}

bool InstanceIdIndexData::isEmpty() const noexcept {
	return this->trxIdList->isEmpty();
}

void InstanceIdIndexData::sort() {
	this->trxIdList->sort();
}

} /* namespace codablecash */
