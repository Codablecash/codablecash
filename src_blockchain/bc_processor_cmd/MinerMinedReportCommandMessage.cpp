/*
 * MinedReportCommandMessage.cpp
 *
 *  Created on: 2023/04/04
 *      Author: iizuka
 */

#include "bc_block/Block.h"

#include "bc_processor/CentralProcessor.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc_p2p_processor/P2pRequestProcessor.h"

#include "bc_processor_cmd/MinerMinedReportCommandMessage.h"

#include "data_history_data/BlockHeaderTransferData.h"

#include "base/StackRelease.h"

#include "bc_network/NodeIdentifierSource.h"
#include "bc_network/NodeIdentifier.h"

#include "bc_p2p/BlochchainP2pManager.h"

#include "bc_p2p_cmd_node_consensus/ReportMinedBlockNodeCommand.h"
#include "bc_p2p_cmd_node_consensus/ReportNonceCalculatedNodeCommand.h"

#include "bc_p2p_cmd_client_notify/ClientNotifyBlockMinedCommand.h"


namespace codablecash {

MinerMinedReportCommandMessage::MinerMinedReportCommandMessage(const Block* block) {
	this->block = new Block(*block);
}

MinerMinedReportCommandMessage::~MinerMinedReportCommandMessage() {
	delete this->block;
}

/**
 * onBlockMined the miner process of this node mined a block
 */
void MinerMinedReportCommandMessage::process(CentralProcessor *processor) {
	BlockchainController* ctrl = processor->getCtrl();
	ctrl->addBlock(this->block);

	// Pos Voting Request
	{
		uint16_t zone = this->block->getZone();
		uint16_t height = this->block->getHeight();

		ctrl->requestPosVote(zone, height);
	}


	// [Network] bload cast
	P2pRequestProcessor* p2pRequestProcessor = processor->getP2pRequestProcessor();
	BlochchainP2pManager* p2pManager = processor->getBlochchainP2pManager();


	BlockHeaderTransferData* data = BlockHeaderTransferData::createFromBlock(this->block); __STP(data);

	// register header for PoS limit
	{
		const BlockHeader* header = data->getHeader();
		uint16_t zone = header->getZone();
		CodablecashSystemParam* config = processor->getCodablecashSystemParam();

		ctrl->registerBlockHeader4Limit(zone, header, config);
	}


	if(p2pRequestProcessor != nullptr && p2pManager != nullptr){
		// notify PoS ticket time limits
		{
			ReportNonceCalculatedNodeCommand cmd;
			const BlockHeader* header = data->getHeader();
			cmd.setHeader(header);

			// sign
			NodeIdentifierSource* nwkey = p2pRequestProcessor->getNetworkKey();
			cmd.sign(nwkey);

			ArrayList<NodeIdentifier> list;
			p2pManager->bloadCastHighPriorityAllZones(&list, &cmd, p2pRequestProcessor);
		}

		// add to history
		p2pRequestProcessor->addHistory(data);

		// notify headers
		{
			ReportMinedBlockNodeCommand command;
			command.setData(data);

			// sign
			NodeIdentifierSource* nwkey = p2pRequestProcessor->getNetworkKey();
			command.sign(nwkey);

			p2pManager->broadCastAllZones(nullptr, &command, p2pRequestProcessor);
		}

		// client notify
		{
			ClientNotifyBlockMinedCommand command;
			command.setBlockHeaderTransferData(data);

			// sign
			NodeIdentifierSource* nwkey = p2pRequestProcessor->getNetworkKey();
			command.sign(nwkey);

			p2pManager->broadCastToClients(&command, p2pRequestProcessor);
		}
	}
}

} /* namespace codablecash */
