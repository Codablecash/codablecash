/*
 * NotifyZoneExtendRequestedTransaction.cpp
 *
 *  Created on: Jun 29, 2026
 *      Author: iizuka
 */

#include "bc_status_cache_extend_shard/NotifyZoneExtendRequestedTransaction.h"

#include "bc_base/BalanceUnit.h"
#include "bc_base/BinaryUtils.h"

#include "bc_trx/TransactionVersion.h"
#include "bc_trx/TransactionId.h"
#include "bc_trx/UtxoId.h"

#include "base_timestamp/SystemTimestamp.h"

#include "base/StackRelease.h"

#include "crypto/Sha256.h"

#include "bc_block/BlockHeaderId.h"


namespace codablecash {

NotifyZoneExtendRequestedTransaction::NotifyZoneExtendRequestedTransaction(const NotifyZoneExtendRequestedTransaction &inst)
		: AbstractInterChainCommunicationTansaction(inst) {
	this->zone = inst.zone;
	this->height = inst.height;
	this->headerId = inst.headerId != nullptr ? dynamic_cast<BlockHeaderId*>(inst.headerId->copyData()) : nullptr;

	this->utxoId = inst.utxoId != nullptr ? dynamic_cast<UtxoId*>(inst.utxoId->copyData()) : nullptr;
}

NotifyZoneExtendRequestedTransaction::NotifyZoneExtendRequestedTransaction() : AbstractInterChainCommunicationTansaction() {
	this->zone = 0;
	this->height = 0;
	this->headerId = nullptr;

	this->utxoId = nullptr;
}

NotifyZoneExtendRequestedTransaction::~NotifyZoneExtendRequestedTransaction() {
	delete this->headerId;
}

uint8_t NotifyZoneExtendRequestedTransaction::getType() const noexcept {
	return TRX_TYPE_ICC_ZONE_EXTEND_REQUESTED;
}

int NotifyZoneExtendRequestedTransaction::binarySize() const {
	BinaryUtils::checkNotNull(this->version);
	BinaryUtils::checkNotNull(this->timestamp);
	BinaryUtils::checkNotNull(this->headerId);

	int total = sizeof(uint8_t);
	total += this->version->binarySize();
	total += this->timestamp->binarySize();

	total += sizeof(uint16_t); // zone
	total += sizeof(uint64_t); // height
	total += this->headerId->binarySize();

	return total;
}

void NotifyZoneExtendRequestedTransaction::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->version);
	BinaryUtils::checkNotNull(this->timestamp);
	BinaryUtils::checkNotNull(this->headerId);

	out->put(getType());

	this->version->toBinary(out);
	this->timestamp->toBinary(out);

	out->putShort(this->zone);
	out->putLong(this->height);
	this->headerId->toBinary(out);
}

void NotifyZoneExtendRequestedTransaction::fromBinary(ByteBuffer *in) {
	delete this->version;
	this->version = TransactionVersion::createFromBinary(in);

	delete this->timestamp;
	this->timestamp = SystemTimestamp::fromBinary(in);

	this->zone = in->getShort();
	this->height = in->getLong();
	this->headerId = BlockHeaderId::fromBinary(in);
}

void NotifyZoneExtendRequestedTransaction::build() {
	BinaryUtils::checkNotNull(this->version);
	BinaryUtils::checkNotNull(this->timestamp);
	BinaryUtils::checkNotNull(this->headerId);

	{
		int capacity = sizeof(uint8_t) + this->version->binarySize() + this->timestamp->binarySize();
		capacity += sizeof(uint16_t) + sizeof(uint64_t) + this->headerId->binarySize();

		ByteBuffer* buff = ByteBuffer::allocateWithEndian(capacity, true); __STP(buff);
		buff->put(getType());

		this->version->toBinary(buff);
		this->timestamp->toBinary(buff);

		buff->putShort(this->zone);
		buff->putLong(this->height);
		this->headerId->toBinary(buff);

		buff->position(0);
		ByteBuffer* sha = Sha256::sha256(buff, true); __STP(sha);

		delete this->trxId;
		this->trxId = new TransactionId((const char*)sha->array(), sha->limit());
	}

	{
		int capacity =  sizeof(uint16_t) + sizeof(uint64_t) + this->headerId->binarySize();

		ByteBuffer* buff = ByteBuffer::allocateWithEndian(capacity, true); __STP(buff);
		buff->putShort(this->zone);
		buff->putLong(this->height);
		this->headerId->toBinary(buff);

		buff->position(0);

		ByteBuffer* sha = Sha256::sha256(buff, true); __STP(sha);

		delete this->utxoId;
		this->utxoId = new UtxoId((const char*)sha->array(), sha->limit());
	}
}

IBlockObject* NotifyZoneExtendRequestedTransaction::copyData() const noexcept {
	return new NotifyZoneExtendRequestedTransaction(*this);
}

BalanceUnit NotifyZoneExtendRequestedTransaction::getFee() const noexcept {
	return BalanceUnit(0);
}

BalanceUnit NotifyZoneExtendRequestedTransaction::getFeeRate() const noexcept {
	return BalanceUnit(0);
}

int NotifyZoneExtendRequestedTransaction::getUtxoSize() const noexcept {
	return 0;
}

AbstractUtxo* NotifyZoneExtendRequestedTransaction::getUtxo(int i) const noexcept {
	return nullptr;
}

int NotifyZoneExtendRequestedTransaction::getUtxoReferenceSize() const noexcept {
	return 0;
}

AbstractUtxoReference* NotifyZoneExtendRequestedTransaction::getUtxoReference(int i) const noexcept {
	return nullptr;
}

bool NotifyZoneExtendRequestedTransaction::validateOnAccept(MemPoolTransaction *memTrx, IStatusCacheContext *context) const {
	// FIXME[multishard] validate;

	return true;
}

TrxValidationResult NotifyZoneExtendRequestedTransaction::validateFinal(const BlockHeader *header, MemPoolTransaction *memTrx, IStatusCacheContext *context) const {
	return TrxValidationResult::OK;
}

bool NotifyZoneExtendRequestedTransaction::checkFilter(const ArrayList<BloomFilter1024> *filtersList) const {
	return false;
}

void NotifyZoneExtendRequestedTransaction::setHeaderInfo(uint16_t zone, uint64_t height, const BlockHeaderId *headerId) {
	this->zone = zone;
	this->height = height;

	delete this->headerId;
	this->headerId = dynamic_cast<BlockHeaderId*>(headerId->copyData());
}

} /* namespace codablecash */
