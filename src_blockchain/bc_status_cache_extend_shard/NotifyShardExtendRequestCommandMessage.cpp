/*
 * NotifyShardExtendRequestCommandMessage.cpp
 *
 *  Created on: Jul 5, 2026
 *      Author: iizuka
 */

#include "bc_status_cache_extend_shard/NotifyShardExtendRequestCommandMessage.h"
#include "bc_status_cache_extend_shard/NotifyZoneExtendRequestedTransaction.h"
#include "bc_status_cache_extend_shard/SendNotifyZoneExtendRequestedNodeCommand.h"

#include "bc_p2p_processor/P2pRequestProcessor.h"

#include "bc_processor/CentralProcessor.h"

#include "bc_p2p/BlochchainP2pManager.h"

#include "bc_block/BlockHeaderId.h"

#include "base/StackRelease.h"

#include "bc_memorypool/MemoryPool.h"
#include "bc_memorypool/MemPoolTransaction.h"

#include "data_history_data/TransactionTransferData.h"

#include "bc_network/NodeIdentifier.h"

#include "bc_block_header_command/BlockHeaderCommandId.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc_status_cache_context/IStatusCacheContext.h"


namespace codablecash {

NotifyShardExtendRequestCommandMessage::NotifyShardExtendRequestCommandMessage() {
	this->commandId = nullptr;
	this->newShardZone = 0;
	this->requestingZone = 0;
	this->height = 0;
	this->headerId = nullptr;
}

NotifyShardExtendRequestCommandMessage::~NotifyShardExtendRequestCommandMessage() {
	delete this->commandId;
	delete this->headerId;
}

void NotifyShardExtendRequestCommandMessage::process(CentralProcessor *processor) {
	P2pRequestProcessor* p2pRequestProcessor = processor->getP2pRequestProcessor();
	BlochchainP2pManager* p2pManager = processor->getBlochchainP2pManager();

	// [multishard] broadcast transaction
	NotifyZoneExtendRequestedTransaction* trx = new NotifyZoneExtendRequestedTransaction(); __STP(trx);
	trx->setHeaderInfo(this->requestingZone, this->height, this->headerId);
	trx->setNewShardZone(this->newShardZone);
	trx->setCommandId(this->commandId);
	trx->build();

	// add self memory pool
	{
		BlockchainController* ctrl = processor->getCtrl();
		uint16_t zoneSelf = ctrl->getZoneSelf();
		MemoryPool* mempool = processor->getMemoryPool();

		bool result = false;
		{
			MemPoolTransaction* memTrx = mempool->begin(); __STP(memTrx);
			IStatusCacheContext* context = ctrl->getBlankStatusCacheContext(zoneSelf); __STP(context);

			result = trx->validateOnAccept(memTrx, context);
		}

		if(result == true){
			mempool->putTransaction(trx);
		}
	}

	if(p2pRequestProcessor != nullptr && p2pManager != nullptr){
		SendNotifyZoneExtendRequestedNodeCommand command;
		TransactionTransferData data;
		data.setTransaction(trx);
		command.setTransactionTransferData(&data);

		NodeIdentifierSource* networkKey = p2pRequestProcessor->getNetworkKey();
		command.sign(networkKey);

		// add into the history
		p2pRequestProcessor->addHistory(&data);

#ifdef __DEBUG__
		{
			bool bl = p2pRequestProcessor->hasHistory(&data);
			assert(bl == true);
		}
#endif


		// broad cast
		ArrayList<NodeIdentifier> list;
		p2pManager->bloadCastHighPriorityAllZones(&list, &command, p2pRequestProcessor);
	}
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

void NotifyShardExtendRequestCommandMessage::setCommandId(const BlockHeaderCommandId *commandId) {
	delete this->commandId;
	this->commandId = dynamic_cast<BlockHeaderCommandId*>(commandId->copyData());
}

} /* namespace codablecash */
