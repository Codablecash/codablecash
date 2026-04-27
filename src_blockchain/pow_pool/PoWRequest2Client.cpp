/*
 * PoWRequest2Client.cpp
 *
 *  Created on: Apr 20, 2026
 *      Author: iizuka
 */

#include "pow_pool/PoWRequest2Client.h"
#include "pow_pool/PoWRequestData.h"

#include "numeric/BigInteger.h"

#include "bc_base/BinaryUtils.h"

#include "base/StackRelease.h"

#include "base_io/ByteBuffer.h"


namespace codablecash {

PoWRequest2Client::PoWRequest2Client(const PoWRequest2Client &inst) {
	this->waiting = inst.waiting;
	this->data = inst.data != nullptr ? new PoWRequestData(*inst.data) : nullptr;
	this->solt = inst.solt != nullptr ? new BigInteger(*inst.solt) : nullptr;
}

PoWRequest2Client::PoWRequest2Client() {
	this->waiting = false;
	this->data = nullptr;
	this->solt = nullptr;
}

PoWRequest2Client::~PoWRequest2Client() {
	delete this->data;
	delete this->solt;
}

int PoWRequest2Client::binarySize() const {
	BinaryUtils::checkNotNull(this->data);
	BinaryUtils::checkNotNull(this->solt);

	int total = sizeof(uint8_t); // suspended
	total += this->data->binarySize();

	total += sizeof(uint32_t);
	total += this->solt->binarySize();

	return total;
}

void PoWRequest2Client::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->data);
	BinaryUtils::checkNotNull(this->solt);

	out->put(this->waiting ? 1 : 0);
	this->data->toBinary(out);

	ByteBuffer* buff = this->solt->toBinary(); __STP(buff);

	int length = buff->limit();
	out->putInt(length);
	out->put(buff);
}

PoWRequest2Client* PoWRequest2Client::fromBinary(ByteBuffer *in) {
	uint8_t bl = in->get();
	bool suspended = bl != 0;

	PoWRequestData* data = PoWRequestData::fromBinary(in); __STP(data);

	int length = in->getInt();
	uint8_t* buff = new uint8_t[length]; StackArrayRelease<uint8_t> __buff(buff);
	in->get(buff, length);

	BigInteger* solt = BigInteger::fromBinary((const char*)buff, length); __STP(solt);

	PoWRequest2Client* request = new PoWRequest2Client();
	request->setRequestData(data);
	request->setSolt(solt);
	request->setSuspended(bl);

	return request;
}

void PoWRequest2Client::setRequestData(const PoWRequestData *data) {
	delete this->data, this->data = nullptr;
	this->data = new PoWRequestData(*data);
}

void PoWRequest2Client::setSolt(const BigInteger *solt) {
	delete this->solt, this->solt = nullptr;
	this->solt = new BigInteger(*solt);
}

void PoWRequest2Client::setSuspended(bool bl) {
	this->waiting = bl;
}

} /* namespace codablecash */
