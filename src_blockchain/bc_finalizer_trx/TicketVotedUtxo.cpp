/*
 * TicketVotedUtxo.cpp
 *
 *  Created on: 2023/05/15
 *      Author: iizuka
 */

#include "bc_finalizer_trx/TicketVotedUtxo.h"

#include "bc_trx/UtxoId.h"

#include "bc_base/BinaryUtils.h"

#include "base/StackRelease.h"

#include "base_io/ByteBuffer.h"

#include "crypto/Sha256.h"

#include "bc_base/AddressDescriptor.h"
namespace codablecash {

TicketVotedUtxo::TicketVotedUtxo(const TicketVotedUtxo &inst) : AbstractUtxo(inst) {
	this->address = inst.address != nullptr ? dynamic_cast<AddressDescriptor*>(inst.address->copyData()) : nullptr;
	this->votedUtxoId = inst.votedUtxoId != nullptr ? dynamic_cast<UtxoId*>(inst.votedUtxoId->copyData()) : nullptr;
	this->amount = inst.amount;
}

TicketVotedUtxo::TicketVotedUtxo() : AbstractUtxo(), amount(BalanceUnit(0L)) {
	this->address = nullptr;
	this->votedUtxoId = nullptr;
}

TicketVotedUtxo::~TicketVotedUtxo() {
	delete this->address;
	delete this->votedUtxoId;
}

uint8_t TicketVotedUtxo::getType() const noexcept {
	return AbstractUtxo::TRX_UTXO_VOTED_TICKET;
}

void TicketVotedUtxo::setVotedUtxoId(const UtxoId *votedUtxoId) noexcept {
	delete this->votedUtxoId;
	this->votedUtxoId = dynamic_cast<UtxoId*>(votedUtxoId->copyData());
}

void TicketVotedUtxo::setAmount(BalanceUnit amount) noexcept {
	this->amount = amount;
}

int TicketVotedUtxo::binarySize() const {
	BinaryUtils::checkNotNull(this->address);
	BinaryUtils::checkNotNull(this->utxoId);
	BinaryUtils::checkNotNull(this->votedUtxoId);

	int total = sizeof(uint8_t);

	total += this->address->binarySize();
	total += this->utxoId->binarySize();
	total += this->votedUtxoId->binarySize();
	total += this->amount.binarySize();

	return total;
}

void TicketVotedUtxo::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->address);
	BinaryUtils::checkNotNull(this->utxoId);
	BinaryUtils::checkNotNull(this->votedUtxoId);

	out->put(getType());

	this->address->toBinary(out);
	this->utxoId->toBinary(out);
	this->votedUtxoId->toBinary(out);
	this->amount.toBinary(out);
}

void TicketVotedUtxo::fromBinary(ByteBuffer *in) {
	this->address = AddressDescriptor::createFromBinary(in);
	this->utxoId = UtxoId::fromBinary(in);
	this->votedUtxoId = UtxoId::fromBinary(in);

	BalanceUnit* unit = BalanceUnit::fromBinary(in); __STP(unit);
	this->amount = *unit;
}

IBlockObject* TicketVotedUtxo::copyData() const noexcept {
	return new TicketVotedUtxo(*this);
}

void TicketVotedUtxo::build() {
	int capacity = this->address->binarySize();
	capacity += this->votedUtxoId->binarySize();
	capacity += this->amount.binarySize();
	capacity += 32;

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(capacity, true); __STP(buff);

	this->address->toBinary(buff);
	this->votedUtxoId->toBinary(buff);
	this->amount.toBinary(buff);
	buff->put(this->nonce, 32);
	__ASSERT_POS(buff);

	buff->position(0);

	ByteBuffer* sha = Sha256::sha256(buff, true); __STP(sha);

	delete this->utxoId;
	this->utxoId = new UtxoId((const char*)sha->array(), sha->limit());
}

const AddressDescriptor* TicketVotedUtxo::getAddress() const noexcept {
	return this->address;
}

BalanceUnit TicketVotedUtxo::getAmount() const noexcept {
	return this->amount;
}

void TicketVotedUtxo::setAddress(const AddressDescriptor *desc) {
	delete this->address, this->address = nullptr;
	this->address = dynamic_cast<AddressDescriptor*>(desc->copyData());
}

} /* namespace codablecash */
