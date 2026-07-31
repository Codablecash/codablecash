/*
 * BlockHeaderCommand.cpp
 *
 *  Created on: Jul 23, 2026
 *      Author: iizuka
 */

#include "base/StackRelease.h"
#include "bc_block_header_command/BlockHeaderCommandId.h"


namespace codablecash {

BlockHeaderCommandId::BlockHeaderCommandId(const BlockHeaderCommandId &inst) : Abstract32BytesId((const char*)inst.id->array(), inst.id->limit()) {
}

BlockHeaderCommandId::BlockHeaderCommandId() : Abstract32BytesId() {

}

BlockHeaderCommandId::BlockHeaderCommandId(const char *binary, int length) : Abstract32BytesId(binary, length) {

}

BlockHeaderCommandId::~BlockHeaderCommandId() {

}

BlockHeaderCommandId* BlockHeaderCommandId::fromBinary(ByteBuffer *in) {
	BlockHeaderCommandId* commandId = new BlockHeaderCommandId();

	int cap = in->getInt();

	uint8_t* buff = new uint8_t[cap];
	StackArrayRelease<uint8_t> __st_buff(buff);

	in->get(buff, cap);

	commandId->id = ByteBuffer::wrapWithEndian(buff, cap, true);
	commandId->id->position(0);

	return commandId;
}

IBlockObject* BlockHeaderCommandId::copyData() const noexcept {
	return new BlockHeaderCommandId(*this);
}

BlockHeaderCommandId* BlockHeaderCommandId::makeRandomHeaderId() {
	ByteBuffer* buff = makeRandom16Bytes(); __STP(buff);

	BlockHeaderCommandId* blockHeaderId = new BlockHeaderCommandId((const char *)buff->array(), buff->capacity());

	return blockHeaderId;
}

} /* namespace codablecash */
