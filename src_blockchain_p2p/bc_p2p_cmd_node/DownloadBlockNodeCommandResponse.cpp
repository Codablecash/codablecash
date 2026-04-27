/*
 * DownloadBlockNodeCommandResponse.cpp
 *
 *  Created on: 2023/11/18
 *      Author: iizuka
 */

#include "bc_p2p_cmd_node/DownloadBlockNodeCommandResponse.h"

#include "bc_block/Block.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "bc_block/BlockHeader.h"
#include "bc_block/BlockHeaderId.h"

#include "bc_base/BinaryUtils.h"

#include <cstdint>

namespace codablecash {

/*DownloadBlockNodeCommandResponse::DownloadBlockNodeCommandResponse(const DownloadBlockNodeCommandResponse &inst)
		: AbstractCommandResponse(inst) {
	this->block = inst.block != nullptr ? new Block(*inst.block) : nullptr;
}*/

DownloadBlockNodeCommandResponse::DownloadBlockNodeCommandResponse() : AbstractCommandResponse(TYPE_RES_DOWNLOAD_BLOCK) {
	this->block = nullptr;
}

DownloadBlockNodeCommandResponse::~DownloadBlockNodeCommandResponse() {
	delete this->block;
}

int DownloadBlockNodeCommandResponse::binarySize() const {
	BinaryUtils::checkNotNull(this->block);

	int total = sizeof(this->type);
	total += this->block->binarySize();

	return total;
}

void DownloadBlockNodeCommandResponse::toBinary(ByteBuffer *buff) const {
	BinaryUtils::checkNotNull(this->block);

	buff->putInt(this->type);

	this->block->toBinary(buff);
}

void DownloadBlockNodeCommandResponse::fromBinary(ByteBuffer *buff) {
	this->block = Block::createFromBinary(buff);
	BinaryUtils::checkNotNull(this->block);
}

UnicodeString* DownloadBlockNodeCommandResponse::toString() const noexcept {
	UnicodeString* ret = new UnicodeString(L"Block height : "); __STP(ret);
	BlockHeader* header = this->block->getHeader();
	uint64_t height = header->getHeight();

	ret->append((int64_t)height);
	ret->append(L" id: [");

	const BlockHeaderId* id = header->getId();
	UnicodeString* idstr = id->toString(); __STP(idstr);
	ret->append(idstr);

	ret->append(L"]");

	return __STP_MV(ret);
}

void DownloadBlockNodeCommandResponse::setBlock(const Block *block) {
	delete this->block;
	this->block = new Block(*block);
}

} /* namespace codablecash */
