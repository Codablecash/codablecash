/*
 * SmartcontractExecContextRegistory.cpp
 *
 *  Created on: Nov 28, 2025
 *      Author: iizuka
 */

#include "smartcontract_executor/SmartcontractExecContextRegistory.h"
#include "smartcontract_executor/SmartcontractExecContextKeyFactory.h"
#include "smartcontract_executor/SmartcontractExecContextDataFactory.h"
#include "smartcontract_executor/SmartcontractExecContextKey.h"
#include "smartcontract_executor/SmartcontractExecContextData.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"

#include "base_io/File.h"

#include "random_access_file/DiskCacheManager.h"

#include "btree/Btree.h"
#include "btree/BtreeConfig.h"


namespace codablecash {

SmartcontractExecContextRegistory::SmartcontractExecContextRegistory(const File* baseDir) {
	this->baseDir = baseDir->get(DIR_NAME);

	this->cacheManager = new DiskCacheManager();
	this->dataStore = nullptr;
}

SmartcontractExecContextRegistory::~SmartcontractExecContextRegistory() {
	close();

	delete this->baseDir;
	delete this->cacheManager;
}

void SmartcontractExecContextRegistory::createBlankDatabase() {
	UnicodeString fileName(MAIN_DATA);

	SmartcontractExecContextKeyFactory* keyFactory = new SmartcontractExecContextKeyFactory(); __STP(keyFactory);
	SmartcontractExecContextDataFactory* dataFactory = new SmartcontractExecContextDataFactory(); __STP(dataFactory);

	Btree btree(this->baseDir, &fileName, this->cacheManager, keyFactory, dataFactory);

	BtreeConfig config;
	config.nodeNumber = 8;
	config.defaultSize = 1024;
	config.blockSize = 32;
	btree.create(&config);
}

void SmartcontractExecContextRegistory::open() {
	UnicodeString fileName(MAIN_DATA);

	SmartcontractExecContextKeyFactory* keyFactory = new SmartcontractExecContextKeyFactory(); __STP(keyFactory);
	SmartcontractExecContextDataFactory* dataFactory = new SmartcontractExecContextDataFactory(); __STP(dataFactory);

	this->dataStore = new Btree(this->baseDir, &fileName, this->cacheManager, keyFactory, dataFactory);
	BtreeOpenConfig opconf;
	opconf.numDataBuffer = 256;
	opconf.numNodeBuffer = 512;
	this->dataStore->open(&opconf);
}

void SmartcontractExecContextRegistory::close() {
	if(this->dataStore != nullptr){
		this->dataStore->close();
		delete this->dataStore;
		this->dataStore = nullptr;
	}
}

void SmartcontractExecContextRegistory::addExecContext(const SmartcontractExecContextKey *key,
		const SmartcontractExecContextData *data) {
	this->dataStore->putData(key, data);

}

SmartcontractExecContextData* SmartcontractExecContextRegistory::getExecContext(const CdbDatabaseSessionId *trxId) {
	SmartcontractExecContextKey key;
	key.setTransactionId(trxId);

	return getExecContext(&key);
}

SmartcontractExecContextData* SmartcontractExecContextRegistory::getExecContext(const SmartcontractExecContextKey *key) {
	IBlockObject* object = this->dataStore->findByKey(key); __STP(object);

	return object != nullptr ? dynamic_cast<SmartcontractExecContextData*>(__STP_MV(object)) : nullptr;
}

} /* namespace codablecash */
