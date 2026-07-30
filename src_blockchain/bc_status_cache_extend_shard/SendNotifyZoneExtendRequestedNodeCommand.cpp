/*
 * SendNotifyZoneExtendRequestedNodeCommand.cpp
 *
 *  Created on: Jul 19, 2026
 *      Author: iizuka
 */

#include "bc_status_cache_extend_shard/SendNotifyZoneExtendRequestedNodeCommand.h"
#include "bc_status_cache_extend_shard/NotifyZoneExtendRequestedTransaction.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc_status_cache_context/IStatusCacheContext.h"

#include "bc_base/BinaryUtils.h"

#include "bc_trx/AbstractBlockchainTransaction.h"

#include "data_history_data/TransactionTransferData.h"

#include "base_timestamp/SystemTimestamp.h"

#include "base/StackRelease.h"

#include "bc/CodablecashNodeInstance.h"

#include "bc_p2p_processor/P2pRequestProcessor.h"

#include "bc_memorypool/MemoryPool.h"
#include "bc_memorypool/MemPoolTransaction.h"

#include "bc/ExceptionThrower.h"

#include "bc_p2p_cmd_node/InvalidTransactionException.h"

#include "bc_p2p/BlockchainNodeHandshake.h"
#include "bc_p2p/BlochchainP2pManager.h"

#include "command_queue_cmd/NodeTransactionAcceptionQueueCommand.h"

#include "bc_network/NodeIdentifierSource.h"
#include "bc_network/NodeIdentifier.h"

#include "bc_p2p_cmd_network/NodeNetworkInfo.h"

#include "pubsub_cmd/OkPubsubResponse.h"
namespace codablecash {

SendNotifyZoneExtendRequestedNodeCommand::SendNotifyZoneExtendRequestedNodeCommand(const SendNotifyZoneExtendRequestedNodeCommand &inst)
		: AbstractConsensusNodeCommand(inst) {
	this->data = inst.data != nullptr ? dynamic_cast<TransactionTransferData*>(inst.data->copyData()) : nullptr;
}

SendNotifyZoneExtendRequestedNodeCommand::SendNotifyZoneExtendRequestedNodeCommand() : AbstractConsensusNodeCommand(TYPE_CONSENSUS_SEND_NOTIFY_ZONE_EXTEND_REQUEST) {
	this->data = nullptr;
}

SendNotifyZoneExtendRequestedNodeCommand::~SendNotifyZoneExtendRequestedNodeCommand() {
	delete this->data;
}

int SendNotifyZoneExtendRequestedNodeCommand::binarySize() const {
	BinaryUtils::checkNotNull(this->data);

	int total = AbstractConsensusNodeCommand::binarySize();
	total += this->data->binarySize();

	return total;
}

void SendNotifyZoneExtendRequestedNodeCommand::toBinary(ByteBuffer *buff) const {
	BinaryUtils::checkNotNull(this->data);

	AbstractConsensusNodeCommand::toBinary(buff);
	this->data->toBinary(buff);
}

void SendNotifyZoneExtendRequestedNodeCommand::fromBinary(ByteBuffer *buff) {
	AbstractConsensusNodeCommand::fromBinary(buff);

	AbstractTransferedData* d = TransactionTransferData::createFromBinary(buff); __STP(d);
	this->data = dynamic_cast<TransactionTransferData*>(d);
	BinaryUtils::checkNotNull(this->data);

	__STP_MV(d);
}

ByteBuffer* SendNotifyZoneExtendRequestedNodeCommand::getSignBinary() const {
	BinaryUtils::checkNotNull(this->data);

	int total = this->data->binarySize();

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(total, true);
	this->data->toBinary(buff);

	return buff;
}

IBlockObject* SendNotifyZoneExtendRequestedNodeCommand::copyData() const noexcept {
	return new SendNotifyZoneExtendRequestedNodeCommand(*this);
}

SystemTimestamp SendNotifyZoneExtendRequestedNodeCommand::getFirstTimestamp() const {
	AbstractBlockchainTransaction* trx = this->data->getTransaction();
	const SystemTimestamp* tm = trx->getTimestamp();

	return *tm;
}

AbstractCommandResponse* SendNotifyZoneExtendRequestedNodeCommand::executeAsNode(BlockchainNodeHandshake *nodeHandShake, CodablecashNodeInstance *inst, bool suspend) const {
	P2pRequestProcessor* processor = inst->getP2pRequestProcessor();

	uint16_t zoneSelf = inst->getZoneSelf();
	{
		AbstractBlockchainTransaction* trx = this->data->getTransaction();
		NotifyZoneExtendRequestedTransaction* extendTrx = dynamic_cast<NotifyZoneExtendRequestedTransaction*>(trx);
		uint16_t newZone = extendTrx->getNewShardZone();
		if(newZone == zoneSelf){
			return new OkPubsubResponse();
		}
	}


	bool alreadyHas = processor->hasHistory(this->data);
	if(!alreadyHas){
		BlockchainController* ctrl = inst->getController();
		MemoryPool* memPool = inst->getMemoryPool();

		processor->addHistory(this->data);

		{
			bool res = processTransaction(zoneSelf, memPool, ctrl);
			ExceptionThrower<InvalidTransactionException>::throwExceptionIfCondition(!res, L"Invalid Transaction was sent.", __FILE__, __LINE__);
		}

		// put on queue
		{
			const PubSubId* pubsubId = nodeHandShake->getPubsubId();

			NodeTransactionAcceptionQueueCommand cmd;
			cmd.setData(this->data);

			processor->putQueue(pubsubId, &cmd);
		}

		// [multishard] review it
		if(!suspend){
			// bload cast
			BlochchainP2pManager* manager = inst->getBlochchainP2pManager();
			{
				SendNotifyZoneExtendRequestedNodeCommand command(*this);

				NodeIdentifierSource* nwkey = processor->getNetworkKey();
				{
					uint16_t zone = inst->getZoneSelf();
					NodeIdentifier nodeId = nwkey->toNodeIdentifier();

					int protocol = manager->getProtocol();
					const UnicodeString* host = manager->getHost();
					uint32_t port = manager->getPort();

					command.addHistory(zone, &nodeId, protocol, host, port);
				}

				command.sign(nwkey);

				// make list to exclude broad cast
				ArrayList<NodeIdentifier> list;
				list.setDeleteOnExit(true);
				command.makeHistoryExcludeList(&list);

				// broadcast
				manager->bloadCastHighPriorityAllZones(&list, &command, processor);
			}
		}
	}

	return new OkPubsubResponse();
}

bool SendNotifyZoneExtendRequestedNodeCommand::processTransaction(uint16_t zoneSelf, MemoryPool *memPool, BlockchainController *ctrl) const {
	AbstractBlockchainTransaction* trx = this->data->getTransaction();

	MemPoolTransaction* memTrx = memPool->begin(); __STP(memTrx);

	IStatusCacheContext* context = ctrl->getStatusCacheContext(zoneSelf); __STP(context);

	return trx->validateOnAccept(memTrx, context);
}

void SendNotifyZoneExtendRequestedNodeCommand::setTransactionTransferData(const TransactionTransferData *data) noexcept {
	delete this->data;
	this->data = dynamic_cast<TransactionTransferData*>(data->copyData());
}

} /* namespace codablecash */
