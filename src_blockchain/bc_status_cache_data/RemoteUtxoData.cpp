/*
 * RemoteUtxoData.cpp
 *
 *  Created on: Jul 7, 2026
 *      Author: iizuka
 */

#include "bc_status_cache_data/RemoteUtxoData.h"

#include "bc_trx/UtxoId.h"

#include "bc_base/BinaryUtils.h"

#include "base/StackRelease.h"


namespace codablecash {

RemoteUtxoData::RemoteUtxoData(const RemoteUtxoData &inst) {
	this->utxoId = inst.utxoId != nullptr ? dynamic_cast<UtxoId*>(inst.utxoId->copyData()) : nullptr;
	this->height = inst.height;
}

RemoteUtxoData::RemoteUtxoData(const UtxoId* id, uint64_t height) {
	this->utxoId = dynamic_cast<UtxoId*>(id->copyData());
	this->height = height;
}

RemoteUtxoData::~RemoteUtxoData() {
	delete this->utxoId;
}

int RemoteUtxoData::binarySize() const {
	BinaryUtils::checkNotNull(this->utxoId);

	int total = this->utxoId->binarySize();
	total += sizeof(uint64_t);

	return total;
}

void RemoteUtxoData::toBinary(ByteBuffer *out) const {
	this->utxoId->toBinary(out);
	out->putLong(this->height);
}

RemoteUtxoData* RemoteUtxoData::createFromBinary(ByteBuffer *in) {
	UtxoId* id = UtxoId::fromBinary(in); __STP(id);
	uint64_t height = in->getLong();

	return new RemoteUtxoData(__STP_MV(id), height);
}

IBlockObject* RemoteUtxoData::copyData() const noexcept {
	return new RemoteUtxoData(*this);
}

} /* namespace codablecash */
