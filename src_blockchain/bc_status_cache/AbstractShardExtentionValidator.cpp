/*
 * AbstractShardExtentionValidator.cpp
 *
 *  Created on: Jun 29, 2026
 *      Author: iizuka
 */

#include "bc_status_cache/AbstractShardExtentionValidator.h"

#include "bc_block_header_command/RecognizedNewShardCommand.h"

#include "bc_status_cache_extend_shard/NotifyZoneExtendRequestedTransaction.h"

#include "bc_trx/TransactionId.h"

#include "merkletree/MerkleCertificate.h"

namespace codablecash {

AbstractShardExtentionValidator::AbstractShardExtentionValidator(const AbstractShardExtentionValidator &inst) {
	this->statusCache = inst.statusCache;
}

AbstractShardExtentionValidator::AbstractShardExtentionValidator() {
	this->statusCache = nullptr;
}

AbstractShardExtentionValidator::~AbstractShardExtentionValidator() {
	this->statusCache = nullptr;
}

void AbstractShardExtentionValidator::setStatusCache(BlockchainStatusCache *stcache) noexcept {
	this->statusCache = stcache;
}

bool AbstractShardExtentionValidator::validateRecognizedNewShardCommand(RecognizedNewShardCommand *recognizeNewShardCommand, IStatusCacheContext *context, BlockchainController *ctrl) {
	NotifyZoneExtendRequestedTransaction* trx = recognizeNewShardCommand->getTransaction();
	const TransactionId* id = trx->getTransactionId();
	ByteBuffer* buff = id->getByteBuffer();

	MerkleCertificate* cert = recognizeNewShardCommand->getCertificate();
	bool res = cert->certificate(buff);

	return res;
}

} /* namespace codablecash */
