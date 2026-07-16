/*
 * RemoteUtxoDetector.cpp
 *
 *  Created on: Jul 9, 2026
 *      Author: iizuka
 */

#include "bc_status_cache_context/RemoteUtxoDetector.h"
#include "bc_status_cache_context/RemoteUtxoHeight.h"

#include "bc_trx/UtxoId.h"

#include "bc_status_cache_data/RemoteUtxoRepository.h"

#include "bc/CodablecashSystemParam.h"

#include "base/StackRelease.h"

#include "bc_status_cache_data/RemoteUtxoData.h"
namespace codablecash {

RemoteUtxoDetector::RemoteUtxoDetector(RemoteUtxoRepository* finalizedUtxoRepo, uint64_t finalizedHeight, const CodablecashSystemParam* config, const BlockchainSoftwareVersion* version) {
	this->finalizedUtxoRepo = finalizedUtxoRepo;
	this->list = new ArrayList<RemoteUtxoHeight>();

	uint64_t expire = config->getRemoteUtxoExpireHeight(version);
	this->expiredHeight = finalizedHeight > expire ? finalizedHeight - expire : 0;
}

RemoteUtxoDetector::~RemoteUtxoDetector() {
	this->list->deleteElements();
	delete this->list;
}

void RemoteUtxoDetector::add(const RemoteUtxoHeight *utxo) noexcept {
	this->list->addElement(new RemoteUtxoHeight(*utxo));
}

bool RemoteUtxoDetector::isRemoteUtxoUsed(const UtxoId *utxoId) const noexcept {
	RemoteUtxoData* data = this->finalizedUtxoRepo->getUtxo(utxoId); __STP(data);
	uint64_t height = data != nullptr ? data->getHeight() : 0;

	bool bl = (data != nullptr && height > this->expiredHeight) ? true : hasUtxo(utxoId);
	return bl;
}

void RemoteUtxoDetector::consumeRemoteUtxo(const UtxoId *utxoId, uint64_t height) {
	RemoteUtxoHeight* data = new RemoteUtxoHeight(height, utxoId);
	this->list->addElement(data);
}

bool RemoteUtxoDetector::hasUtxo(const UtxoId *utxoId) const noexcept {
	bool ret = false;

	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		RemoteUtxoHeight* data = this->list->get(i);
		const UtxoId* id = data->getUtxoId();

		if(utxoId->equals(id)){
			ret = true;
			break;
		}
	}

	return ret;
}

} /* namespace codablecash */
