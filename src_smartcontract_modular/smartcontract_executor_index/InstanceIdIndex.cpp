/*
 * InstanceIdIndex.cpp
 *
 *  Created on: Nov 28, 2025
 *      Author: iizuka
 */

#include "smartcontract_executor_index/InstanceIdIndex.h"
#include "smartcontract_executor_index/InstanceIdIndexDataFactory.h"
#include "smartcontract_executor_index/InstanceIdIndexKey.h"
#include "smartcontract_executor_index/InstanceIdIndexKeyFactory.h"
#include "smartcontract_executor_index/InstanceIdIndexData.h"
#include "smartcontract_executor_index/IInstanceSessionContextListner.h"

#include "random_access_file/DiskCacheManager.h"

#include "btree/Btree.h"
#include "btree/BtreeConfig.h"
#include "btree/BtreeReverseScanner.h"

#include "base/StackRelease.h"
#include "base/Long.h"

#include "base_io/File.h"

#include "transaction/SmartcontractInstanceAddress.h"


namespace codablecash {

InstanceIdIndex::InstanceIdIndex(const File* baseDir) {
	this->baseDir = baseDir->get(DIR_NAME);

	this->cacheManager = new DiskCacheManager();
	this->dataStore = nullptr;
}

InstanceIdIndex::~InstanceIdIndex() {
	close();

	delete this->baseDir;
	delete this->cacheManager;
}

void InstanceIdIndex::createBlankDatabase() {
	UnicodeString fileName(MAIN_DATA);

	InstanceIdIndexKeyFactory* keyFactory = new InstanceIdIndexKeyFactory(); __STP(keyFactory);
	InstanceIdIndexDataFactory* dataFactory = new InstanceIdIndexDataFactory(); __STP(dataFactory);

	Btree btree(this->baseDir, &fileName, this->cacheManager, keyFactory, dataFactory);

	BtreeConfig config;
	config.nodeNumber = 8;
	config.defaultSize = 1024;
	config.blockSize = 32;
	btree.create(&config);
}

void InstanceIdIndex::open() {
	UnicodeString fileName(MAIN_DATA);

	InstanceIdIndexKeyFactory* keyFactory = new InstanceIdIndexKeyFactory(); __STP(keyFactory);
	InstanceIdIndexDataFactory* dataFactory = new InstanceIdIndexDataFactory(); __STP(dataFactory);

	this->dataStore = new Btree(this->baseDir, &fileName, this->cacheManager, keyFactory, dataFactory);
	BtreeOpenConfig opconf;
	opconf.numDataBuffer = 256;
	opconf.numNodeBuffer = 512;
	this->dataStore->open(&opconf);
}

void InstanceIdIndex::close() {
	if(this->dataStore != nullptr){
		this->dataStore->close();
		delete this->dataStore;
		this->dataStore = nullptr;
	}
}

void InstanceIdIndex::put(const InstanceIdIndexKey *key, const InstanceIdIndexData *data) {
	this->dataStore->putData(key, data);
}

InstanceIdIndexData* InstanceIdIndex::findData(const InstanceIdIndexKey *key) {
	IBlockObject* obj = this->dataStore->findByKey(key);

	return obj != nullptr ? dynamic_cast<InstanceIdIndexData*>(obj) : nullptr;
}

void InstanceIdIndex::remove(const InstanceIdIndexKey *key) {
	this->dataStore->remove(key);
}

void InstanceIdIndex::findHeads(const SmartcontractInstanceAddress *address, IInstanceSessionContextListner* listner) {
	InstanceIdIndexKey key(address, Long::MAX_UINT64);

	BtreeReverseScanner* scanner = this->dataStore->getReverseScanner(); __STP(scanner);

	scanner->begin(&key);

	while(scanner->hasPrevious()){
		const IBlockObject* obj = scanner->previous();
		const AbstractBtreeKey* abkey = scanner->previousKey();

		const InstanceIdIndexData* data = dynamic_cast<const InstanceIdIndexData*>(obj);
		const InstanceIdIndexKey* thisKey = dynamic_cast<const InstanceIdIndexKey*>(abkey);

		SmartcontractInstanceAddress* instAddtess = thisKey->getInstanceAddress();
		if(instAddtess->compareTo(address) != 0){
			break;
		}

		int maxLoop = data->size();
		for(int i = 0; i != maxLoop; ++i){
			const InstanceSessionContext* context = data->get(i);
			listner->visit(context);
		}
	}
}

} /* namespace codablecash */
