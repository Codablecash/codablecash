/*
 * TransferedMinedReportCommandMessage.cpp
 *
 *  Created on: 2023/11/23
 *      Author: iizuka
 */

#include "bc_processor_cmd/TransferedMinedReportCommandMessage.h"

#include "data_history_data/BlockHeaderTransferData.h"

#include "bc_network/NodeIdentifier.h"

#include "bc_processor/CentralProcessor.h"

#include "bc_p2p_processor/P2pRequestProcessor.h"

#include "bc_p2p/BlochchainP2pManager.h"
#include "bc_p2p/BlockchainNodeHandshake.h"
#include "bc_p2p/BlockchainNodeHandshakeException.h"
#include "bc_p2p/StackHandshakeReleaser.h"

#include "bc_blockstore/CodablecashBlockchain.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc_block/BlockHeader.h"
#include "bc_block/Block.h"
#include "bc_block_body/BlockBody.h"

#include "bc_p2p_cmd_node/DownloadOmittedBlockBodyNodeCommand.h"
#include "bc_p2p_cmd_node/DownloadOmittedBlockBodyNodeCommandResponse.h"
#include "bc_p2p_cmd_node/DownloadTransactionsNodeCommand.h"
#include "bc_p2p_cmd_node/DownloadTransactionsNodeCommandResponse.h"

#include "bc_p2p_cmd_node_consensus/ReportMinedBlockNodeCommand.h"

#include "bc/ExceptionThrower.h"
#include "bc/ISystemLogger.h"

#include "base/StackRelease.h"

#include "bc_block_validator/BlockValidator.h"

#include "bc_memorypool/MemoryPool.h"

#include "bc_trx/AbstractBlockchainTransaction.h"

#include "bc_network/NodeIdentifierSource.h"

#include "pubsub/P2pHandshake.h"

#include "base_thread/StackUnlocker.h"

#include "base_thread/SysMutex.h"

#include "bc_p2p_cmd_client_notify/ClientNotifyBlockMinedCommand.h"

#include "bc_block_body/OmittedBlockBodyFixer.h"

namespace codablecash {

TransferedMinedReportCommandMessage::TransferedMinedReportCommandMessage() {
	this->data = nullptr;
	this->nodeId = nullptr;
}

TransferedMinedReportCommandMessage::~TransferedMinedReportCommandMessage() {
	delete this->data;
	delete this->nodeId;
}

void TransferedMinedReportCommandMessage::setData(const BlockHeaderTransferData *data) {
	delete this->data;
	this->data = dynamic_cast<BlockHeaderTransferData*>(data->copyData());
}

void TransferedMinedReportCommandMessage::setNodeId(const NodeIdentifier *nodeId) {
	delete this->nodeId;
	this->nodeId = dynamic_cast<NodeIdentifier*>(nodeId->copyData());
}

/**
 * After getting ReportMinedBlockNodeCommand, from miners or other nodes, this process is driven.
 * @param processor
 */
void TransferedMinedReportCommandMessage::process(CentralProcessor *processor) {
	P2pRequestProcessor* requestProcessor = processor->getP2pRequestProcessor();
	BlochchainP2pManager* p2pManager = processor->getBlochchainP2pManager();
	ISystemLogger* logger = processor->getLogger();
	MemoryPool* memPool = processor->getMemoryPool();
	CodablecashSystemParam* config = processor->getCodablecashSystemParam();

	BlockchainController* ctrl = processor->getCtrl();
	uint16_t zoneSelf = ctrl->getZoneSelf();

	const BlockHeader* header = this->data->getHeader();
	uint16_t zone = header->getZone();

	// [consensus]check if the block time is after PoSLimit
	{
		uint64_t height = header->getHeight();
		ctrl->getPosVoteLimit(zone, height);
	}


	bool dataAdded = false;
	if(zoneSelf == zone){
		NodeIdentifierSource* networkKey = requestProcessor->getNetworkKey();
		dataAdded = importBlock(memPool, ctrl, p2pManager, networkKey, logger, config);
	}else{
		dataAdded = importHeader(ctrl);
	}

	if(dataAdded){
		{
			NodeIdentifierSource* networkKey = requestProcessor->getNetworkKey();
			ReportMinedBlockNodeCommand command;
			command.setData(this->data);

			command.sign(networkKey);

			p2pManager->broadCastAllZones(this->nodeId, &command, requestProcessor);
		}

		// client notify
		{
			ClientNotifyBlockMinedCommand command;
			command.setBlockHeaderTransferData(this->data);

			// sign
			NodeIdentifierSource* nwkey = requestProcessor->getNetworkKey();
			command.sign(nwkey);

			p2pManager->broadCastToClients(&command, requestProcessor);
		}
	}
}

bool TransferedMinedReportCommandMessage::importHeader(BlockchainController *ctrl) {
	const BlockHeader* header = this->data->getHeader();

	return ctrl->addBlockHeader(header);
}

bool TransferedMinedReportCommandMessage::importBlock(MemoryPool* memPool, BlockchainController *ctrl, BlochchainP2pManager *p2pManager
		, NodeIdentifierSource* networkKey ,ISystemLogger* logger, CodablecashSystemParam* config) {
	const BlockHeader* header = this->data->getHeader();
	uint16_t zone = header->getZone();
	uint64_t height = header->getHeight();
	const BlockHeaderId* headerId = header->getId();


	// check if already block exists
	{
		bool bl = ctrl->hasHeaderId(zone, height, headerId);
		if(bl){
			return false;
		}
	}

	DownloadOmittedBlockBodyNodeCommand command;
	command.setZone(header->getZone());
	command.setHeight(height);
	command.setHeaderId(headerId);

	// sign
	command.sign(networkKey);

	// download entire block command
	DownloadOmittedBlockBodyNodeCommandResponse* omittedBodyResponse = nullptr;
	{
		BlockchainNodeHandshake* handshake = p2pManager->getNodeHandshakeByNodeId(this->nodeId);
		ExceptionThrower<BlockchainNodeHandshakeException>::throwExceptionIfCondition(handshake == nullptr, L"Node connection has alrealy closed.", __FILE__, __LINE__);

		StackHandshakeReleaser __releaser(handshake);

		AbstractCommandResponse* response = handshake->sendCommnad(&command); __STP(response);
		omittedBodyResponse = dynamic_cast<DownloadOmittedBlockBodyNodeCommandResponse*>(response);

		if(response!= nullptr){
			UnicodeString* message = response->toString(); __STP(message);
			logger->debugLog(ISystemLogger::DEBUG_NODE_TRANSFER_RESPONSE, message, __FILE__, __LINE__);
		}

		ExceptionThrower<BlockchainNodeHandshakeException>::throwExceptionIfCondition(omittedBodyResponse == nullptr, L"Respose error.", __FILE__, __LINE__);
		__STP_MV(response);
	}
	__STP(omittedBodyResponse);

	// download entire block command
	const OmittedBlockBody* obody = omittedBodyResponse->getBody();

	// ommit to body
	OmittedBlockBodyFixer fixer(this->nodeId, this->processor->getMemoryPool(), p2pManager, headerId, height, networkKey);
	BlockBody* body = obody->toBlockBody(height, headerId, &fixer, logger);	__STP(body);

	Block block(dynamic_cast<BlockHeader*>(header->copyData()), __STP_MV(body));

	BlockValidator validator(&block, config, memPool, ctrl);
	validator.validate();

	return ctrl->addBlock(&block);
}

} /* namespace codablecash */
