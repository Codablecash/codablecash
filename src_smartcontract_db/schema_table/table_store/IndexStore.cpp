/*
 * IndexStore.cpp
 *
 *  Created on: 2020/05/16
 *      Author: iizuka
 */

#include "schema_table/table_store/IndexStore.h"

#include "schema_table/table/CdbTable.h"
#include "schema_table/table/CdbTableIndex.h"
#include "schema_table/table/CdbTableColumn.h"

#include "base/StackRelease.h"

#include "base_io/File.h"

#include "btree/Btree.h"
#include "btree/BtreeConfig.h"

#include "schema_table/table_store/CdbStorageManager.h"

#include "schema_table/record/table_record/CdbKeyFactory.h"
#include "schema_table/record/table_record/CdbDataFactory.h"
#include "schema_table/record/table_record/CdbRecord.h"

#include "schema_table/record/table_record_key/CdbRecordKey.h"
#include "schema_table/record/table_record_key/AbstractCdbKey.h"

#include "scan_select/scan/IndexScanner.h"

#include "schema_table/record/table_record_value/CdbOidValueList.h"

#include "engine/CdbOid.h"

#include "btreekey/NullKey.h"
using alinous::File;

namespace codablecash {

IndexStore::IndexStore(DiskCacheManager* cacheManager, const File* tableDir, const CdbTable* table, const CdbTableIndex* index) {
	this->tableDir = new File(*tableDir);
	this->table = table;
	this->index = index;
	this->btree = nullptr;
	this->opened = false;
	this->cacheManager = cacheManager;
}

IndexStore::~IndexStore() {
	close();

	delete this->tableDir;
	this->table = nullptr;
	this->index = nullptr;
	this->cacheManager = nullptr;
}

void IndexStore::createStore(const File* tableDir, const CdbTable* table, const CdbTableIndex* index, DiskCacheManager* cacheManager) {
	const UnicodeString* name = index->getName();

	Btree btree(tableDir, name, cacheManager, &CdbStorageManager::keyFactory, &CdbStorageManager::dataFactory);

	BtreeConfig config;
	btree.create(&config);
}

void IndexStore::cleanupStore(const File* tableDir, const CdbTable* table, const CdbTableIndex* index) {
	const UnicodeString* name = index->getName();

	Btree::clearFiles(tableDir, name);
}

void IndexStore::reset() {
	close();

	cleanupStore(this->tableDir, this->table, this->index);
	createStore(this->tableDir, this->table, this->index, this->cacheManager);

	load();
}


void IndexStore::load() {
	const UnicodeString* name = this->index->getName();
	this->btree = new Btree(this->tableDir, name, this->cacheManager, &CdbStorageManager::keyFactory, &CdbStorageManager::dataFactory);

	BtreeOpenConfig opconf;
	this->btree->open(&opconf);
	this->opened = true;
}

void IndexStore::close() noexcept {
	if(this->btree != nullptr){
		if(this->opened){
			this->opened = false;
			this->btree->close();
		}

		delete this->btree;
		this->btree = nullptr;
	}
}

const CdbOid* IndexStore::getIndexOid() const noexcept {
	return this->index->getOid();
}

void IndexStore::insert(const CdbRecord* rec) {
	CdbRecordKey* key = new CdbRecordKey(); __STP(key);

	const ArrayList<CdbTableColumn>* list = this->index->getColumns();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		CdbTableColumn* col = list->get(i);
		int pos = col->getPosition();
		const AbstractCdbValue* value = rec->get(pos);

		if(value == nullptr){
			key->addKey(new NullKey());
			continue;
		}
		key->addKey(value->toKey());
	}


	CdbOidValueList* oidList = nullptr;
	IBlockObject* record = this->btree->findByKey(key);
	if(record != nullptr){
		oidList = dynamic_cast<CdbOidValueList*>(record);
		assert(oidList != nullptr);
	}
	else{
		oidList = new CdbOidValueList();
	}

	StackRelease<CdbOidValueList> __oidListRelease(oidList);
//	uint64_t newOid = rec->getOid()->getOidValue();

	oidList->addOid(rec->getOid()->copy());

	this->btree->putData(key, oidList);
}

IndexScanner* IndexStore::getScanner(CdbRecordKey* begin, bool beginEq, CdbRecordKey* end, bool endEq) {
	return new IndexScanner(begin, beginEq, end, endEq, this);
}

Btree* IndexStore::getBtree() const noexcept {
	return this->btree;
}

} /* namespace codablecash */
