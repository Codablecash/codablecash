/*
 * ReportNonceCalculatedNodeCommand.cpp
 *
 *  Created on: 2024/07/23
 *      Author: iizuka
 */

#include "bc_p2p_cmd_node_consensus/ReportNonceCalculatedNodeCommand.h"

#include "base_timestamp/SystemTimestamp.h"

#include "bc_block/BlockHeader.h"
#include "bc_block/BlockHeaderId.h"

#include "bc_base/BinaryUtils.h"

#include "bc/CodablecashNodeInstance.h"
#include "bc/ExceptionThrower.h"

#include "bc_p2p_processor/P2pRequestProcessor.h"

#include "data_history_data/BlockHeaderTransferData.h"

#include "bc_status_cache/BlockchainController.h"

#include "pubsub_cmd/OkPubsubResponse.h"
#include "pubsub_cmd/WarningResponse.h"

#include "pow/PoWNonceResult.h"

#include "base/StackRelease.h"

#include "numeric/BigInteger.h"

#include "bc_block_validator/BlockValidationException.h"

#include "bc_p2p/BlochchainP2pManager.h"

#include "bc_network/NodeIdentifier.h"
#include "bc_network/NodeIdentifierSource.h"
#include "bc_p2p_cmd_network/NodeNetworkInfo.h"


namespace codablecash {

ReportNonceCalculatedNodeCommand::ReportNonceCalculatedNodeCommand(const ReportNonceCalculatedNodeCommand &inst)
		: AbstractConsensusNodeCommand(inst) {
	this->header = inst.header != nullptr ? dynamic_cast<BlockHeader*>(inst.header->copyData()) : nullptr;
}

ReportNonceCalculatedNodeCommand::ReportNonceCalculatedNodeCommand()
	: AbstractConsensusNodeCommand(AbstractConsensusNodeCommand::TYPE_CONSENSUS_REPORT_NONCE_CALCULATED) {
	this->header = nullptr;
}

ReportNonceCalculatedNodeCommand::~ReportNonceCalculatedNodeCommand() {
	delete this->header;
}

SystemTimestamp ReportNonceCalculatedNodeCommand::getFirstTimestamp() const {
	const SystemTimestamp* tm = this->header->getTimestamp();

	return *tm;
}

int ReportNonceCalculatedNodeCommand::binarySize() const {
	BinaryUtils::checkNotNull(this->header);

	int total = AbstractConsensusNodeCommand::binarySize();

	total += this->header->binarySize();

	return total;

}

void ReportNonceCalculatedNodeCommand::toBinary(ByteBuffer *buff) const {
	BinaryUtils::checkNotNull(this->header);

	AbstractConsensusNodeCommand::toBinary(buff);
	this->header->toBinary(buff);
}

void ReportNonceCalculatedNodeCommand::fromBinary(ByteBuffer *buff) {
	AbstractConsensusNodeCommand::fromBinary(buff);

	this->header = BlockHeader::createFromBinary(buff);
	BinaryUtils::checkNotNull(this->header);
}

ByteBuffer* ReportNonceCalculatedNodeCommand::getSignBinary() const {
	BinaryUtils::checkNotNull(this->header);

	int total = this->header->binarySize();

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(total, true);
	this->header->toBinary(buff);

	return buff;

}

IBlockObject* ReportNonceCalculatedNodeCommand::copyData() const noexcept {
	return new ReportNonceCalculatedNodeCommand(*this);
}

AbstractCommandResponse* ReportNonceCalculatedNodeCommand::executeAsNode(BlockchainNodeHandshake *nodeHandShake, CodablecashNodeInstance *inst, bool suspend) const {
	P2pRequestProcessor* p2pRequestProcessor = inst->getP2pRequestProcessor();
	BlochchainP2pManager* p2pManager = inst->getBlochchainP2pManager();
	BlockchainController* ctrl = inst->getController();

	{
		const PoWNonce* nonce = this->header->getPoWNonce();
		PoWNonceResult* res = nonce->calcResult(this->header->getLastHeaderId(), this->header->getMerkleRoot(), this->header->getTimestamp()); __STP(res);
		BigInteger diff = res->getDifficulty();

		uint16_t zone = this->header->getZone();
		uint64_t lastHeight = this->header->getHeight() - 1;
		const BlockHeaderId *lastHeaderId = this->header->getLastHeaderId();
		const SystemTimestamp* tm = this->header->getTimestamp();

		// check exists
		if(!ctrl->hasHeaderId(zone, lastHeight, lastHeaderId)){
			WarningResponse* res = new WarningResponse();
			UnicodeString message(L"Cannot find last header id ");
			UnicodeString* strId =  lastHeaderId->toString(); __STP(strId);
			message.append(strId);

			res->setMessage(&message);

			return res;
		}

		BigInteger targetDiff = ctrl->calcTargetDifficulty(zone, lastHeight, lastHeaderId, tm);

		ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(diff.compareTo(targetDiff) < 0
				, L"Difficulty of the hash is not enough.", __FILE__, __LINE__);
	}

	// [consensus] validate Pos and header time
	{
		uint16_t zone = this->header->getZone();
		uint64_t lastHeight = this->header->getHeight() - 1;
		SystemTimestamp* limitTm = ctrl->getPosVoteLimit(zone, lastHeight); __STP(limitTm);

		if(lastHeight > 1){
			ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(limitTm == nullptr
					, L"Block must be generated after the Pos Time Limit.", __FILE__, __LINE__);

			const SystemTimestamp* blockTm = this->header->getTimestamp();

			ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(blockTm->compareTo(limitTm) < 0
					, L"Block must be generated after the Pos Time Limit.", __FILE__, __LINE__);
		}
	}

	// register header for PoS limit
	bool updated = false;
	{
		uint16_t zone = this->header->getZone();
		CodablecashSystemParam* config = inst->getCodablecashSystemParam();

		updated = ctrl->registerBlockHeader4Limit(zone, this->header, config);
	}


	if(!suspend && updated){
		// [consensus] Request Stake vote
		{
			uint64_t height = this->header->getHeight();
			uint16_t zone = this->header->getZone();
			ctrl->requestPosVote(zone, height);
		}


		ReportNonceCalculatedNodeCommand cmd(*this);
		cmd.setHeader(this->header);

		// add self to the history
		NodeIdentifierSource* nwkey = p2pRequestProcessor->getNetworkKey();
		{
			uint16_t zone = inst->getZoneSelf();
			NodeIdentifier nodeId = nwkey->toNodeIdentifier();

			int protocol = p2pManager->getProtocol();
			const UnicodeString* host = p2pManager->getHost();
			uint32_t port = p2pManager->getPort();

			cmd.addHistory(zone, &nodeId, protocol, host, port);
		}
		cmd.sign(nwkey);

		ArrayList<NodeIdentifier> list;
		list.setDeleteOnExit(true);
		{
			const ArrayList<NodeNetworkInfo>* history = cmd.getHistory();
			int maxLoop = history->size();
			for(int i = 0; i != maxLoop; ++i){
				NodeNetworkInfo* his = history->get(i);
				const NodeIdentifier* nodeId = his->getNodeIdentifier();

				NodeIdentifier* newId = dynamic_cast<NodeIdentifier*>(nodeId->copyData());
				list.addElement(newId);
			}
		}

		// broadcast
		p2pManager->bloadCastHighPriorityAllZones(&list, &cmd, p2pRequestProcessor);
	}

	return new OkPubsubResponse();
}

void ReportNonceCalculatedNodeCommand::setHeader(const BlockHeader *header) {
	delete this->header;
	this->header = dynamic_cast<BlockHeader*>(header->copyData());
}

} /* namespace codablecash */
