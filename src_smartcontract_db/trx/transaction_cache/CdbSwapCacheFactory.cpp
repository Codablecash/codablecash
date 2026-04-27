/*
 * CdbSwapCacheFactory.cpp
 *
 *  Created on: 2020/09/24
 *      Author: iizuka
 */

#include "trx/transaction_cache/CdbSwapCacheFactory.h"
#include "trx/transaction_cache/SingleKeyOidCache.h"
#include "trx/transaction_cache/OidKeyRecordCache.h"
#include "trx/transaction_cache/GroupRecordCache.h"
#include "trx/transaction_cache/RecordScanIdentityCache.h"

#include "trx/transaction_cache_array/OidArrayCache.h"

#include "schema_table/record/table_record/CdbDataFactory.h"
#include "schema_table/record/table_record/CdbKeyFactory.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "base_io/File.h"

#include "scan_select/scan_condition/func_aggregate/CountSQLFunctionCache.h"


namespace codablecash {

const UnicodeString CdbSwapCacheFactory::PREFIX_SINGLE_KEY(L"singlekey_");
const UnicodeString CdbSwapCacheFactory::PREFIX_OID_KEY(L"oidkey_");
const UnicodeString CdbSwapCacheFactory::PREFIX_GROUP_KEY(L"groupkey_");
const UnicodeString CdbSwapCacheFactory::PREFIX_OID_ARRAY_KEY(L"oidarraykey_");
const UnicodeString CdbSwapCacheFactory::PREFIX_COUNT_SQL_FUNCTION_KEY(L"countsqlfunckey_");
const UnicodeString CdbSwapCacheFactory::PREFIX_RECORD_SCAN_IDENTITY_KEY(L"countsqlfunckey_");

CdbSwapCacheFactory::CdbSwapCacheFactory(const File* tmpdir, DiskCacheManager* diskCache)
		: SwapCacheFactory(tmpdir, diskCache, nullptr, nullptr) {
	this->dataFactory = new CdbDataFactory();
	this->keyFactory = new CdbKeyFactory();
}

CdbSwapCacheFactory::~CdbSwapCacheFactory() {
	delete this->dataFactory;
	delete this->keyFactory;
}

UnicodeString* CdbSwapCacheFactory::getName() noexcept {
	UnicodeString* name = new UnicodeString(CdbSwapCacheFactory::PREFIX_SINGLE_KEY);

	name->append((int64_t)this->serial++);

	return name;
}

SingleKeyOidCache* CdbSwapCacheFactory::createSingleKeyOidCache(int swappiness) {
	UnicodeString* name = getName(); __STP(name);

	SingleKeyOidCache* cache = new SingleKeyOidCache(this->baseDir, name, dynamic_cast<CdbKeyFactory*>(this->keyFactory),
			dynamic_cast<CdbDataFactory*>(this->dataFactory), this->diskCache);
	cache->setSwappiness(swappiness);
	cache->init(8);

	return cache;
}

OidKeyRecordCache* CdbSwapCacheFactory::createOidKeyRecordCache(int swappiness) {
	UnicodeString* name = new UnicodeString(CdbSwapCacheFactory::PREFIX_OID_KEY); __STP(name);
	name->append((int64_t)this->serial++);

	OidKeyRecordCache* cache = new OidKeyRecordCache(this->baseDir, name, dynamic_cast<CdbKeyFactory*>(this->keyFactory),
			dynamic_cast<CdbDataFactory*>(this->dataFactory), this->diskCache);
	cache->setSwappiness(swappiness);
	cache->init(8);

	return cache;
}

GroupRecordCache* CdbSwapCacheFactory::createGroupRecordCache(int swappiness) {
	UnicodeString* name = new UnicodeString(CdbSwapCacheFactory::PREFIX_GROUP_KEY); __STP(name);
	name->append((int64_t)this->serial++);

	GroupRecordCache* cache = new GroupRecordCache(this->baseDir, name, dynamic_cast<CdbKeyFactory*>(this->keyFactory),
			dynamic_cast<CdbDataFactory*>(this->dataFactory), this->diskCache);

	cache->setSwappiness(swappiness);
	cache->init(8);

	return cache;
}

OidArrayCache* CdbSwapCacheFactory::createOidArrayCache(int swappiness) {
	UnicodeString* name = new UnicodeString(CdbSwapCacheFactory::PREFIX_OID_ARRAY_KEY); __STP(name);
	name->append((int64_t)this->serial++);

	OidArrayCache* cache = new OidArrayCache();
	UnicodeString* dir = this->baseDir->getAbsolutePath(); __STP(dir);

	cache->init(dir, name, this->diskCache);

	return cache;
}

CountSQLFunctionCache* CdbSwapCacheFactory::createCountSQLFunctionCache(int swappiness) {
	UnicodeString* name = new UnicodeString(CdbSwapCacheFactory::PREFIX_COUNT_SQL_FUNCTION_KEY); __STP(name);
	name->append((int64_t)this->serial++);

	CountSQLFunctionCache* cache = new CountSQLFunctionCache(this->baseDir, name, dynamic_cast<CdbKeyFactory*>(this->keyFactory),
			dynamic_cast<CdbDataFactory*>(this->dataFactory), this->diskCache);

	cache->setSwappiness(swappiness);
	cache->init(8);

	return cache;
}

RecordScanIdentityCache* CdbSwapCacheFactory::createRecordScanIdentityCache(int swappiness) {
	UnicodeString* name = new UnicodeString(CdbSwapCacheFactory::PREFIX_RECORD_SCAN_IDENTITY_KEY); __STP(name);
	name->append((int64_t)this->serial++);

	RecordScanIdentityCache* cache = new RecordScanIdentityCache(this->baseDir, name, dynamic_cast<CdbKeyFactory*>(this->keyFactory),
			dynamic_cast<CdbDataFactory*>(this->dataFactory), this->diskCache);

	cache->setSwappiness(swappiness);
	cache->init(8);

	return cache;
}


} /* namespace codablecash */
