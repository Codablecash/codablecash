/*
 * PoWRequestStatusData.cpp
 *
 *  Created on: Apr 20, 2026
 *      Author: iizuka
 */

#include "pow_pool/PoWRequestStatusData.h"

#include "base_io/ByteBuffer.h"


namespace codablecash {

PoWRequestStatusData::PoWRequestStatusData(const PoWRequestStatusData &inst) {
	this->waiting = inst.waiting;
}

PoWRequestStatusData::PoWRequestStatusData() {
	this->waiting = false;
}

PoWRequestStatusData::~PoWRequestStatusData() {

}

void PoWRequestStatusData::setWaiting(bool bl) {
	this->waiting = bl;
}

int PoWRequestStatusData::binarySize() const {
	int total = sizeof(uint8_t);
	return total;
}

void PoWRequestStatusData::toBinary(ByteBuffer *out) const {
	out->put(this->waiting ? 1 : 0);
}

PoWRequestStatusData* PoWRequestStatusData::fromBinary(ByteBuffer *in) {
	uint8_t b = in->get();
	bool suspended = b == 0 ? false : true;

	PoWRequestStatusData* ret = new PoWRequestStatusData();
	ret->setWaiting(suspended);
	return ret;
}

} /* namespace codablecash */
