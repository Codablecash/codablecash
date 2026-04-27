/*
 * ReportMinedBlockNodeCommand.cpp
 *
 *  Created on: 2023/11/21
 *      Author: iizuka
 */



#include "bc_p2p_cmd_node_consensus/ReportMinedBlockNodeCommand.h"

#include "data_history_data/BlockHeaderTransferData.h"

#include "bc_base/BinaryUtils.h"

#include "base_io/ByteBuffer.h"

#include "base/StackRelease.h"

#include "bc/CodablecashNodeInstance.h"
#include "bc/ExceptionThrower.h"

#include "bc_p2p_processor/P2pRequestProcessor.h"

#include "pubsub_cmd/OkPubsubResponse.h"

#include "bc_processor_cmd/TransferedMinedReportCommandMessage.h"

#include "bc_processor/CentralProcessor.h"

#include "bc_p2p/BlockchainNodeHandshake.h"
#include "bc_p2p/BlochchainP2pManager.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc_block_validator/BlockValidationException.h"

#include "base_timestamp/SystemTimestamp.h"

namespace codablecash {

ReportMinedBlockNodeCommand::ReportMinedBlockNodeCommand(const ReportMinedBlockNodeCommand &inst) : AbstractNodeCommand(inst) {
	this->data = inst.data != nullptr ? dynamic_cast<BlockHeaderTransferData*>(inst.data->copyData()) : nullptr;
}

ReportMinedBlockNodeCommand::ReportMinedBlockNodeCommand() : AbstractNodeCommand(TYPE_NODE_REPORT_MINED_BLOCK) {
	this->data = nullptr;
}

ReportMinedBlockNodeCommand::~ReportMinedBlockNodeCommand() {
	delete this->data;
}

int ReportMinedBlockNodeCommand::binarySize() const {
	BinaryUtils::checkNotNull(this->data);

	int total = AbstractNodeCommand::binarySize();

	total += this->data->binarySize();

	return total;
}

void ReportMinedBlockNodeCommand::toBinary(ByteBuffer *buff) const {
	BinaryUtils::checkNotNull(this->data);

	AbstractNodeCommand::toBinary(buff);
	this->data->toBinary(buff);
}

ByteBuffer* ReportMinedBlockNodeCommand::getSignBinary() const {
	BinaryUtils::checkNotNull(this->data);

	int total = this->data->binarySize();

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(total, true);
	this->data->toBinary(buff);

	return buff;
}

void ReportMinedBlockNodeCommand::fromBinary(ByteBuffer *buff) {
	AbstractNodeCommand::fromBinary(buff);

	AbstractTransferedData* d = AbstractTransferedData::createFromBinary(buff); __STP(d);
	this->data = dynamic_cast<BlockHeaderTransferData*>(d);
	BinaryUtils::checkNotNull(this->data);

	__STP_MV(d);
}

IBlockObject* ReportMinedBlockNodeCommand::copyData() const noexcept {
	return new ReportMinedBlockNodeCommand(*this);
}

/**
 * Block transfered from mining node
 */
AbstractCommandResponse* ReportMinedBlockNodeCommand::executeAsNode(BlockchainNodeHandshake *nodeHandShake, CodablecashNodeInstance *inst, bool suspend) const {
	P2pRequestProcessor* processor = inst->getP2pRequestProcessor();
	BlockchainController* ctrl = inst->getController();

	bool alreadyReceived = processor->hasHistory(this->data);
	if(!alreadyReceived){
		const BlockHeader* header = this->data->getHeader();
		uint16_t zone = header->getZone();
		uint64_t height = header->getHeight();
		const BlockHeaderId* headerId = header->getLastHeaderId();


		// [consensus] validate Pos and header time
		{
			uint16_t zone = header->getZone();
			uint64_t lastHeight = header->getHeight() - 1;
			SystemTimestamp* limitTm = ctrl->getPosVoteLimit(zone, lastHeight); __STP(limitTm);

			if(lastHeight > 1){
				ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(limitTm == nullptr
						, L"Block must be generated after the Pos Time Limit.", __FILE__, __LINE__);

				const SystemTimestamp* blockTm = header->getTimestamp();

				ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(blockTm->compareTo(limitTm) < 0
						, L"Block must be generated after the Pos Time Limit.", __FILE__, __LINE__);
			}
		}

		// check if has blockchain
		bool alreadyHasChain = false;
		{
			inst->validateZone(zone);

			// ReportMinedBlockNodeCommand
			alreadyHasChain = ctrl->hasHeaderId(zone, height, headerId);
		}

		if(!alreadyHasChain){
			// register header for PoS limit
			{
				const BlockHeader* header = this->data->getHeader();
				uint16_t zone = header->getZone();
				CodablecashSystemParam* param = inst->getCodablecashSystemParam();

				ctrl->registerBlockHeader4Limit(zone, header, param);
			}

			processor->addHistory(this->data);

			CentralProcessor* centralProcessor = inst->getCentralProcessor();

			// mined block comes
			TransferedMinedReportCommandMessage* message = new TransferedMinedReportCommandMessage(); __STP(message);
			message->setData(this->data);

			const NodeIdentifier* nodeId = nodeHandShake->getNodeId(); // source node id
			message->setNodeId(nodeId);

			centralProcessor->addCommandMessage(__STP_MV(message));
		}
	}

	return new OkPubsubResponse();
}

void ReportMinedBlockNodeCommand::setData(const BlockHeaderTransferData *data) {
	delete this->data;
	this->data = dynamic_cast<BlockHeaderTransferData*>(data->copyData());
}

} /* namespace codablecash */
