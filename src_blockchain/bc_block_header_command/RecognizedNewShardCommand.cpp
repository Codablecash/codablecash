/*
 * RecognizedNewShardCommand.cpp
 *
 *  Created on: Jul 4, 2026
 *      Author: iizuka
 */

#include "bc_block_header_command/RecognizedNewShardCommand.h"

#include "bc_block/BlockHeader.h"

#include "bc_processor/CentralProcessor.h"

#include "bc_blockstore/CodablecashBlockchain.h"

#include "bc_status_cache/BlockchainController.h"
#include "bc_status_cache/BlockchainStatusCache.h"

#include "bc_status_cache_extend_shard/NotifyZoneExtendRequestedTransaction.h"

#include "bc_base/BinaryUtils.h"

#include "base/StackRelease.h"



namespace codablecash {

RecognizedNewShardCommand::RecognizedNewShardCommand(const RecognizedNewShardCommand &inst) : AbstractBlockHeaderCommand(inst) {
	this->trx = inst.trx != nullptr ? dynamic_cast<NotifyZoneExtendRequestedTransaction*>(inst.trx->copyData()) : nullptr;

}

RecognizedNewShardCommand::RecognizedNewShardCommand() : AbstractBlockHeaderCommand(AbstractBlockHeaderCommand::RECOGNIZED_SHARD_COMMAND) {
	this->trx = nullptr;
}

RecognizedNewShardCommand::~RecognizedNewShardCommand() {
	delete this->trx;
}

int RecognizedNewShardCommand::binarySize() const {
	BinaryUtils::checkNotNull(this->trx);

	int total = sizeof(uint16_t);

	total += this->trx->binarySize();

	return total;
}

void RecognizedNewShardCommand::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->trx);

	out->putShort(this->type);

	this->trx->toBinary(out);
}

void RecognizedNewShardCommand::fromBinary(ByteBuffer *in) {
	AbstractBlockchainTransaction* atrx = AbstractBlockchainTransaction::createFromBinary(in); __STP(atrx);
	NotifyZoneExtendRequestedTransaction* ntrx = dynamic_cast<NotifyZoneExtendRequestedTransaction*>(atrx);

	BinaryUtils::checkNotNull(ntrx);

	this->trx = ntrx; __STP_MV(atrx);
}

IBlockObject* RecognizedNewShardCommand::copyData() const noexcept {
	return new RecognizedNewShardCommand(*this);
}

void RecognizedNewShardCommand::onFinalize(const BlockHeader *header, BlockchainStatusCache *statusCache, CodablecashBlockchain *blockchain,
		ILockinManager *lockinManager, const CodablecashSystemParam *config) {
	uint16_t targetZone = header->getZone();
	uint16_t numRecognized = this->trx->getNewShardZone() + 1;

	statusCache->setNumRecognizedZones(targetZone, numRecognized);

}

void RecognizedNewShardCommand::setTransaction(const NotifyZoneExtendRequestedTransaction *trx) {
	delete this->trx;
	this->trx = dynamic_cast<NotifyZoneExtendRequestedTransaction*>(trx->copyData());
}

} /* namespace codablecash */
