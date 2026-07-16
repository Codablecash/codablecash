/*
 * RemoteUtxoHeightIndex.cpp
 *
 *  Created on: Jul 7, 2026
 *      Author: iizuka
 */

#include "bc_status_cache_data/RemoteUtxoHeightIndex.h"
#include "bc_status_cache_data/RemoteUtxoHeightIndexDataFactory.h"
#include "bc_status_cache_data/RemoteUtxoHeightIndexData.h"

#include "base/StackRelease.h"

#include "base_io/File.h"

#include "random_access_file/DiskCacheManager.h"

#include "btree/Btree.h"
#include "btree/BtreeConfig.h"

#include "btreekey/BtreeKeyFactory.h"
#include "btreekey/ULongKey.h"


namespace codablecash {

RemoteUtxoHeightIndex::RemoteUtxoHeightIndex(const File* baseDir) {
	this->baseDir = new File(*baseDir);

	this->cacheManager = new DiskCacheManager();
	this->btree = nullptr;
}

RemoteUtxoHeightIndex::~RemoteUtxoHeightIndex() {
	close();

	delete this->baseDir;
	delete this->cacheManager;
}

void RemoteUtxoHeightIndex::initBlank() {
	UnicodeString fileName(HISTORY_BASE_DIR);

	BtreeKeyFactory* keyFactory = new BtreeKeyFactory(); __STP(keyFactory);
	RemoteUtxoHeightIndexDataFactory* dataFactory = new RemoteUtxoHeightIndexDataFactory(); __STP(dataFactory);

	Btree btree(this->baseDir, &fileName, this->cacheManager, keyFactory, dataFactory);
	BtreeConfig config;
	config.nodeNumber = 8;
	config.defaultSize = 1024;
	config.blockSize = 32;
	btree.create(&config);
}

void RemoteUtxoHeightIndex::open() {
	UnicodeString fileName(HISTORY_BASE_DIR);

	BtreeKeyFactory* keyFactory = new BtreeKeyFactory(); __STP(keyFactory);
	RemoteUtxoHeightIndexDataFactory* dataFactory = new RemoteUtxoHeightIndexDataFactory(); __STP(dataFactory);

	this->btree = new Btree(this->baseDir, &fileName, this->cacheManager, keyFactory, dataFactory);

	BtreeOpenConfig opconf;
	opconf.numDataBuffer = 256;
	opconf.numNodeBuffer = 512;
	this->btree->open(&opconf);
}

void RemoteUtxoHeightIndex::close() {
	if(this->btree != nullptr){
		this->btree->close();
		delete this->btree, this->btree = nullptr;
	}
}

void RemoteUtxoHeightIndex::addUtxo(uint64_t height, const UtxoId *utxoId) {
	ULongKey key(height);
	RemoteUtxoHeightIndexData* data;
	data->add(utxoId);

	this->btree->putData(&key, data);
}

BtreeReverseScanner* RemoteUtxoHeightIndex::getReverseScanner() {
	return this->btree->getReverseScanner();
}

void RemoteUtxoHeightIndex::remove(uint64_t height) {
	ULongKey key(height);
	this->btree->remove(&key);
}

} /* namespace codablecash */
