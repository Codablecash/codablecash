/*
 * CdlDatabaseSessionId.cpp
 *
 *  Created on: 2023/07/08
 *      Author: iizuka
 */

#include "trx/session/base/CdbDatabaseSessionId.h"

#include "osenv/funcs.h"

#include "base/StackRelease.h"

#include "base_io/ByteBuffer.h"

#include "crypto/Sha256.h"


namespace codablecash {

CdbDatabaseSessionId::CdbDatabaseSessionId(const CdbDatabaseSessionId &inst) : Abstract32BytesId(inst) {
}


CdbDatabaseSessionId::CdbDatabaseSessionId() : Abstract32BytesId() {

}

CdbDatabaseSessionId::CdbDatabaseSessionId(const char *binary, int length) : Abstract32BytesId(binary, length) {
}

CdbDatabaseSessionId::~CdbDatabaseSessionId() {

}

CdbDatabaseSessionId* CdbDatabaseSessionId::createRandomId() {
	uint64_t msec = Os::getMicroSec();

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(sizeof(msec), true); __STP(buff);
	buff->putLong(msec);

	ByteBuffer* sha = Sha256::sha256(buff, true); __STP(sha);
	sha->position(0);

	CdbDatabaseSessionId* id = new CdbDatabaseSessionId((const char*)sha->array(), sha->limit());

	return id;
}

CdbDatabaseSessionId* CdbDatabaseSessionId::from32BytesId(const Abstract32BytesId *id) {
	ByteBuffer* buff = id->getByteBuffer();

	char *binary = (char*)buff->array();
	int length = buff->capacity();

	return new CdbDatabaseSessionId(binary, length);
}

int CdbDatabaseSessionId::binarySize() const {
	return Abstract32BytesId::binarySize();
}

void CdbDatabaseSessionId::toBinary(ByteBuffer *out) const {
	Abstract32BytesId::toBinary(out);
}

CdbDatabaseSessionId* CdbDatabaseSessionId::createFromBinary(ByteBuffer *in) {
	CdbDatabaseSessionId* sessionId = new CdbDatabaseSessionId();

	int cap = in->getInt();

	uint8_t* buff = new uint8_t[cap];
	StackArrayRelease<uint8_t> __st_buff(buff);

	in->get(buff, cap);

	sessionId->id = ByteBuffer::wrapWithEndian(buff, cap, true);
	sessionId->id->position(0);

	return sessionId;
}

IBlockObject* CdbDatabaseSessionId::copyData() const noexcept {
	return new CdbDatabaseSessionId(*this);
}

} /* namespace codablecash */
