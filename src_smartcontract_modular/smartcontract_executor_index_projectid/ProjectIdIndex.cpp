/*
 * ProjectIdIndex.cpp
 *
 *  Created on: Jan 29, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_index_projectid/ProjectIdIndex.h"
#include "smartcontract_executor_index_projectid/ProjectIdIndexKeyFactory.h"
#include "smartcontract_executor_index_projectid/ProjectIndexDataFactory.h"
#include "smartcontract_executor_index_projectid/ProjectIdIndexKey.h"
#include "smartcontract_executor_index_projectid/ProjectIndexData.h"
#include "smartcontract_executor_index_projectid/SmartcontractVersionKey.h"

#include "base_io/File.h"

#include "random_access_file/DiskCacheManager.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "btree/Btree.h"
#include "btree/BtreeConfig.h"

#include "btreekey/InfinityKey.h"

#include "transaction/SmartcontractModuleAddress.h"

#include "btree/BtreeReverseScanner.h"

#include "modular_project/SmartcontractProjectId.h"


namespace codablecash {

ProjectIdIndex::ProjectIdIndex(const File* baseDir) {
	this->baseDir = baseDir->get(DIR_NAME);

	this->cacheManager = new DiskCacheManager();
	this->dataStore = nullptr;
}

ProjectIdIndex::~ProjectIdIndex() {
	delete this->baseDir;
	delete this->cacheManager;
}

void ProjectIdIndex::createBlankDatabase() {
	UnicodeString fileName(MAIN_DATA);

	ProjectIdIndexKeyFactory* keyFactory = new ProjectIdIndexKeyFactory(); __STP(keyFactory);
	ProjectIndexDataFactory* dataFactory = new ProjectIndexDataFactory(); __STP(dataFactory);

	Btree btree(this->baseDir, &fileName, this->cacheManager, keyFactory, dataFactory);

	BtreeConfig config;
	config.nodeNumber = 8;
	config.defaultSize = 1024;
	config.blockSize = 32;
	btree.create(&config);
}

void ProjectIdIndex::open() {
	UnicodeString fileName(MAIN_DATA);

	ProjectIdIndexKeyFactory* keyFactory = new ProjectIdIndexKeyFactory(); __STP(keyFactory);
	ProjectIndexDataFactory* dataFactory = new ProjectIndexDataFactory(); __STP(dataFactory);

	this->dataStore = new Btree(this->baseDir, &fileName, this->cacheManager, keyFactory, dataFactory);
	BtreeOpenConfig opconf;
	opconf.numDataBuffer = 256;
	opconf.numNodeBuffer = 512;
	this->dataStore->open(&opconf);
}

void ProjectIdIndex::close() {
	if(this->dataStore != nullptr){
		this->dataStore->close();
		delete this->dataStore;
		this->dataStore = nullptr;
	}
}

void ProjectIdIndex::add(const ProjectIdIndexKey *key, const ProjectIndexData *data) {
	this->dataStore->putData(key, data);
}

SmartcontractProjectId* ProjectIdIndex::getLatestProjectId(const SmartcontractModuleAddress *moduleAddress) {
	SmartcontractProjectId* ret = nullptr;

	InfinityKey ikey;
	ProjectIdIndexKey key(moduleAddress, &ikey);

	BtreeReverseScanner* scanner = this->dataStore->getReverseScanner(); __STP(scanner);
	scanner->begin(&key);

	while(scanner->hasPrevious()){
		const IBlockObject* obj = scanner->previous();
		const AbstractBtreeKey* abkey = scanner->previousKey();

		const ProjectIdIndexKey* vkey = dynamic_cast<const ProjectIdIndexKey*>(abkey);
		const ProjectIndexData* data = dynamic_cast<const ProjectIndexData*>(obj);

		SmartcontractModuleAddress* address = vkey->getSmartcontractModuleAddress();
		if(address->compareTo(moduleAddress) == 0){
			const SmartcontractProjectId* projectId = data->getProjectId();
			ret = dynamic_cast<SmartcontractProjectId*>(projectId->copyData());
			break;
		}
	}

	return ret;
}

} /* namespace codablecash */
