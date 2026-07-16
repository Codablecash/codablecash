/*
 * NotifyShardExtendRequestCommandMessage.cpp
 *
 *  Created on: Jul 5, 2026
 *      Author: iizuka
 */

#include "bc_status_cache_extend_shard/NotifyShardExtendRequestCommandMessage.h"

#include "bc_p2p_processor/P2pRequestProcessor.h"

#include "bc_p2p/BlochchainP2pManager.h"

#include "bc_processor/CentralProcessor.h"

#include "bc_block/BlockHeaderId.h"

namespace codablecash {

NotifyShardExtendRequestCommandMessage::NotifyShardExtendRequestCommandMessage() {
	this->newShardZone = 0;
	this->requestingZone = 0;
	this->height = 0;
	this->headerId = nullptr;
}

NotifyShardExtendRequestCommandMessage::~NotifyShardExtendRequestCommandMessage() {
	delete this->headerId;
}

void NotifyShardExtendRequestCommandMessage::process(CentralProcessor *processor) {
	P2pRequestProcessor* p2pRequestProcessor = processor->getP2pRequestProcessor();
	BlochchainP2pManager* p2pManager = processor->getBlochchainP2pManager();



}

void NotifyShardExtendRequestCommandMessage::setNewShardZone(uint16_t newShardZone) noexcept {
	this->newShardZone = newShardZone;
}

void NotifyShardExtendRequestCommandMessage::setRequestingZone(uint16_t requestingZone) noexcept {
	this->requestingZone = requestingZone;
}

void NotifyShardExtendRequestCommandMessage::setHeaderInfo(uint64_t height, const BlockHeaderId *headerId) noexcept {
	this->height = height;

	delete this->headerId;
	this->headerId = dynamic_cast<BlockHeaderId*>(headerId->copyData());
}

} /* namespace codablecash */
