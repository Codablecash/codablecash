/*
 * Stakebase.cpp
 *
 *  Created on: 2023/05/15
 *      Author: iizuka
 */

#include "bc_block_body/Stakebase.h"

#include "bc_base/BalanceUnit.h"
#include "bc_base/BinaryUtils.h"

namespace codablecash {

Stakebase::Stakebase(const Stakebase &inst) : AbstractUtxoReference(inst) {
	this->amount = inst.amount != nullptr ? dynamic_cast<BalanceUnit*>(inst.amount->copyData()) : nullptr;
}

Stakebase::Stakebase() : AbstractUtxoReference() {
	this->amount = nullptr;
}

Stakebase::~Stakebase() {
	delete this->amount;
}

int Stakebase::binarySize() const {
	BinaryUtils::checkNotNull(this->amount);

	int total = sizeof(uint8_t);
	total += this->amount->binarySize();

	return total;
}

void Stakebase::toBinary(ByteBuffer *out) const {
	out->put(getType());
	this->amount->toBinary(out);
}

void Stakebase::fromBinary(ByteBuffer *in) {
	this->amount = BalanceUnit::fromBinary(in);
	BinaryUtils::checkNotNull(this->amount);
}

IBlockObject* Stakebase::copyData() const noexcept {
	return new Stakebase(*this);
}

void Stakebase::setAmount(const BalanceUnit *amount) noexcept {
	delete this->amount;
	this->amount = dynamic_cast<BalanceUnit*>(amount->copyData());
}

bool Stakebase::checkFilter(const ArrayList<BloomFilter1024> *filtersList) const {
	return false;
}

} /* namespace codablecash */
