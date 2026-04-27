/*
 * SendVoteTransactionNodeCommand.cpp
 *
 *  Created on: 2024/07/24
 *      Author: iizuka
 */

#include "bc_p2p_cmd_node_consensus/SendVoteTransactionNodeCommand.h"

#include "data_history_data/TransactionTransferData.h"

#include "base_timestamp/SystemTimestamp.h"

#include "bc_trx/AbstractBlockchainTransaction.h"

#include "bc_base/BinaryUtils.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "bc/CodablecashSystemParam.h"
#include "bc/CodablecashNodeInstance.h"
#include "bc/ExceptionThrower.h"

#include "pubsub_cmd/OkPubsubResponse.h"

#include "bc_p2p/BlockchainNodeHandshake.h"
#include "bc_p2p/BlochchainP2pManager.h"

#include "bc_p2p_cmd_node/InvalidTransactionException.h"

#include "bc_p2p_processor/P2pRequestProcessor.h"

#include "bc_memorypool/MemoryPool.h"
#include "bc_memorypool/MemPoolTransaction.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc_status_cache_context/IStatusCacheContext.h"


#include "command_queue_cmd/NodeTransactionAcceptionQueueCommand.h"

#include "bc_network/NodeIdentifierSource.h"
#include "bc_network/NodeIdentifier.h"

#include "bc_p2p_cmd_client_notify/ClientNotifyNewTransactionCommand.h"

#include "bc_p2p_cmd_network/NodeNetworkInfo.h"


namespace codablecash {

SendVoteTransactionNodeCommand::SendVoteTransactionNodeCommand(const SendVoteTransactionNodeCommand &inst)
		: AbstractConsensusNodeCommand(inst) {
	this->data = inst.data != nullptr ? dynamic_cast<TransactionTransferData*>(inst.data->copyData()) : nullptr;
}

SendVoteTransactionNodeCommand::SendVoteTransactionNodeCommand()
	: AbstractConsensusNodeCommand(AbstractConsensusNodeCommand::TYPE_CONSENSUS_SEND_VOTE_TRANSACTION) {
	this->data = nullptr;
}

SendVoteTransactionNodeCommand::~SendVoteTransactionNodeCommand() {
	delete this->data;
}

int SendVoteTransactionNodeCommand::binarySize() const {
	BinaryUtils::checkNotNull(this->data);

	int total = AbstractConsensusNodeCommand::binarySize();
	total += this->data->binarySize();

	return total;
}

void SendVoteTransactionNodeCommand::toBinary(ByteBuffer *buff) const {
	BinaryUtils::checkNotNull(this->data);

	AbstractConsensusNodeCommand::toBinary(buff);
	this->data->toBinary(buff);
}

ByteBuffer* SendVoteTransactionNodeCommand::getSignBinary() const {
	BinaryUtils::checkNotNull(this->data);

	int total = this->data->binarySize();

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(total, true);
	this->data->toBinary(buff);

	return buff;
}

void SendVoteTransactionNodeCommand::fromBinary(ByteBuffer *buff) {
	AbstractConsensusNodeCommand::fromBinary(buff);

	AbstractTransferedData* d = TransactionTransferData::createFromBinary(buff); __STP(d);
	this->data = dynamic_cast<TransactionTransferData*>(d);
	BinaryUtils::checkNotNull(this->data);

	__STP_MV(d);
}

AbstractCommandResponse* SendVoteTransactionNodeCommand::executeAsNode(BlockchainNodeHandshake *nodeHandShake, CodablecashNodeInstance *inst, bool suspend) const {
	P2pRequestProcessor* processor = inst->getP2pRequestProcessor();

	uint16_t zoneSelf = inst->getZoneSelf();
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

		if(!suspend){
			// bload cast
			BlochchainP2pManager* manager = inst->getBlochchainP2pManager();
			{
				SendVoteTransactionNodeCommand command(*this);
				command.setTransactionTransferData(this->data);

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
				{
					const ArrayList<NodeNetworkInfo>* history = command.getHistory();
					int maxLoop = history->size();
					for(int i = 0; i != maxLoop; ++i){
						NodeNetworkInfo* his = history->get(i);
						const NodeIdentifier* nodeId = his->getNodeIdentifier();

						NodeIdentifier* newId = dynamic_cast<NodeIdentifier*>(nodeId->copyData());
						list.addElement(newId);
					}
				}
				// broadcast
				manager->bloadCastHighPriorityAllZones(&list, &command, processor);
			}

			{
				ClientNotifyNewTransactionCommand command;
				command.setTransactionTransferData(this->data);
				command.sign(inst->getNetworkKey());

				manager->broadCastToClients(&command, processor);
			}
		}
	}

	return new OkPubsubResponse();
}


bool SendVoteTransactionNodeCommand::processTransaction(uint16_t zoneSelf, MemoryPool *memPool, BlockchainController *ctrl) const {
	AbstractBlockchainTransaction* trx = this->data->getTransaction();

	MemPoolTransaction* memTrx = memPool->begin(); __STP(memTrx);

	IStatusCacheContext* context = ctrl->getStatusCacheContext(zoneSelf); __STP(context);

	return trx->validateOnAccept(memTrx, context);
}

IBlockObject* SendVoteTransactionNodeCommand::copyData() const noexcept {
	return new SendVoteTransactionNodeCommand(*this);
}

SystemTimestamp SendVoteTransactionNodeCommand::getFirstTimestamp() const {
	AbstractBlockchainTransaction* trx = this->data->getTransaction();
	const SystemTimestamp* tm = trx->getTimestamp();

	return *tm;
}

void SendVoteTransactionNodeCommand::setTransactionTransferData(const TransactionTransferData *data) noexcept {
	delete this->data;
	this->data = dynamic_cast<TransactionTransferData*>(data->copyData());
}

} /* namespace codablecash */
