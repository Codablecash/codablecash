/*
 * PoWClientID.cpp
 *
 *  Created on: Apr 13, 2026
 *      Author: iizuka
 */

#include "pow_pool_client/PoWWorkerId.h"

#include "osenv/funcs.h"

#include "crypto/Sha256.h"

#include "base/StackRelease.h"


namespace codablecash {

PoWWorkerId::PoWWorkerId(const PoWWorkerId &inst) : Abstract32BytesId(inst) {

}

PoWWorkerId::PoWWorkerId() : Abstract32BytesId() {

}

PoWWorkerId::PoWWorkerId(const char *binary, int length) : Abstract32BytesId(binary, length) {
}

PoWWorkerId::~PoWWorkerId() {

}

PoWWorkerId* PoWWorkerId::createRandomId() {
	uint64_t msec = Os::getMicroSec();

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(sizeof(msec), true); __STP(buff);
	buff->putLong(msec);

	ByteBuffer* sha = Sha256::sha256(buff, true); __STP(sha);
	sha->position(0);

	PoWWorkerId* id = new PoWWorkerId((const char*)sha->array(), sha->limit());

	return id;
}

IBlockObject* PoWWorkerId::copyData() const noexcept {
	return new PoWWorkerId(*this);
}

PoWWorkerId* PoWWorkerId::fromBinary(ByteBuffer *in) {
	PoWWorkerId* workerId = new PoWWorkerId();

	int cap = in->getInt();

	uint8_t* buff = new uint8_t[cap];
	StackArrayRelease<uint8_t> __st_buff(buff);

	in->get(buff, cap);

	workerId->id = ByteBuffer::wrapWithEndian(buff, cap, true);
	workerId->id->position(0);

	return workerId;
}

} /* namespace codablecash */
