/*
 * RemoteUtxoRepository.cpp
 *
 *  Created on: Jul 7, 2026
 *      Author: iizuka
 */

#include "bc_status_cache_data/RemoteUtxoRepository.h"
#include "bc_status_cache_data/RemoteUtxoHistory.h"
#include "bc_status_cache_data/RemoteUtxoHeightIndex.h"
#include "bc_status_cache_data/RemoteUtxoHeightIndexData.h"
#include "bc_status_cache_data/RemoteUtxoData.h"

#include "base_io/File.h"

#include "base/StackRelease.h"

#include "btree/BtreeReverseScanner.h"

#include "btreekey/ULongKey.h"

#include "bc_trx/UtxoId.h"


namespace codablecash {

RemoteUtxoRepository::RemoteUtxoRepository(const File* baseDir) {
	this->baseDir = baseDir->get(REMOTE_UTXO_ID);

	this->utxoHistory = new RemoteUtxoHistory(this->baseDir);
	this->heightIndex = new RemoteUtxoHeightIndex(this->baseDir);
}

RemoteUtxoRepository::~RemoteUtxoRepository() {
	close();

	delete this->baseDir;
}

void RemoteUtxoRepository::initBlank() {
	this->utxoHistory->initBlank();
	this->heightIndex->initBlank();
}

void RemoteUtxoRepository::open() {
	this->utxoHistory->open();
	this->heightIndex->open();
}

void RemoteUtxoRepository::close() {
	if(this->utxoHistory != nullptr){
		this->utxoHistory->close();
		delete this->utxoHistory, this->utxoHistory = nullptr;
	}
	if(this->heightIndex != nullptr){
		this->heightIndex->close();
		delete this->heightIndex, this->heightIndex = nullptr;
	}
}

void RemoteUtxoRepository::addUtxo(uint64_t height, const UtxoId *utxoId) noexcept {
	this->utxoHistory->add(utxoId, height);
	this->heightIndex->addUtxo(height, utxoId);
}

RemoteUtxoData* RemoteUtxoRepository::getUtxo(const UtxoId *utxoId) {
	RemoteUtxoData* data = this->utxoHistory->getData(utxoId);
	return data;
}

void RemoteUtxoRepository::clean(uint64_t cleanHeight) {
	ArrayList<UtxoId> removeUtxo;
	removeUtxo.setDeleteOnExit();

	BtreeReverseScanner* scanner = this->heightIndex->getReverseScanner(); __STP(scanner);

	ULongKey key(cleanHeight);
	scanner->begin(&key);

	uint64_t cleanStartHeight = cleanHeight;
	while(scanner->hasPrevious()){
		const IBlockObject* obj = scanner->previous();
		const RemoteUtxoHeightIndexData* data = dynamic_cast<const RemoteUtxoHeightIndexData*>(obj);

		ArrayList<UtxoId>* utxoList = data->getList();
		int maxLoop = utxoList->size();
		for(int i = 0; i != maxLoop; ++i){
			const UtxoId* utxoId = utxoList->get(i);
			removeUtxo.addElement(dynamic_cast<UtxoId*>(utxoId->copyData()));
		}

		const AbstractBtreeKey* key = scanner->previousKey();
		const ULongKey* ukey = dynamic_cast<const ULongKey*>(key);
		cleanStartHeight = ukey->getValue();
	}

	// remove height index
	{
		for(uint64_t i = cleanStartHeight; i <= cleanHeight; i++){
			this->heightIndex->remove(i);
		}
	}

	// remove utxo
	{
		int maxLoop = removeUtxo.size();
		for(int i = 0; i != maxLoop; ++i){
			UtxoId* utxoId = removeUtxo.get(i);

			this->utxoHistory->remove(utxoId);
		}
	}
}

} /* namespace codablecash */
