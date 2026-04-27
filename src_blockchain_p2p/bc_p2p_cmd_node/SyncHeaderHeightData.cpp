/*
 * SyncHeaderHeightData.cpp
 *
 *  Created on: 2023/11/07
 *      Author: iizuka
 */

#include "bc_p2p_cmd_node/SyncHeaderHeightData.h"

#include "data_history_data/BlockHeaderTransferData.h"

#include "base/StackRelease.h"

#include "bc_base/BinaryUtils.h"

#include "bc_block/BlockHeader.h"
#include "bc_block/BlockHeaderId.h"


namespace codablecash {

SyncHeaderHeightData::SyncHeaderHeightData(const SyncHeaderHeightData &inst) {
	this->height = inst.getHeight();
	this->list = new ArrayList<BlockHeaderTransferData>();

	int maxLoop = inst.list->size();
	for(int i = 0; i != maxLoop; ++i){
		BlockHeaderTransferData* data = inst.list->get(i);

		this->list->addElement(dynamic_cast<BlockHeaderTransferData*>(data->copyData()));
	}
}

SyncHeaderHeightData::SyncHeaderHeightData() {
	this->height = 0;
	this->list = new ArrayList<BlockHeaderTransferData>();
}

SyncHeaderHeightData::~SyncHeaderHeightData() {
	this->list->deleteElements();
	delete this->list;
}

int SyncHeaderHeightData::binarySize() const {
	int total = sizeof(uint32_t);

	total += sizeof(this->height);

	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		BlockHeaderTransferData* data = this->list->get(i);

		total += data->binarySize();
	}

	return total;
}

void SyncHeaderHeightData::toBinary(ByteBuffer *out) const {
	out->putLong(this->height);

	int maxLoop = this->list->size();
	out->putInt(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		BlockHeaderTransferData* data = this->list->get(i);

		data->toBinary(out);
	}
}

SyncHeaderHeightData* SyncHeaderHeightData::createFromBinary(ByteBuffer *in) {
	SyncHeaderHeightData* inst = new SyncHeaderHeightData(); __STP(inst);

	inst->height = in->getLong();

	int maxLoop = in->getInt();
	for(int i = 0; i != maxLoop; ++i){
		AbstractTransferedData* d = AbstractTransferedData::createFromBinary(in); __STP(d);
		BlockHeaderTransferData* data = dynamic_cast<BlockHeaderTransferData*>(d);

		BinaryUtils::checkNotNull(data);

		inst->addHeaderData(data);
	}

	return __STP_MV(inst);
}

IBlockObject* SyncHeaderHeightData::copyData() const noexcept {
	return new SyncHeaderHeightData(*this);
}

UnicodeString* SyncHeaderHeightData::toString() const noexcept {
	UnicodeString* ret = new UnicodeString(L""); __STP(ret);

	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		BlockHeaderTransferData* data = this->list->get(i);
		const BlockHeader* header = data->getHeader();
		uint64_t height = header->getHeight();
		const BlockHeaderId* id = header->getId();

		if(i != 0){
			ret->append(L", ");
		}

		ret->append(L"Height : ");
		ret->append((int64_t)height);
		ret->append(L"[");
		{
			UnicodeString* str = id->toString(); __STP(str);
			ret->append(str);
		}
		ret->append(L"]");
	}


	return __STP_MV(ret);
}

void SyncHeaderHeightData::setHeight(uint64_t height) noexcept {
	this->height = height;
}

void SyncHeaderHeightData::addHeaderData(const BlockHeaderTransferData *data) {
	this->list->addElement(dynamic_cast<BlockHeaderTransferData*>(data->copyData()));
}

void SyncHeaderHeightData::verifyVoteTransactionsData() const {
	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		BlockHeaderTransferData* data = this->list->get(i);
		data->validateVoteTransactionsData();
	}
}

} /* namespace codablecash */
