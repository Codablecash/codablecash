/*
 * CodableDatabase.cpp
 *
 *  Created on: 2020/05/09
 *      Author: iizuka
 */

#include "engine/CodableDatabase.h"
#include "engine/CdbException.h"
#include "engine/CdbLocalCacheManager.h"

#include "trx/transaction/CdbTransactionManager.h"
#include "trx/transaction/SchemaObjectIdPublisher.h"
#include "trx/transaction/CdbTransaction.h"

#include "base_io/File.h"

#include "schema_table/schema/SchemaManager.h"

#include "schema_table/table_store/CdbStorageManager.h"

#include "engine_lock/DatabaseLevelLock.h"

#include "base/StackRelease.h"

#include "schema_table/record/table_record_local/LocalOidFactory.h"

#include "scan_select/scan_condition/func/SQLFunctionRegistory.h"

#include "engine_lock/ReadLockHandle.h"

#include "engine_lock/StackDbLockUnlocker.h"

#include "trx/session/base/UndoSpaceManager.h"

#include "engine_lock/WriteLockHandle.h"
namespace codablecash {

CodableDatabase::CodableDatabase() {
	this->trxManager = new CdbTransactionManager(this);
	this->schema = nullptr;
	this->loadedFile = nullptr;
	this->store = nullptr;
	this->dbLevelLock = new DatabaseLevelLock();
	this->localCacheManager = nullptr;
	this->localOidFactory = nullptr;
	this->functionRegistory = new SQLFunctionRegistory();
}

CodableDatabase::~CodableDatabase() {
	closeDatabase();

	delete this->trxManager;
	this->trxManager = nullptr;

	delete this->dbLevelLock;
	this->dbLevelLock = nullptr;

	delete this->functionRegistory;
	this->functionRegistory = nullptr;
}

void CodableDatabase::createBlankDatabase(File *dbdir, const File* undoDir) {
	dbdir->deleteDir();

	dbdir->mkdirs();

	SchemaManager::createBlankSchema(dbdir);

	// undo
	UndoSpaceManager::createUndoSpace(undoDir);
}

void CodableDatabase::createDatabase(File* dbdir, const File* undoDir) {
	dbdir->deleteDir();

	dbdir->mkdirs();

	// schema
	SchemaManager::createSchema(&SchemaManager::PUBLIC, dbdir);

	// undo
	UndoSpaceManager::createUndoSpace(undoDir);
}

bool CodableDatabase::loadDatabase(const File* dbdir, const File* undoDir) {
	File* tmpdir = dbdir->get(L"temp"); __STP(tmpdir);

	return loadDatabase(dbdir, tmpdir, undoDir);
}

bool CodableDatabase::loadDatabase(const File* dbdir, const File* tmpdir, const File* undoDir) {
	if(!dbdir->exists() || !dbdir->isDirectory() || !undoDir->exists() || !undoDir->isDirectory()){
		return false;
	}

	if(!tmpdir->exists()){
		tmpdir->mkdirs();
	}


	this->localOidFactory = new LocalOidFactory();
	this->localCacheManager = new CdbLocalCacheManager(tmpdir, this->localOidFactory);
	this->localCacheManager->init();

	this->store = new CdbStorageManager();

	this->schema = new SchemaManager(this);
	this->schema->addSchemaUpdateListner(this->trxManager);
	this->schema->addSchemaUpdateListner(this->store);

	try{
		this->schema->loadSchema(dbdir);
		this->trxManager->loadSession(undoDir);
	}
	catch(Exception* e){
		delete e;
		return false;
	}

	this->loadedFile = new File(*dbdir);

	return true;
}

void CodableDatabase::closeDatabase() noexcept {
	if(this->loadedFile != nullptr || this->schema != nullptr){
		delete this->store;
		this->store = nullptr;

		delete this->schema;
		this->schema = nullptr;

		delete this->loadedFile;
		this->loadedFile = nullptr;

		delete this->localCacheManager;
		this->localCacheManager = nullptr;

		delete this->localOidFactory;
		this->localOidFactory = nullptr;
	}
}

void CodableDatabase::newSession(const CdbDatabaseSessionId *sessionId) noexcept {
	WriteLockHandle* lockH = databaseWriteLock();
	StackDbLockUnlocker unlocker(lockH);

	this->trxManager->newSession(sessionId);
}

void CodableDatabase::undoSession(const CdbDatabaseSessionId *sessionId) {
	WriteLockHandle* lockH = databaseWriteLock();
	StackDbLockUnlocker unlocker(lockH);

	this->trxManager->undoSession(sessionId);
}

CdbTransaction* CodableDatabase::newTransaction() {
	checkDatabaseLoaded();

	uint64_t transactionId = this->schema->newTransactionId();

	return this->trxManager->newTransaction(transactionId);
}

CdbStorageManager* CodableDatabase::getStorageManager() const noexcept {
	return this->store;
}

SchemaManager* CodableDatabase::getSchemaManager() const noexcept {
	return this->schema;
}

uint64_t CodableDatabase::getSchemaObjectVersionId() const noexcept {
	SchemaObjectIdPublisher* schemaIdPublisher = this->trxManager->getSchemaObjectIdPublisher();

	return schemaIdPublisher->getSchemaObjectVersionId();
}

void CodableDatabase::checkDatabaseLoaded() const {
	if(this->loadedFile == nullptr){
		throw new CdbException(__FILE__, __LINE__);
	}
}

/*Schema* CodableDatabase::getSchema() const noexcept {
	return this->schema;
}*/

ReadLockHandle* CodableDatabase::databaseReadLock() {
	return this->dbLevelLock->readLock();
}

WriteLockHandle* CodableDatabase::databaseWriteLock() {
	return this->dbLevelLock->writeLock();
}

void CodableDatabase::snapshot(ISnapshotHandler *handler) {
	ReadLockHandle* lockH = databaseReadLock();
	StackDbLockUnlocker unclocker(lockH);

	this->schema->snapshotObjectIds(handler);

	this->schema->snapshot(handler, this->store);
}

void CodableDatabase::__testCheckIndexes() {
	this->store->__testCheckIndexes();
}

void CodableDatabase::__testCheckBinaryEquals(CodableDatabase *otherDb) {
	this->schema->__testCheckEqualsObjectIds(otherDb->schema);
	this->schema->__testCheckEquals(this->store, otherDb->schema, otherDb->store);
}

} /* namespace alinous */
