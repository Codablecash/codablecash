/*
 * RemoteUtxoHeight.cpp
 *
 *  Created on: Jul 9, 2026
 *      Author: iizuka
 */

#include "bc_status_cache_context/RemoteUtxoHeight.h"

#include "bc_trx/UtxoId.h"


namespace codablecash {

RemoteUtxoHeight::RemoteUtxoHeight(const RemoteUtxoHeight &inst) {
	this->height = inst.height;
	this->utxoId = dynamic_cast<UtxoId*>(inst.utxoId->copyData());
}

RemoteUtxoHeight::RemoteUtxoHeight(uint64_t height, const UtxoId* utxoId) {
	this->height = height;
	this->utxoId = dynamic_cast<UtxoId*>(utxoId->copyData());
}

RemoteUtxoHeight::~RemoteUtxoHeight() {
	delete this->utxoId;
}

} /* namespace codablecash */
