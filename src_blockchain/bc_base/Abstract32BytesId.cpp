/*
 * Abstract32BytesId.cpp
 *
 *  Created on: 2023/01/23
 *      Author: iizuka
 */

#include "bc_base/Abstract32BytesId.h"

#include "base_io/ByteBuffer.h"

#include "numeric/BigInteger.h"

#include "base/StackRelease.h"

#include "bc_wallet/HdWalletSeed.h"

#include "bc/ISystemLogger.h"

namespace codablecash {

const BigInteger Abstract32BytesId::Q(L"ff66c4652cbb54e13e4cc75898014aef72332e147343a95031cf416ca9f77ce7", 16);
const BigInteger Abstract32BytesId::G(L"e000000000000000000000000000000000000000000000000000000000000002", 16);

Abstract32BytesId::Abstract32BytesId() {
	this->id = nullptr;
}

Abstract32BytesId::Abstract32BytesId(const Abstract32BytesId &inst) {
	const ByteBuffer* buff = inst.id;
	this->id = ByteBuffer::wrapWithEndian(buff->array(), buff->capacity(), true);
}

Abstract32BytesId::Abstract32BytesId(const char *binary, int length) {
	this->id = ByteBuffer::wrapWithEndian((const uint8_t*)binary, length, true);
}
Abstract32BytesId::~Abstract32BytesId() {
	delete this->id;
}

int Abstract32BytesId::size() const noexcept {
	return this->id->limit();
}

const char* Abstract32BytesId::toArray() const noexcept {
	return (const char*)this->id->array();
}

int Abstract32BytesId::binarySize() const {
	int total = sizeof(int32_t);
	total += this->id->capacity();

	return total;
}

void Abstract32BytesId::toBinary(ByteBuffer *out) const {
	int cap = this->id->capacity();
	out->putInt(cap);
	out->put(this->id->array(), cap);
}

bool Abstract32BytesId::equals(const Abstract32BytesId *other) const noexcept {
	return this->id->binaryEquals(other->id);
}

int Abstract32BytesId::compareTo(const Abstract32BytesId *other) const noexcept {
	return this->id->binaryCmp(other->id);
}

BigInteger* Abstract32BytesId::toBigInteger() const noexcept {
	this->id->position(0);

	const char* bin = (const char*)this->id->array();
	int length = this->id->limit();

	BigInteger* ret = BigInteger::fromBinary(bin, length);
	return ret;
}

UnicodeString* Abstract32BytesId::toString() const {
	BigInteger* bigInt = toBigInteger(); __STP(bigInt);

	UnicodeString str = bigInt->toString(16);

	return new UnicodeString(str);
}

int Abstract32BytesId::hashCode() const {
	BigInteger* bint = BigInteger::fromBinary((const char*)this->id->array(), this->id->limit()); __STP(bint);
	int32_t hash = (int32_t)bint->longValue();
	return hash;
}

int Abstract32BytesId::ValueCompare::operator ()(const Abstract32BytesId *const a, const Abstract32BytesId *const b) const {
	return a->compareTo(b);
}

ByteBuffer* Abstract32BytesId::makeRandom16Bytes() {
	int size = 0;
	BigInteger p(L"0", 16);
	do{
		BigInteger seed = BigInteger::ramdom();

		BigInteger s = seed.mod(Abstract32BytesId::Q);
		p = G.modPow(s, Abstract32BytesId::Q);
		size = p.binarySize();
	} while(size != 32);

	ByteBuffer* buff = p.toBinary(); __STP(buff);
	buff->position(0);

	return __STP_MV(buff);
}

void Abstract32BytesId::debugLog(ISystemLogger *logger) const {
	UnicodeString* str = toString(); __STP(str);

	UnicodeString message(L"id : ");
	message.append(str);
	logger->debugLog(ISystemLogger::DEBUG_DBSESSION_INFO, &message, __FILE__, __LINE__);
}

} /* namespace codablecash */
