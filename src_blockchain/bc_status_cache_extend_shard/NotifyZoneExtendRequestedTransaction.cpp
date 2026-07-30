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
#include "base/ArrayList.h"

#include "crypto/Sha256.h"

#include "bc_block/BlockHeaderId.h"
#include "bc_block/BlockHeader.h"

#include "bc_status_cache_context/IStatusCacheContext.h"
#include "bc_status_cache_context/RemoteUtxoDetector.h"

#include "bc_status_cache/BlockchainStatusCache.h"

#include "bc_blockstore/CodablecashBlockchain.h"

#include "bc_block_header_command/BlockHeaderCommandId.h"
#include "bc_block_header_command/NewShardZoneCommand.h"


using alinous::ArrayList;


namespace codablecash {

NotifyZoneExtendRequestedTransaction::NotifyZoneExtendRequestedTransaction(const NotifyZoneExtendRequestedTransaction &inst)
		: AbstractInterChainCommunicationTansaction(inst) {
	this->zone = inst.zone;
	this->height = inst.height;
	this->headerId = inst.headerId != nullptr ? dynamic_cast<BlockHeaderId*>(inst.headerId->copyData()) : nullptr;

	this->utxoId = inst.utxoId != nullptr ? dynamic_cast<UtxoId*>(inst.utxoId->copyData()) : nullptr;
	this->newShardZone = inst.newShardZone;

	this->commandId = inst.commandId != nullptr ? dynamic_cast<BlockHeaderCommandId*>(inst.commandId->copyData()) : nullptr;
}

NotifyZoneExtendRequestedTransaction::NotifyZoneExtendRequestedTransaction() : AbstractInterChainCommunicationTansaction() {
	this->zone = 0;
	this->height = 0;
	this->headerId = nullptr;

	this->newShardZone = 0;

	this->utxoId = nullptr;
	this->commandId = nullptr;
}

NotifyZoneExtendRequestedTransaction::~NotifyZoneExtendRequestedTransaction() {
	delete this->headerId;
	delete this->commandId;
}

uint8_t NotifyZoneExtendRequestedTransaction::getType() const noexcept {
	return TRX_TYPE_ICC_ZONE_EXTEND_REQUESTED;
}

int NotifyZoneExtendRequestedTransaction::binarySize() const {
	BinaryUtils::checkNotNull(this->version);
	BinaryUtils::checkNotNull(this->timestamp);
	BinaryUtils::checkNotNull(this->headerId);
	BinaryUtils::checkNotNull(this->commandId);

	int total = sizeof(uint8_t);
	total += this->version->binarySize();
	total += this->timestamp->binarySize();

	total += sizeof(uint16_t); // zone
	total += sizeof(uint64_t); // height
	total += this->headerId->binarySize();

	total += sizeof(this->newShardZone);

	total += this->commandId->binarySize();

	return total;
}

void NotifyZoneExtendRequestedTransaction::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->version);
	BinaryUtils::checkNotNull(this->timestamp);
	BinaryUtils::checkNotNull(this->headerId);
	BinaryUtils::checkNotNull(this->commandId);

	out->put(getType());

	this->version->toBinary(out);
	this->timestamp->toBinary(out);

	out->putShort(this->zone);
	out->putLong(this->height);
	this->headerId->toBinary(out);

	out->putShort(this->newShardZone);

	this->commandId->toBinary(out);
}

void NotifyZoneExtendRequestedTransaction::fromBinary(ByteBuffer *in) {
	delete this->version;
	this->version = TransactionVersion::createFromBinary(in);

	delete this->timestamp;
	this->timestamp = SystemTimestamp::fromBinary(in);

	this->zone = in->getShort();
	this->height = in->getLong();
	this->headerId = BlockHeaderId::fromBinary(in);

	this->newShardZone = in->getShort();

	this->commandId = BlockHeaderCommandId::fromBinary(in);

	build();
}

void NotifyZoneExtendRequestedTransaction::build() {
	BinaryUtils::checkNotNull(this->version);
	BinaryUtils::checkNotNull(this->timestamp);
	BinaryUtils::checkNotNull(this->headerId);
	BinaryUtils::checkNotNull(this->commandId);

	{
		int capacity = sizeof(uint8_t) + this->version->binarySize() + this->timestamp->binarySize();
		capacity += sizeof(uint16_t) + sizeof(uint64_t) + this->headerId->binarySize() + sizeof(this->newShardZone)
				+ this->commandId->binarySize();

		ByteBuffer* buff = ByteBuffer::allocateWithEndian(capacity, true); __STP(buff);
		buff->put(getType());

		this->version->toBinary(buff);
		this->timestamp->toBinary(buff);

		buff->putShort(this->zone);
		buff->putLong(this->height);
		this->headerId->toBinary(buff);

		buff->putShort(this->newShardZone);

		this->commandId->toBinary(buff);

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

		delete this->utxoId, this->utxoId = nullptr;
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
	bool ret = false;

	CodablecashBlockchain* blockchain = context->getBlockChain();
	BlockchainStatusCache* statusCache = context->getBlockchainStatusCache();

	uint64_t finalizedHeight = statusCache->getFinalizedHeight(this->zone);

	{
		const BlockHeader* header = nullptr;

		ArrayList<BlockHeader>* list = blockchain->getBlockHeadersHeightAt(this->zone, this->height); __STP(list);
		list->setDeleteOnExit();

		int maxLoop = list->size();
		for(int i = 0; i != maxLoop; ++i){
			const BlockHeader* h = list->get(i);
			const BlockHeaderId* id = h->getId();
			if(this->headerId->equals(id)){
				header = h;
			}
		}

		if(header != nullptr){
			const AbstractBlockHeaderCommand* command = header->getHeaderCommand(this->commandId);
			const NewShardZoneCommand* newShardZoneCommand = dynamic_cast<const NewShardZoneCommand*>(command);

			// const BlockHeaderCommandId* newCommnadId = newShardZoneCommand->getCommandId();
			uint64_t headerHeight = header->getHeight();

			ret = newShardZoneCommand != nullptr && this->commandId->equals(newShardZoneCommand->getCommandId());
		}
	}

	return ret;
}

TrxValidationResult NotifyZoneExtendRequestedTransaction::validateFinal(const BlockHeader *header, MemPoolTransaction *memTrx, IStatusCacheContext *context) const {
	RemoteUtxoDetector* remoteDetector = context->getRemoteUtxoDetector();

	if(remoteDetector->isRemoteUtxoUsed(this->utxoId)){
		return TrxValidationResult::INVALID;
	}

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

void NotifyZoneExtendRequestedTransaction::setNewShardZone(uint16_t newShardZone) noexcept {
	this->newShardZone = newShardZone;
}

void NotifyZoneExtendRequestedTransaction::setCommandId(const BlockHeaderCommandId *commandId) {
	delete this->commandId;
	this->commandId = dynamic_cast<BlockHeaderCommandId*>(commandId->copyData());
}

} /* namespace codablecash */
