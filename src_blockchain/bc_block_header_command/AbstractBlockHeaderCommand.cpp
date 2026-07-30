/*
 * AbstractBlockHeaderCommand.cpp
 *
 *  Created on: Jun 13, 2026
 *      Author: iizuka
 */

#include "bc_block_header_command/AbstractBlockHeaderCommand.h"
#include "bc_block_header_command/NewShardZoneCommand.h"
#include "bc_block_header_command/RecognizedNewShardCommand.h"
#include "bc_block_header_command/BlockHeaderCommandId.h"

#include "base_io/ByteBuffer.h"

#include "base/StackRelease.h"

#include "crypto/Sha256.h"


namespace codablecash {

AbstractBlockHeaderCommand::AbstractBlockHeaderCommand(const AbstractBlockHeaderCommand &inst) {
	this->type = inst.type;
	this->commandId = inst.commandId != nullptr ? dynamic_cast<BlockHeaderCommandId*>(inst.commandId->copyData()) : nullptr;
}

AbstractBlockHeaderCommand::AbstractBlockHeaderCommand(uint16_t type) {
	this->type = type;
	this->commandId = nullptr;
}

AbstractBlockHeaderCommand::~AbstractBlockHeaderCommand() {
	delete this->commandId;
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
	ret->buildCommandId();

	return __STP_MV(ret);
}

void AbstractBlockHeaderCommand::buildCommandId() {
	int cap = binarySize();

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);
	toBinary(buff);

	buff->position(0);
	ByteBuffer* sha = Sha256::sha256(buff, true); __STP(sha);

	delete this->commandId;
	this->commandId = new BlockHeaderCommandId((const char*)sha->array(), sha->limit());
}

} /* namespace codablecash */
