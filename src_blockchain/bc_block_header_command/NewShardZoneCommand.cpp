/*
 * NewShardZoneCommand.cpp
 *
 *  Created on: Jun 13, 2026
 *      Author: iizuka
 */

#include "bc_block_header_command/NewShardZoneCommand.h"

#include "bc_blockstore/CodablecashBlockchain.h"

#include "bc_status_cache/BlockchainStatusCache.h"

#include "bc_status_cache_extend_shard/NotifyShardExtendRequestCommandMessage.h"

#include "bc_processor/CentralProcessor.h"

#include "bc_status_cache_extend_shard/GenerateNewGenesisBlockCommandMessage.h"

#include "bc_block/BlockHeaderId.h"
#include "bc_block/BlockHeader.h"
#include "bc_block/Block.h"

#include "bc_base/BinaryUtils.h"
namespace codablecash {


NewShardZoneCommand::NewShardZoneCommand(const NewShardZoneCommand &inst) : AbstractBlockHeaderCommand(inst) {
	this->newShardZone = inst.newShardZone;
	this->requestingZone = inst.requestingZone;
	this->genesisBlock = inst.genesisBlock != nullptr ? new Block(*inst.genesisBlock) : nullptr;
}

NewShardZoneCommand::NewShardZoneCommand() : AbstractBlockHeaderCommand(AbstractBlockHeaderCommand::NEW_SHARD_COMMAND) {
	this->newShardZone = 0;
	this->requestingZone = 0;
	this->genesisBlock =  nullptr;
}

NewShardZoneCommand::~NewShardZoneCommand() {
	delete this->genesisBlock;
}

int NewShardZoneCommand::binarySize() const {
	BinaryUtils::checkNotNull(this->genesisBlock);

	int total = sizeof(uint16_t);

	total += sizeof(uint16_t);
	total += sizeof(uint16_t);
	total += this->genesisBlock->binarySize();

	return total;
}

void NewShardZoneCommand::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->genesisBlock);

	out->putShort(this->type);

	out->putShort(this->newShardZone);
	out->putShort(this->requestingZone);
	this->genesisBlock->toBinary(out);
}

void NewShardZoneCommand::fromBinary(ByteBuffer* in) {
	this->newShardZone = in->getShort();
	this->requestingZone = in->getShort();

	this->genesisBlock = Block::createFromBinary(in);
}

IBlockObject* NewShardZoneCommand::copyData() const noexcept {
	return new NewShardZoneCommand(*this);
}

void NewShardZoneCommand::setNewShardZone(uint16_t newShardZone) noexcept {
	this->newShardZone = newShardZone;
}

void NewShardZoneCommand::setRequestingZone(uint16_t requestingZone) noexcept {
	this->requestingZone = requestingZone;
}

void NewShardZoneCommand::onFinalize(const BlockHeader *header, BlockchainStatusCache *statusCache, CodablecashBlockchain *blockchain,
		ILockinManager *lockinManager, const CodablecashSystemParam *config) {
	uint16_t zoneSelf = statusCache->getZoneSelf();

	// FIXME[multishard] create phisical store
	if(zoneSelf != this->newShardZone){
		statusCache->newZone(false);
		blockchain->addZone(this->newShardZone);
	}else{
		statusCache->newZone(true);
		blockchain->addZone(this->newShardZone);

		//[multishard] generate genesis block
		CentralProcessor* processor = blockchain->getProcessor();

		GenerateNewGenesisBlockCommandMessage* message = new GenerateNewGenesisBlockCommandMessage();
		message->setNewShardZone(this->newShardZone);

		processor->addCommandMessage(message);
	}


	// broad cast ICC
	if(zoneSelf == this->requestingZone){ // make inter shard communication trx
		CentralProcessor* processor = blockchain->getProcessor();

		NotifyShardExtendRequestCommandMessage* message = new NotifyShardExtendRequestCommandMessage();
		message->setNewShardZone(this->newShardZone);
		message->setRequestingZone(this->requestingZone);

		uint64_t height = header->getHeight();
		const BlockHeaderId* headerId = header->getId();
		message->setHeaderInfo(height, headerId);

		processor->addCommandMessage(message);
	}
}

void NewShardZoneCommand::setGenesisblock(const Block *block) {
	delete this->genesisBlock;
	this->genesisBlock = new Block(*block);
}

} /* namespace codablecash */
