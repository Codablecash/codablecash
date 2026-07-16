/*
 * GenerateNewGenesisBlockCommandMessage.cpp
 *
 *  Created on: Jul 5, 2026
 *      Author: iizuka
 */

#include "bc_status_cache_extend_shard/GenerateNewGenesisBlockCommandMessage.h"

namespace codablecash {

GenerateNewGenesisBlockCommandMessage::GenerateNewGenesisBlockCommandMessage() {
	this->newShardZone = 0;
}

GenerateNewGenesisBlockCommandMessage::~GenerateNewGenesisBlockCommandMessage() {

}


void GenerateNewGenesisBlockCommandMessage::process(CentralProcessor *processor) {

}

void GenerateNewGenesisBlockCommandMessage::setNewShardZone(uint16_t newShardZone) noexcept {
	this->newShardZone = newShardZone;
}

} /* namespace codablecash */
