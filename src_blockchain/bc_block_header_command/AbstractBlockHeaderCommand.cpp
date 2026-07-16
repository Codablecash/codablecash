/*
 * AbstractBlockHeaderCommand.cpp
 *
 *  Created on: Jun 13, 2026
 *      Author: iizuka
 */

#include "bc_block_header_command/AbstractBlockHeaderCommand.h"
#include "bc_block_header_command/NewShardZoneCommand.h"
#include "bc_block_header_command/RecognizedNewShardCommand.h"

#include "base_io/ByteBuffer.h"

#include "base/StackRelease.h"


namespace codablecash {

AbstractBlockHeaderCommand::AbstractBlockHeaderCommand(const AbstractBlockHeaderCommand &inst) {
	this->type = inst.type;
}

AbstractBlockHeaderCommand::AbstractBlockHeaderCommand(uint16_t type) {
	this->type = type;
}

AbstractBlockHeaderCommand::~AbstractBlockHeaderCommand() {

}

AbstractBlockHeaderCommand* AbstractBlockHeaderCommand::createFromBinary(ByteBuffer* in) {
	uint16_t t = in->getShort();

	AbstractBlockHeaderCommand* ret = nullptr;

	switch(t){
	case NEW_SHARD_COMMAND:
		ret = new NewShardZoneCommand();
		break;
	case RECOGNIZED_SHARD_COMMAND:
		ret = new RecognizedNewShardCommand();
		break;
	default:
		return nullptr;
	}
	__STP(ret);

	ret->fromBinary(in);

	return __STP_MV(ret);
}

} /* namespace codablecash */
