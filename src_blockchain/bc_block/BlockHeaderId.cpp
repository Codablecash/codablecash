/*
 * BlockHeaderId.cpp
 *
 *  Created on: 2023/02/16
 *      Author: iizuka
 */

#include "bc_block/BlockHeaderId.h"

#include "base/StackRelease.h"

#include "base_io/ByteBuffer.h"

#include "numeric/BigInteger.h"
namespace codablecash {

BlockHeaderId::BlockHeaderId(const BlockHeaderId &inst) : Abstract32BytesId((const char*)inst.id->array(), inst.id->limit()) {

}

BlockHeaderId::BlockHeaderId() : Abstract32BytesId() {
}

BlockHeaderId::BlockHeaderId(const char *binary, int length) : Abstract32BytesId(binary, length) {
}

BlockHeaderId::~BlockHeaderId() {
}

BlockHeaderId* BlockHeaderId::fromBinary(ByteBuffer* in) {
	BlockHeaderId* trxId = new BlockHeaderId();

	int cap = in->getInt();

	uint8_t* buff = new uint8_t[cap];
	StackArrayRelease<uint8_t> __st_buff(buff);

	in->get(buff, cap);

	trxId->id = ByteBuffer::wrapWithEndian(buff, cap, true);
	trxId->id->position(0);

	return trxId;
}

IBlockObject* BlockHeaderId::copyData() const noexcept {
	this->id->position(0);
	return new BlockHeaderId(*this);
}

BlockHeaderId* BlockHeaderId::makeRandomHeaderId() {
	ByteBuffer* buff = makeRandom16Bytes(); __STP(buff);

	BlockHeaderId* blockHeaderId = new BlockHeaderId((const char *)buff->array(), buff->capacity());

	return blockHeaderId;
}

} /* namespace codablecash */
