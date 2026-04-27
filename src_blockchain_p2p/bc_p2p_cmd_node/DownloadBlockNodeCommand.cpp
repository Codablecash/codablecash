/*
 * DownloadBlockNodeCommand.cpp
 *
 *  Created on: 2023/11/18
 *      Author: iizuka
 */

#include "bc_p2p_cmd_node/DownloadBlockNodeCommand.h"

#include "bc_block/BlockHeaderId.h"
#include "bc_block/Block.h"

#include "bc_base/BinaryUtils.h"

#include "bc/CodablecashNodeInstance.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc_p2p_cmd_node/DownloadBlockNodeCommandResponse.h"

#include "base/StackRelease.h"
#include "base/ArrayList.h"

#include "bc_block/BlockHeader.h"

#include "pubsub_cmd/WarningResponse.h"
namespace codablecash {

DownloadBlockNodeCommand::DownloadBlockNodeCommand(const DownloadBlockNodeCommand& inst) : AbstractNodeCommand(inst) {
	this->zone = inst.zone;
	this->height = inst.height;
	this->headerId = inst.headerId != nullptr ? dynamic_cast<BlockHeaderId*>(inst.headerId->copyData()) : nullptr;
}

DownloadBlockNodeCommand::DownloadBlockNodeCommand() : AbstractNodeCommand(TYPE_NODE_DOWNLOAD_BLOCK) {
	this->zone = 0;
	this->height = 0;
	this->headerId = nullptr;
}

DownloadBlockNodeCommand::~DownloadBlockNodeCommand() {
	delete this->headerId;
}

int DownloadBlockNodeCommand::binarySize() const {
	BinaryUtils::checkNotNull(this->headerId);

	int total = AbstractNodeCommand::binarySize();

	total += sizeof(this->zone);
	total += sizeof(this->height);
	total += this->headerId->binarySize();

	return total;
}

void DownloadBlockNodeCommand::toBinary(ByteBuffer *buff) const {
	BinaryUtils::checkNotNull(this->headerId);

	AbstractNodeCommand::toBinary(buff);

	buff->putShort(this->zone);
	buff->putLong(this->height);
	this->headerId->toBinary(buff);
}

ByteBuffer* DownloadBlockNodeCommand::getSignBinary() const {
	BinaryUtils::checkNotNull(this->headerId);

	int total =  sizeof(this->zone);
	total += sizeof(this->height);
	total += this->headerId->binarySize();

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(total, true);
	buff->putShort(this->zone);
	buff->putLong(this->height);
	this->headerId->toBinary(buff);

	return buff;
}

void DownloadBlockNodeCommand::fromBinary(ByteBuffer *buff) {
	AbstractNodeCommand::fromBinary(buff);

	this->zone = buff->getShort();
	this->height = buff->getLong();
	this->headerId = BlockHeaderId::fromBinary(buff);

	BinaryUtils::checkNotNull(this->headerId);
}

IBlockObject* DownloadBlockNodeCommand::copyData() const noexcept {
	return new DownloadBlockNodeCommand(*this);
}

bool DownloadBlockNodeCommand::usePendingQueue() const noexcept {
	return false;
}

AbstractCommandResponse* DownloadBlockNodeCommand::executeAsNode(BlockchainNodeHandshake *nodeHandShake, CodablecashNodeInstance *inst, bool suspend) const {
	DownloadBlockNodeCommandResponse* response = new DownloadBlockNodeCommandResponse(); __STP(response);
	{
		BlockchainController* ctrl = inst->getController();
		Block* block = ctrl->getBlockHeightAt(this->zone, this->height, this->headerId); __STP(block);

		if(block != nullptr){
			response->setBlock(block);
		}
		else {
			WarningResponse* wres = new WarningResponse();
			UnicodeString str(L"Cannot find block header id: ");
			{
				UnicodeString* idstr = this->headerId->toString(); __STP(idstr);
				str.append(idstr);
			}
			str.append(L" at height ");
			{
				str.append((int64_t)this->height);
			}
			wres->setMessage(&str);

			return wres;
		}
	}

	return __STP_MV(response);
}

void DownloadBlockNodeCommand::setZone(uint16_t zone) noexcept {
	this->zone = zone;
}

void DownloadBlockNodeCommand::setHeight(uint64_t height) noexcept {
	this->height = height;
}

void DownloadBlockNodeCommand::setHeaderId(const BlockHeaderId *headerId) noexcept {
	delete this->headerId;
	this->headerId = dynamic_cast<BlockHeaderId*>(headerId->copyData());
}

} /* namespace codablecash */
