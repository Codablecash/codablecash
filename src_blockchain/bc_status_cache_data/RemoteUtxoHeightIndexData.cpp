/*
 * RemoteUtxoHeightIndexData.cpp
 *
 *  Created on: Jul 8, 2026
 *      Author: iizuka
 */

#include "bc_status_cache_data/RemoteUtxoHeightIndexData.h"

#include "bc_trx/UtxoId.h"

#include "base/StackRelease.h"


namespace codablecash {

RemoteUtxoHeightIndexData::RemoteUtxoHeightIndexData(const RemoteUtxoHeightIndexData &inst) {
	this->list = new ArrayList<UtxoId>();

	int maxLoop = inst.list->size();
	for(int i = 0; i != maxLoop; ++i){
		const UtxoId* v = inst.list->get(i);

		this->list->addElement(dynamic_cast<UtxoId*>(v->copyData()));
	}
}

RemoteUtxoHeightIndexData::RemoteUtxoHeightIndexData() {
	this->list = new ArrayList<UtxoId>();
}

RemoteUtxoHeightIndexData::~RemoteUtxoHeightIndexData() {
	this->list->deleteElements();
	delete this->list;
}

int RemoteUtxoHeightIndexData::binarySize() const {
	int total = 0;

	int maxLoop = this->list->size();
	total += sizeof(uint16_t);

	for(int i = 0; i != maxLoop; ++i){
		const UtxoId* v = this->list->get(i);
		total += v->binarySize();
	}

	return total;
}

void RemoteUtxoHeightIndexData::toBinary(ByteBuffer *out) const {
	int maxLoop = this->list->size();
	out->putShort(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		const UtxoId* v = this->list->get(i);
		v->toBinary(out);
	}
}

RemoteUtxoHeightIndexData* RemoteUtxoHeightIndexData::createFromBinary(ByteBuffer *in) {
	RemoteUtxoHeightIndexData* data = new RemoteUtxoHeightIndexData(); __STP(data);

	int maxLoop = in->getShort();

	for(int i = 0; i != maxLoop; ++i){
		UtxoId* v = UtxoId::fromBinary(in);
		data->list->addElement(v);
	}

	return data;
}

IBlockObject* RemoteUtxoHeightIndexData::copyData() const noexcept {
	return new RemoteUtxoHeightIndexData(*this);
}

void RemoteUtxoHeightIndexData::join(const RemoteUtxoHeightIndexData *value) noexcept {
	int maxLoop = value->list->size();
	for(int i = 0; i != maxLoop; ++i){
		const UtxoId* v =  value->list->get(i);

		if(contains(v)){
			continue;
		}

		UtxoId* newTrx = dynamic_cast<UtxoId*>(v->copyData());
		this->list->addElement(newTrx);
	}
}

bool RemoteUtxoHeightIndexData::contains(const UtxoId *utxoId) const noexcept {
	int index = indexof(utxoId);
	return index >= 0;
}

void RemoteUtxoHeightIndexData::remove(const UtxoId *utxoId) noexcept {
	int index = indexof(utxoId);

	if(index >= 0){
		this->list->remove(index);
	}
}

int RemoteUtxoHeightIndexData::indexof(const UtxoId *utxoId) const noexcept {
	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		UtxoId* v = this->list->get(i);
		if(utxoId->equals(v)){
			return i;
		}
	}

	return -1;
}

void RemoteUtxoHeightIndexData::add(const UtxoId *utxoId) noexcept {
	this->list->addElement(dynamic_cast<UtxoId*>(utxoId->copyData()));
}

} /* namespace codablecash */
