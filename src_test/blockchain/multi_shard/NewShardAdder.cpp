/*
 * NewShardAdder.cpp
 *
 *  Created on: Jun 24, 2026
 *      Author: iizuka
 */

#include "NewShardAdder.h"

#include "bc_block/Block.h"
#include "bc_block/BlockHeader.h"

#include "bc_block_header_command/NewShardZoneCommand.h"

namespace codablecash {

NewShardAdder::NewShardAdder(const NewShardAdder &inst) {
	this->height = inst.height;

	this->newShardZone = inst.newShardZone;
	this->requestingZone = inst.requestingZone;
}

NewShardAdder::NewShardAdder() {
	this->height = 0;
	this->newShardZone = 0;
	this->requestingZone = 0;
}

NewShardAdder::~NewShardAdder() {

}

void NewShardAdder::init(uint16_t newShardZone, uint16_t requestingZone) {
	this->newShardZone = newShardZone;
	this->requestingZone = requestingZone;
}

void NewShardAdder::onBlockGenerated(Block *block, MemPoolTransaction *memTrx, BlockchainController *ctrl) {
	uint16_t height = block->getHeight();
	if(height == this->height){
		__addCommand(block, memTrx, ctrl);
	}
}

void NewShardAdder::__addCommand(Block *block, MemPoolTransaction *memTrx, BlockchainController *ctrl) {
	BlockHeader* header = block->getHeader();

	NewShardZoneCommand cmd;
	cmd.setNewShardZone(this->newShardZone);
	cmd.setRequestingZone(this->requestingZone);

	Block gblock(this->newShardZone, 1);
	gblock.build();
	cmd.setGenesisblock(&gblock);

	header->addHeaderCommand(&cmd);
}

IBlockGenerationListner* NewShardAdder::copy() const noexcept {
	return new NewShardAdder(*this);
}

void NewShardAdder::setHeight(uint64_t height) {
	this->height = height;
}

} /* namespace codablecash */
