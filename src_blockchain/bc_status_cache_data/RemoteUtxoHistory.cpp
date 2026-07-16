/*
 * RemoteUtxoHistory.cpp
 *
 *  Created on: Jul 7, 2026
 *      Author: iizuka
 */

#include "bc_status_cache_data/RemoteUtxoHistory.h"
#include "bc_status_cache_data/RemoteUtxoDataFactory.h"

#include "bc_base_utxo_index/UtxoIdKeyFactory.h"
#include "bc_base_utxo_index/UtxoIdKey.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "base_io/File.h"

#include "btree/Btree.h"
#include "btree/BtreeConfig.h"

#include "random_access_file/DiskCacheManager.h"

#include "bc_status_cache_data/RemoteUtxoData.h"

namespace codablecash {

RemoteUtxoHistory::RemoteUtxoHistory(const File* baseDir) {
	this->baseDir = new File(*baseDir);

	this->cacheManager = new DiskCacheManager();
	this->btree = nullptr;
}

RemoteUtxoHistory::~RemoteUtxoHistory() {
	close();

	delete baseDir;
	delete this->cacheManager;
}

void RemoteUtxoHistory::initBlank() {
	UnicodeString fileName(HISTORY_FILE);

	UtxoIdKeyFactory* keyFactory = new UtxoIdKeyFactory(); __STP(keyFactory);
	RemoteUtxoDataFactory* dataFactory = new RemoteUtxoDataFactory(); __STP(dataFactory);

	Btree btree(this->baseDir, &fileName, this->cacheManager, keyFactory, dataFactory);

	BtreeConfig config;
	config.nodeNumber = 8;
	config.defaultSize = 1024;
	config.blockSize = 32;
	btree.create(&config);
}

void RemoteUtxoHistory::open() {
	UnicodeString fileName(HISTORY_FILE);

	UtxoIdKeyFactory* keyFactory = new UtxoIdKeyFactory(); __STP(keyFactory);
	RemoteUtxoDataFactory* dataFactory = new RemoteUtxoDataFactory(); __STP(dataFactory);

	this->btree = new Btree(this->baseDir, &fileName, this->cacheManager, keyFactory, dataFactory);
	BtreeOpenConfig opconf;
	opconf.numDataBuffer = 256;
	opconf.numNodeBuffer = 512;
	this->btree->open(&opconf);
}

void RemoteUtxoHistory::close() {
	if(this->btree != nullptr){
		this->btree->close();
		delete this->btree, this->btree = nullptr;
	}
}

void RemoteUtxoHistory::add(const UtxoId *utxoId, uint64_t height) {
	UtxoIdKey key(utxoId);
	RemoteUtxoData data(utxoId, height);

	this->btree->putData(&key, &data);
}

void RemoteUtxoHistory::remove(const UtxoId *utxoId) {
	UtxoIdKey key(utxoId);
	this->btree->remove(&key);
}

RemoteUtxoData* RemoteUtxoHistory::getData(const UtxoId *utxoId) {
	UtxoIdKey key(utxoId);

	IBlockObject* obj = this->btree->findByKey(&key);
	return dynamic_cast<RemoteUtxoData*>(obj);
}

} /* namespace codablecash */
