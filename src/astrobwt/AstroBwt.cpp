/*
 * AstroBwt.cpp
 *
 *  Created on: 2022/04/12
 *      Author: iizuka
 */

#include "AstroBwt.h"

#include "base/StackRelease.h"

#include "base_io/ByteBuffer.h"

#include "Sha3256.h"
#include "Salsa20.h"
#include "BwtTransform.h"
#include "osenv/memory.h"

using alinous::Mem;

namespace codablecash {

AstroBwt::AstroBwt() {
	this->stage1 = new uint8_t[stage1_length];
	this->stage2 = new uint8_t[stage2_max_length];
	this->bwtBuff = ByteBuffer::allocateWithEndian(stage2_max_length, true);
	this->bwtTransform = new BwtTransform(stage2_max_length);
	this->transformMask = DEFAULT_MASK2;
}

AstroBwt::~AstroBwt() {
	delete [] this->stage1;
	delete [] this->stage2;
	delete this->bwtBuff;
	delete this->bwtTransform;
}

ByteBuffer* AstroBwt::POW(const uint8_t* data, int length) const {

	// 1. Calulate SHA3-256 of input data
	ByteBuffer* sha3First = Sha3_256::sha3_256((const char*)data, length, false); __STP(sha3First);

	// 2. Expand data using Salsa20  cipher  69371  bytes
	uint8_t counter[16];

	Mem::memset(this->stage1, 0, stage1_length);
	Mem::memset(counter, 0, 16);

	const uint8_t* key = sha3First->array();

	Salsa20::crypto_stream_xor(stage1, stage1, stage1_length, counter, 0, key);

	// 3. Calculate BWT of step 2
	this->bwtTransform->transform(this->bwtBuff, stage1, stage1_length);

	// 4. Calculate SHA3-256 of BWT data
	ByteBuffer* sha3Fourth = Sha3_256::sha3_256((const char*)this->bwtBuff->array(), this->bwtBuff->limit(), false); __STP(sha3Fourth);

	// 5. Expand data using Salsa20  cipher  69371  + random number based on step 4
	//uint32_t rndValue = sha3Fourth->getInt(0) & 0xfffff;

	uint32_t rndValue = sha3Fourth->getInt(0) & this->transformMask; // 0xffff;
	uint32_t stage2_length = this->stage1_length + rndValue;

	Mem::memset(this->stage2, 0, stage2_length);
	Mem::memset(counter, 0, 16);

	Salsa20::crypto_stream_xor(stage2, stage2, stage2_length, counter, 0, key);

	// 6. Calculate BWT of data from step 5
	this->bwtTransform->transform(this->bwtBuff ,this->stage2, stage2_length);


	// 7. Calculate SHA3-256 of BWT data from step 6
	ByteBuffer* sha3Final = Sha3_256::sha3_256((const char*)this->bwtBuff->array(), this->bwtBuff->limit(), false);

	return sha3Final;
}



} /* namespace codablecash */
