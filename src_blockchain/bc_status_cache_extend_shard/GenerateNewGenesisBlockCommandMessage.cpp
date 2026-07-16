/*
 * GenerateNewGenesisBlockCommandMessage.cpp
 *
 *  Created on: Jul 5, 2026
 *      Author: iizuka
 */

#include "bc_status_cache_extend_shard/GenerateNewGenesisBlockCommandMessage.h"

#include "bc_processor/CentralProcessor.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc_block/Block.h"


namespace codablecash {

GenerateNewGenesisBlockCommandMessage::GenerateNewGenesisBlockCommandMessage() {
	this->newShardZone = 0;
	this->genesisBlock = nullptr;
}

GenerateNewGenesisBlockCommandMessage::~GenerateNewGenesisBlockCommandMessage() {
	delete this->genesisBlock;
}


void GenerateNewGenesisBlockCommandMessage::process(CentralProcessor *processor) {
	BlockchainController* ctrl = processor->getCtrl();
	ctrl->addBlock(this->genesisBlock);
}

void GenerateNewGenesisBlockCommandMessage::setNewShardZone(uint16_t newShardZone) noexcept {
	this->newShardZone = newShardZone;
}

void GenerateNewGenesisBlockCommandMessage::setGenesisBlock(const Block *block) {
	delete this->genesisBlock;
	this->genesisBlock = new Block(*block);
}

} /* namespace codablecash */
