/*
 * MuSigHashBuilder.cpp
 *
 *  Created on: 2023/02/02
 *      Author: iizuka
 */

#include "musig/MuSigHashBuilder.h"

#include "base/StackRelease.h"

#include "base_io/ByteBuffer.h"

#include "ecda/Secp256k1Point.h"

#include "../../src_ext/yescrypt/sha256.h"

namespace codablecash {

MuSigHashBuilder::MuSigHashBuilder() {
	this->list = new ArrayList<ByteBuffer>();
}

MuSigHashBuilder::~MuSigHashBuilder() {
	this->list->deleteElements();
	delete this->list;
}

void MuSigHashBuilder::add(const Secp256k1Point *pt) {
	ByteBuffer* buff = pt->to65Bytes();
	buff->position(0);

	this->list->addElement(buff);
}

void MuSigHashBuilder::addBigInteger(const BigInteger *bi) {
	ByteBuffer* buff = bi->toBinary();
	buff->position(0);

	this->list->addElement(buff);
}

void MuSigHashBuilder::addArray(const char *data, int length) {
	ByteBuffer* buff = ByteBuffer::wrapWithEndian(data, length, true);
	buff->position(0);

	this->list->addElement(buff);
}

void MuSigHashBuilder::buildHash() {
	int binsize = binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(binsize, true); __STP(buff);

	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		ByteBuffer* b = this->list->get(i);

		buff->put(b);
	}

	{
		buff->position(0);

		SHA256_CTX ctx;
		SHA256_Init(&ctx);
		SHA256_Update(&ctx, buff->array(), buff->limit());

		SHA256_Final((uint8_t *)this->sha256, &ctx);
	}
}

BigInteger MuSigHashBuilder::getResultAsBigInteger() const noexcept {
	BigInteger* res = BigInteger::fromBinary((const char*)this->sha256, 32); __STP(res);

	return *res;
}

int MuSigHashBuilder::binarySize() const noexcept {
	int total = 0;

	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		ByteBuffer* b = this->list->get(i);

		total += b->limit();
	}

	return total;
}

} /* namespace codablecash */
