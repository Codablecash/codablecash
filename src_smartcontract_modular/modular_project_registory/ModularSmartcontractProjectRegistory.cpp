/*
 * ModularSmartcontractProjectRegistory.cpp
 *
 *  Created on: Nov 10, 2025
 *      Author: iizuka
 */

#include "modular_project_registory/ModularSmartcontractProjectRegistory.h"
#include "modular_project_registory/ProjectIdKeyFactory.h"
#include "modular_project_registory/SmartcontractProjectDataFactory.h"
#include "modular_project_registory/ProjectIdKey.h"
#include "modular_project_registory/SmartcontractProjectData.h"

#include "random_access_file/DiskCacheManager.h"

#include "btree/Btree.h"
#include "btree/BtreeConfig.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "base_io/File.h"


namespace codablecash {

ModularSmartcontractProjectRegistory::ModularSmartcontractProjectRegistory(const File* baseDir) {
	this->baseDir = baseDir->get(DIR_NAME);

	this->cacheManager = new DiskCacheManager();
	this->dataStore = nullptr;
}

ModularSmartcontractProjectRegistory::~ModularSmartcontractProjectRegistory() {
	close();

	delete this->baseDir;
	delete this->cacheManager;
}

void ModularSmartcontractProjectRegistory::createBlankDatabase() {
	UnicodeString fileName(MAIN_DATA);

	ProjectIdKeyFactory* keyFactory = new ProjectIdKeyFactory(); __STP(keyFactory);
	SmartcontractProjectDataFactory* dataFactory = new SmartcontractProjectDataFactory(); __STP(dataFactory);

	Btree btree(this->baseDir, &fileName, this->cacheManager, keyFactory, dataFactory);

	BtreeConfig config;
	config.nodeNumber = 8;
	config.defaultSize = 1024;
	config.blockSize = 32;
	btree.create(&config);
}

void ModularSmartcontractProjectRegistory::open() {
	UnicodeString fileName(MAIN_DATA);

	ProjectIdKeyFactory* keyFactory = new ProjectIdKeyFactory(); __STP(keyFactory);
	SmartcontractProjectDataFactory* dataFactory = new SmartcontractProjectDataFactory(); __STP(dataFactory);

	this->dataStore = new Btree(this->baseDir, &fileName, this->cacheManager, keyFactory, dataFactory);
	BtreeOpenConfig opconf;
	opconf.numDataBuffer = 256;
	opconf.numNodeBuffer = 512;
	this->dataStore->open(&opconf);
}

void ModularSmartcontractProjectRegistory::close() {
	if(this->dataStore != nullptr){
		this->dataStore->close();
		delete this->dataStore;
		this->dataStore = nullptr;
	}
}

void ModularSmartcontractProjectRegistory::put(const ProjectIdKey *key, const SmartcontractProjectData *data) {
	this->dataStore->putData(key, data);
}

SmartcontractProjectData* ModularSmartcontractProjectRegistory::findProjectById(const ProjectIdKey *key) {
	IBlockObject* obj = this->dataStore->findByKey(key);

	return dynamic_cast<SmartcontractProjectData*>(obj);
}

} /* namespace codablecash */
