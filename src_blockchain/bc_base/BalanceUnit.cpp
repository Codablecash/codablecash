/*
 * BalanceUnit.cpp
 *
 *  Created on: 2019/01/03
 *      Author: iizuka
 */

#include "bc_base/BalanceUnit.h"
#include "base_io/ByteBuffer.h"

namespace codablecash {

BalanceUnit& BalanceUnit::operator =(const BalanceUnit& o) {
	if(&o != this){
		this->type = o.type;
		this->amount = o.amount;
	}
	return *this;
}

BalanceUnit BalanceUnit::operator +(const BalanceUnit &o) const {
	uint64_t val = this->amount + o.amount;
	return BalanceUnit(val);
}

BalanceUnit BalanceUnit::operator -(const BalanceUnit &o) const {
	uint64_t val = this->amount >= o.amount ? this->amount - o.amount : 0;
	return BalanceUnit(val);
}

BalanceUnit BalanceUnit::operator *(const BalanceUnit &o) const {
	uint64_t val = this->amount * o.amount;
	return BalanceUnit(val);
}

BalanceUnit BalanceUnit::operator /(const uint64_t div) const {
	uint64_t val = this->amount / div;
	return BalanceUnit(val);
}

BalanceUnit& BalanceUnit::operator +=(const BalanceUnit &b) {
	this->amount += b.amount;
	return *this;
}

BalanceUnit& BalanceUnit::operator -=(const BalanceUnit &b) {
	uint64_t val = this->amount >= b.amount ? this->amount - b.amount : 0;
	this->amount = val;
	return *this;
}

BalanceUnit::BalanceUnit(const BalanceUnit& inst) {
	this->type = inst.type;
	this->amount = inst.amount;
}

BalanceUnit::BalanceUnit(uint64_t amount) {
	this->type = BalanceUnit::TYPE_PERICA;
	this->amount = amount;
}

BalanceUnit::BalanceUnit() {
	this->type = BalanceUnit::TYPE_PERICA;
	this->amount = 0;
}

BalanceUnit::~BalanceUnit() {
}

uint64_t BalanceUnit::getAmount() const noexcept {
	return this->amount;
}

int BalanceUnit::binarySize() const {
	return sizeof(uint8_t) + sizeof(uint64_t);
}

void BalanceUnit::toBinary(ByteBuffer* out) const {
	out->put(getType());
	out->putLong(this->amount);
}

void BalanceUnit::importBinary(ByteBuffer* in) {
	this->amount = in->getLong();
}

BalanceUnit* BalanceUnit::fromBinary(ByteBuffer *in) {
	BalanceUnit* ret = nullptr;
	uint8_t type = in->get();

	if(type == TYPE_PERICA){
		ret = new BalanceUnit();
		ret->importBinary(in);
	}

	return ret;
}

IBlockObject* BalanceUnit::copyData() const noexcept {
	return new BalanceUnit(*this);
}

int BalanceUnit::compareTo(const BalanceUnit *other) const noexcept {
	return this->amount == other->amount ? 0 : ( this->amount > other->amount ? 1 : -1);
}

int BalanceUnit::compareTo(const BalanceUnit &other) const noexcept {
	return compareTo(&other);
}

} /* namespace codablecash */


