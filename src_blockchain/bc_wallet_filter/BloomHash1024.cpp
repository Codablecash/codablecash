/*
 * BloomHash512.cpp
 *
 *  Created on: May 6, 2025
 *      Author: iizuka
 */

#include "base_io/ByteBuffer.h"
#include "bc_wallet_filter/BloomHash1024.h"


namespace codablecash {

BloomHash1024::BloomHash1024(const BloomHash1024 &inst) {
	this->values[0] = inst.values[0];
	this->values[1] = inst.values[1];
	this->values[2] = inst.values[2];

	this->flag = inst.flag;
}

BloomHash1024::BloomHash1024(uint8_t v0, uint8_t v1, uint8_t v2, uint16_t flag) {
	this->values[0] = v0;
	this->values[1] = v1;
	this->values[2] = v2;

	this->flag = flag;
}

BloomHash1024::BloomHash1024(uint16_t v0, uint16_t v1, uint16_t v2) {
	this->values[0] = (v0 & FILTER8);
	this->values[1] = (v1 & FILTER8);
	this->values[2] = (v2 & FILTER8);

	this->flag = 0;

	setBits(v0, 0);
	setBits(v1, 1);
	setBits(v2, 2);
}

BloomHash1024::~BloomHash1024() {

}

/**
 * make flag
 * @param base
 * @param pos
 */
void BloomHash1024::setBits(uint16_t base, int pos) noexcept {
	uint16_t val = base >> 8;
	val = val << (pos * NUM_EXTRA_BITS);

	uint16_t mask = ~(FILTER2 << (pos * NUM_EXTRA_BITS));

	// clear flag
	this->flag = this->flag & mask;

	// set value
	this->flag = this->flag | val;
}

uint16_t BloomHash1024::getUpperBits(int pos) const noexcept {
	uint16_t val = this->flag >> (pos * NUM_EXTRA_BITS);

	val = val & FILTER2;
	return val << 8;
}

uint16_t BloomHash1024::getValue(int pos) const noexcept {
	uint16_t value = this->values[pos];

	uint16_t upper = getUpperBits(pos);

	return value | upper;
}

int BloomHash1024::binarySize() const {
	return sizeof(uint8_t) * 3 + sizeof(this->flag);
}

void BloomHash1024::toBinary(ByteBuffer *out) {
	out->put(this->values[0]);
	out->put(this->values[1]);
	out->put(this->values[2]);

	out->putShort(this->flag);
}

BloomHash1024* BloomHash1024::createFromBinary(ByteBuffer *in) {
	uint8_t v0 = in->get();
	uint8_t v1 = in->get();
	uint8_t v2 = in->get();
	uint16_t flag = in->getShort();

	return new BloomHash1024(v0, v1, v2, flag);
}

bool BloomHash1024::equals(const BloomHash1024 *other) const noexcept {
	return this->values[0] == other->values[0] &&
			this->values[1] == other->values[1] &&
			this->values[2] == other->values[2] &&
			this->flag == other->flag;
}

BloomHash1024* BloomHash1024::copyData() const noexcept {
	return new BloomHash1024(*this);
}

} /* namespace codablecash */
