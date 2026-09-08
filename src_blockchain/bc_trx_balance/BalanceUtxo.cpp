/*
 * BalanceUtxo.cpp
 *
 *  Created on: 2023/02/20
 *      Author: iizuka
 */

#include "bc_trx_balance/BalanceUtxo.h"

#include "bc_base/BalanceUnit.h"
#include "bc_base/AddressDescriptor.h"
#include "bc_base/BinaryUtils.h"

#include "base/StackRelease.h"

#include "base_io/ByteBuffer.h"

#include "crypto/Sha256.h"

#include "bc_trx/UtxoId.h"

#include "bc_trx_balance/FeeShortageException.h"
namespace codablecash {

BalanceUtxo::BalanceUtxo(const BalanceUtxo& inst) : AbstractUtxo(inst), amount(inst.amount) {
	this->addressDesc = inst.addressDesc != nullptr ? new AddressDescriptor(*inst.addressDesc) : nullptr;
}

BalanceUtxo::BalanceUtxo() : AbstractUtxo(), amount(0) {
	this->addressDesc = nullptr;
}

BalanceUtxo::BalanceUtxo(BalanceUnit amount) : amount(amount) {
	this->addressDesc = nullptr;
}

BalanceUtxo::~BalanceUtxo() {
	delete this->addressDesc;
}

uint8_t BalanceUtxo::getType() const noexcept {
	return AbstractUtxo::TRX_UTXO_BALANCE;
}

void BalanceUtxo::setAddress(const AddressDescriptor *desc) noexcept {
	delete this->addressDesc;
	this->addressDesc = new AddressDescriptor(*desc);
}

int BalanceUtxo::binarySize() const {
	BinaryUtils::checkNotNull(this->utxoId);
	BinaryUtils::checkNotNull(this->addressDesc);

	int total = sizeof(uint8_t);

	total += this->utxoId->binarySize();

	total += this->addressDesc->binarySize();
	total += this->amount.binarySize();

	return total;
}

void BalanceUtxo::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->utxoId);
	BinaryUtils::checkNotNull(this->addressDesc);

	out->put(getType());

	this->utxoId->toBinary(out);

	this->addressDesc->toBinary(out);
	this->amount.toBinary(out);
}

void BalanceUtxo::fromBinary(ByteBuffer *in) {
	this->utxoId = UtxoId::fromBinary(in);

	this->addressDesc = AddressDescriptor::createFromBinary(in);

	BalanceUnit* unit = BalanceUnit::fromBinary(in); __STP(unit);
	this->amount = *unit;
}

IBlockObject* BalanceUtxo::copyData() const noexcept {
	BalanceUtxo* u = new BalanceUtxo(*this);

	return u;
}

void BalanceUtxo::build() {
	int capacity = this->addressDesc->binarySize();
	capacity += this->amount.binarySize();
	capacity += 32; // nonce

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(capacity, true); __STP(buff);

	this->addressDesc->toBinary(buff);
	this->amount.toBinary(buff);
	buff->put(this->nonce, 32);
	buff->position(0);

	ByteBuffer* sha = Sha256::sha256(buff, true); __STP(sha);

	delete this->utxoId;
	this->utxoId = new UtxoId((const char*)sha->array(), sha->limit());
}

void BalanceUtxo::discountFee(const BalanceUnit &feeRemain) {
	if(this->amount.compareTo(&feeRemain) < 0){
		throw new FeeShortageException(__FILE__, __LINE__);
	}

	this->amount -= feeRemain;
}

void BalanceUtxo::setAmount(const BalanceUnit amount) noexcept {
	this->amount = amount;
}

} /* namespace codablecash */
