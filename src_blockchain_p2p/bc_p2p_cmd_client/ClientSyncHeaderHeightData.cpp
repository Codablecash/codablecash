/*
 * ClientSyncHeaderHeightData.cpp
 *
 *  Created on: May 4, 2025
 *      Author: iizuka
 */

#include "bc_p2p_cmd_client/ClientSyncHeaderHeightData.h"
#include "bc_p2p_cmd_client/ClientBlockHeaderTransferData.h"

#include "base/StackRelease.h"

#include "bc_block/BlockHeaderId.h"

#include "data_history/AbstractTransferedData.h"

#include "bc_base/BinaryUtils.h"
namespace codablecash {

ClientSyncHeaderHeightData::ClientSyncHeaderHeightData(const ClientSyncHeaderHeightData &inst) {
	this->height = inst.getHeight();
	this->list = new ArrayList<ClientBlockHeaderTransferData>();

	int maxLoop = inst.list->size();
	for(int i = 0; i != maxLoop; ++i){
		ClientBlockHeaderTransferData* data = inst.list->get(i);

		this->list->addElement(dynamic_cast<ClientBlockHeaderTransferData*>(data->copyData()));
	}
}

ClientSyncHeaderHeightData::ClientSyncHeaderHeightData() {
	this->height = 0;
	this->list = new ArrayList<ClientBlockHeaderTransferData>();
}

ClientSyncHeaderHeightData::~ClientSyncHeaderHeightData() {
	this->list->deleteElements();
	delete this->list;
}

int ClientSyncHeaderHeightData::binarySize() const {
	int total = sizeof(uint32_t);

	total += sizeof(this->height);

	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		ClientBlockHeaderTransferData* data = this->list->get(i);

		total += data->binarySize();
	}

	return total;
}

void ClientSyncHeaderHeightData::toBinary(ByteBuffer *out) const {
	out->putLong(this->height);

	int maxLoop = this->list->size();
	out->putInt(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		ClientBlockHeaderTransferData* data = this->list->get(i);

		data->toBinary(out);
	}
}

ClientSyncHeaderHeightData* ClientSyncHeaderHeightData::createFromBinary(ByteBuffer *in) {
	ClientSyncHeaderHeightData* inst = new ClientSyncHeaderHeightData(); __STP(inst);

	inst->height = in->getLong();

	int maxLoop = in->getInt();
	for(int i = 0; i != maxLoop; ++i){
		AbstractTransferedData* d = AbstractTransferedData::createFromBinary(in); __STP(d);
		ClientBlockHeaderTransferData* data = dynamic_cast<ClientBlockHeaderTransferData*>(d);

		BinaryUtils::checkNotNull(data);

		inst->addHeaderData(data);
	}

	return __STP_MV(inst);
}

IBlockObject* ClientSyncHeaderHeightData::copyData() const noexcept {
	return new ClientSyncHeaderHeightData(*this);
}

UnicodeString* ClientSyncHeaderHeightData::toString() const noexcept {
	UnicodeString* ret = new UnicodeString(L""); __STP(ret);

	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		ClientBlockHeaderTransferData* data = this->list->get(i);
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

void ClientSyncHeaderHeightData::addHeaderData(const ClientBlockHeaderTransferData *data) {
	this->list->addElement(dynamic_cast<ClientBlockHeaderTransferData*>(data->copyData()));
}

void ClientSyncHeaderHeightData::verifyVoteTransactionsData() const {
	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		ClientBlockHeaderTransferData* data = this->list->get(i);
		data->validateVoteTransactionsData();
	}
}

void ClientSyncHeaderHeightData::verifyTransactionsData() const {
	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		ClientBlockHeaderTransferData* data = this->list->get(i);
		data->validateTransactions();
	}
}

void ClientSyncHeaderHeightData::setHeight(uint64_t height) noexcept {
	this->height = height;
}

} /* namespace codablecash */
