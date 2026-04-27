/*
 * CdbTransactionManager.cpp
 *
 *  Created on: 2020/05/09
 *      Author: iizuka
 */

#include "trx/transaction/CdbTransactionManager.h"
#include "trx/transaction/CdbTransaction.h"
#include "trx/transaction/SchemaObjectIdPublisher.h"
#include "trx/transaction/RecordObjectIdPublisher.h"
#include "trx/transaction/LocalOid2PublishedOidLog.h"

#include "trx/transaction_log/AbstractTransactionLog.h"
#include "trx/transaction_log/CreateTableLog.h"
#include "trx/transaction_log/InsertLog.h"
#include "trx/transaction_log/UpdateLog.h"

#include "engine/CodableDatabase.h"
#include "engine/CdbException.h"

#include "schema_table/schema/SchemaManager.h"
#include "schema_table/schema/Schema.h"

#include "schema_table/record/table_record/CdbTableIdentifier.h"
#include "schema_table/record/table_record/CdbRecord.h"

#include "schema_table/table/CdbTable.h"

#include "schema_table/table_store/CdbStorageManager.h"
#include "schema_table/table_store/TableStore.h"

#include "trx/transaction_log_alter/AbstractAlterCommandLog.h"
#include "trx/transaction_log_alter/AlterAddColumnCommandLog.h"
#include "trx/transaction_log_alter/AlterAddIndexCommandLog.h"
#include "trx/transaction_log_alter/AlterDropIndexCommandLog.h"
#include "trx/transaction_log_alter/AlterDropColumnCommandLog.h"
#include "trx/transaction_log_alter_modify/AlterAddPrimaryKeyCommandLog.h"
#include "trx/transaction_log_alter_modify/AlterDropPrimaryKeyCommandLog.h"
#include "trx/transaction_log_alter_modify/AlterModifyCommandLog.h"
#include "trx/transaction_log_alter_modify/AlterRenameTableCommandLog.h"
#include "trx/transaction_log_alter_modify/AlterRenameColumnCommandLog.h"

#include "engine/CdbOid.h"

#include "engine_lock/StackDbLockUnlocker.h"

#include "trx/transaction_log/DropTableLog.h"
#include "trx/transaction_log/DeleteLog.h"

#include "trx/session/base/CdbDatabaseSession.h"
#include "trx/session/base/UndoIds.h"

#include "trx/session/record/UndoUpdateRecordPart.h"
#include "trx/session/record/UndoDeleteRecordPart.h"

#include "trx/session/ddl/UndoCreateTableLog.h"
#include "trx/session/ddl/UndoDropTableLog.h"

#include "trx/session/base/UndoRegisterTransaction.h"

#include "lang_sql/sql_join_parts/TableIdentifier.h"

#include "base/StackRelease.h"


namespace codablecash {

CdbTransactionManager::CdbTransactionManager(CodableDatabase* db) {
	this->db = db;
	this->schemaIdPublisher = nullptr;
	this->recordObjectIdPublisher = nullptr;
	this->schemaManager = nullptr;


	this->dbsession = nullptr;
}

CdbTransactionManager::~CdbTransactionManager() {
	this->db = nullptr;
	delete this->schemaIdPublisher;
	this->schemaManager = nullptr;

	delete this->recordObjectIdPublisher;
	this->recordObjectIdPublisher = nullptr;

	delete this->dbsession;
}

void CdbTransactionManager::loadSession(const File *undoDir) {
	delete this->dbsession;
	this->dbsession = new CdbDatabaseSession(undoDir);
	this->dbsession->load();
}

void CdbTransactionManager::newSession(const CdbDatabaseSessionId *sessionId) noexcept {
	UndoIds undoLog;

	// save current session id as last one
	const CdbDatabaseSessionId* lastSessionId = this->dbsession->getSessionId();
	undoLog.setLastSessionId(lastSessionId);

	this->schemaManager->snapshotObjectIds(&undoLog);


	this->dbsession->newSession(sessionId);
	this->dbsession->addUndoLog(&undoLog);
}

void CdbTransactionManager::undoSession(const CdbDatabaseSessionId *sessionId) noexcept {
	this->dbsession->undoSession(sessionId, this);
}

void CdbTransactionManager::schemaLoaded(SchemaManager* sc) {
	delete this->schemaIdPublisher;
	this->schemaIdPublisher = new SchemaObjectIdPublisher(sc);

	delete this->recordObjectIdPublisher;
	this->recordObjectIdPublisher = new RecordObjectIdPublisher(sc);

	this->schemaManager = sc;
}

void CdbTransactionManager::onCreateTable(SchemaManager* mgr, const CdbTable* table) {
	// do nothing
}

void CdbTransactionManager::onDropTable(SchemaManager* mgr,	const CdbTable* table) {
	// do nothing
}

void CdbTransactionManager::onAlterModify(CdbTransactionManager* trxManager, SchemaManager* mgr,
		const CdbTable* table, const ColumnModifyContext* ctx) {
	// do nothing
}

void CdbTransactionManager::onDropPrimaryKey(CdbTransactionManager* trxManager, SchemaManager* mgr,
		const CdbTable* table, const CdbTableIndex* primaryKey) {
	// do nothing
}

void CdbTransactionManager::onAlterTableRenameTable(CdbTransactionManager* trxManager, SchemaManager* mgr,
		const CdbTable* table, TableRenameContext* ctx) {
	// do nothing
}

void CdbTransactionManager::onAddPrimaryKey(CdbTransactionManager* trxManager, SchemaManager* mgr,
		const CdbTable* table, const CdbTableIndex* primaryKey) {
	// do nothing
}

void CdbTransactionManager::onAddColumn(CdbTransactionManager* trxManager, SchemaManager* mgr,
		const CdbTable* table, const CdbTableColumn* newColumn,
		const CdbTableIndex* newUniqueIndex) {
	// do nothing
}

void CdbTransactionManager::onDropColumn(CdbTransactionManager* trxManager, SchemaManager* mgr,
		const CdbTable* table, const CdbTableColumn* removalColumn,
		const ArrayList<CdbTableIndex>* removalIndexes) {
	// do nothing
}

void CdbTransactionManager::onAddIndex(CdbTransactionManager* trxManager, SchemaManager* mgr,
		const CdbTable* table, const CdbTableIndex* newIndex) {
	// do nothing
}

void CdbTransactionManager::onDropIndex(CdbTransactionManager* trxManager, SchemaManager* mgr,
		const CdbTable* table, const CdbTableIndex* removalIndex) {
	// do nothing
}

CdbTransaction* CdbTransactionManager::newTransaction(uint64_t transactionId) {
	CdbTransaction* trx = new CdbTransaction(this, transactionId);

	return trx;
}

SchemaObjectIdPublisher* CdbTransactionManager::getSchemaObjectIdPublisher() const noexcept {
	return this->schemaIdPublisher;
}

RecordObjectIdPublisher* CdbTransactionManager::getRecordObjectIdPublisher() const noexcept {
	return this->recordObjectIdPublisher;
}

void CdbTransactionManager::commitCreateTable(CreateTableLog* cmd) {
	UndoRegisterTransaction undoTrx(this->dbsession->getUndoSpaceManager());
	UndoCreateTableLog undoLog;

	CdbTable* table = cmd->getTable();
	this->schemaManager->createTable(table);

	undoLog.setTableOid(table->getOid());

	this->dbsession->addcommittedCommand(cmd);
	this->dbsession->addUndoLog(&undoLog);
	undoTrx.logAdded();

	undoTrx.commit();
}

void CdbTransactionManager::commitDropTable(DropTableLog* cmd) {
	UndoRegisterTransaction undoTrx(this->dbsession->getUndoSpaceManager());

	const TableIdentifier* tableId = cmd->getTableId();

	{
		UndoDropTableLog undoLog;
		undoLog.setDbSession(this->dbsession);

		CdbTable* table = this->schemaManager->getTable(tableId, nullptr);

		undoLog.setOid(table->getOid());
		undoLog.setSchemaName(tableId->getSchema());
		undoLog.setName(tableId->getTableName());

		this->schemaManager->snapshot(&undoLog, this->db->getStorageManager());
		undoTrx.logAdded();
	}

	this->schemaManager->dropTable(tableId);

	this->dbsession->addcommittedCommand(cmd);
	undoTrx.commit();
}

void CdbTransactionManager::commitAlterTable(AbstractAlterCommandLog* cmd) {
	uint8_t type = cmd->getType();

	switch(type){
	case AbstractAlterCommandLog::TRX_ALTER_ADD_INDEX:
		handleAlterTableAddIndex(cmd);
		break;
	case AbstractAlterCommandLog::TRX_ALTER_ADD_COLUMN:
		handleAlterTableAddColumn(cmd);
		break;
	case AbstractAlterCommandLog::TRX_ALTER_DROP_INDEX:
		handleAlterTableDropIndex(cmd);
		break;
	case AbstractAlterCommandLog::TRX_ALTER_DROP_COLUMN:
		handleAlterTableDropColumn(cmd);
		break;
	case AbstractAlterCommandLog::TRX_ALTER_ADD_PRIMARY_KEY:
		handleAlterTableAddPrimaryKey(cmd);
		break;
	case AbstractAlterCommandLog::TRX_ALTER_DROP_PRIMARY_KEY:
		handleAlterTableDropPrimaryKey(cmd);
		break;
	case AbstractAlterCommandLog::TRX_ALTER_MODIFY:
		handleAlterTableModify(cmd);
		break;
	case AbstractAlterCommandLog::TRX_ALTER_RENAME_COLUMN:
		handleAlterTableRenameColumn(cmd);
		break;
	case AbstractAlterCommandLog::TRX_ALTER_RENAME_TABLE:
		handleAlterTableRenameTable(cmd);
		break;
	default:
		throw new CdbException(L"unknown command type.", __FILE__, __LINE__);
	}

	this->dbsession->addcommittedCommand(cmd);
}

void CdbTransactionManager::handleAlterTableAddIndex(AbstractAlterCommandLog* cmd) {
	AlterAddIndexCommandLog* cmdlog = dynamic_cast<AlterAddIndexCommandLog*>(cmd);
	this->schemaManager->handleAlterTableAddIndex(cmdlog, this);
}

void CdbTransactionManager::handleAlterTableAddColumn(AbstractAlterCommandLog* cmd) {
	AlterAddColumnCommandLog* cmdlog = dynamic_cast<AlterAddColumnCommandLog*>(cmd);
	this->schemaManager->handleAlterTableAddColumn(cmdlog, this);
}

void CdbTransactionManager::handleAlterTableDropIndex(AbstractAlterCommandLog* cmd) {
	AlterDropIndexCommandLog* cmdlog = dynamic_cast<AlterDropIndexCommandLog*>(cmd);
	this->schemaManager->handleAlterTableDropIndex(cmdlog, this);
}

void CdbTransactionManager::handleAlterTableDropColumn(AbstractAlterCommandLog* cmd) {
	AlterDropColumnCommandLog* cmdlog = dynamic_cast<AlterDropColumnCommandLog*>(cmd);
	this->schemaManager->handleAlterTableDropColumn(cmdlog, this);
}

void CdbTransactionManager::handleAlterTableAddPrimaryKey(AbstractAlterCommandLog* cmd) {
	AlterAddPrimaryKeyCommandLog* cmdlog = dynamic_cast<AlterAddPrimaryKeyCommandLog*>(cmd);
	this->schemaManager->handleAlterTableAddPrimaryKey(cmdlog, this);
}

void CdbTransactionManager::handleAlterTableDropPrimaryKey(AbstractAlterCommandLog* cmd) {
	AlterDropPrimaryKeyCommandLog* cmdlog = dynamic_cast<AlterDropPrimaryKeyCommandLog*>(cmd);
	this->schemaManager->handleAlterTableDropPrimaryKey(cmdlog, this);
}

void CdbTransactionManager::handleAlterTableModify(AbstractAlterCommandLog* cmd) {
	AlterModifyCommandLog* cmdlog = dynamic_cast<AlterModifyCommandLog*>(cmd);
	this->schemaManager->handleAlterTableModify(cmdlog, this);
}

void CdbTransactionManager::handleAlterTableRenameColumn(AbstractAlterCommandLog* cmd) {
	AlterRenameColumnCommandLog* cmdlog = dynamic_cast<AlterRenameColumnCommandLog*>(cmd);
	this->schemaManager->handleAlterTableRenameColumn(cmdlog, this);
}

void CdbTransactionManager::handleAlterTableRenameTable(AbstractAlterCommandLog* cmd) {
	AlterRenameTableCommandLog* cmdlog = dynamic_cast<AlterRenameTableCommandLog*>(cmd);
	this->schemaManager->handleAlterTableRenameTable(cmdlog, this);
}

void CdbTransactionManager::commitInsert(InsertLog* cmd) {
	CdbTableIdentifier* tableIdentifier = cmd->getCdbTableIdentifier();
	const UnicodeString* schemaName = tableIdentifier->getSchema();
	const UnicodeString* tableName = tableIdentifier->getTable();

	Schema* schema = this->schemaManager->getSchema(schemaName);
	if(schema == nullptr){
		throw new CdbException(L"Schema to INSERT TABLE does not exists.", __FILE__, __LINE__);
	}

	CdbTable* table = schema->getCdbTableByName(tableName);
	if(table == nullptr){
		throw new CdbException(L"Table to INSERT TABLE does not exists.", __FILE__, __LINE__);
	}

	const CdbOid* oid = table->getOid();

	CdbStorageManager* storeManager = this->db->getStorageManager();
	TableStore* store = storeManager->getTableStore(oid);
	assert(store != nullptr);

	// table level lock
	{
		AbstractLockHandle* lockH = store->writeLock();
		StackDbLockUnlocker unlocker(lockH);

		const ArrayList<CdbRecord>* records = cmd->getRecords();

		int maxLoop = records->size();
		for(int i = 0; i != maxLoop; ++i){
			CdbRecord* rec = records->get(i);
			store->validateRecord(rec);
		}


		for(int i = 0; i != maxLoop; ++i){
			CdbRecord* rec = records->get(i);

			store->insert(rec);
		}

		this->dbsession->addcommittedCommand(cmd);
	}
}

void CdbTransactionManager::commitUpdate(UpdateLog* cmd, CdbTransaction* trx, CdbDatabaseSession* session) {
	CdbTableIdentifier* tableIdentifier = cmd->getCdbTableIdentifier();
	const UnicodeString* schemaName = tableIdentifier->getSchema();
	const UnicodeString* tableName = tableIdentifier->getTable();

	Schema* schema = this->schemaManager->getSchema(schemaName);
	if(schema == nullptr){
		throw new CdbException(L"Schema to UPDATE TABLE does not exists.", __FILE__, __LINE__);
	}

	CdbTable* table = schema->getCdbTableByName(tableName);
	if(table == nullptr){
		throw new CdbException(L"Table to UPDATE TABLE does not exists.", __FILE__, __LINE__);
	}

	const CdbOid* oid = table->getOid();

	CdbStorageManager* storeManager = this->db->getStorageManager();
	TableStore* store = storeManager->getTableStore(oid);
	assert(store != nullptr);

	LocalOid2PublishedOidLog* oidLog = trx->getOidPublishLog();

	// table level lock
	{
		AbstractLockHandle* lockH = store->writeLock();
		StackDbLockUnlocker unlocker(lockH);

		const ArrayList<CdbRecord>* list = cmd->getRecords();

		int maxLoop = list->size();
		for(int i = 0; i != maxLoop; ++i){
			CdbRecord* record = list->get(i);
			const CdbOid* oid = record->getOid();

			if(oid->isLocal()){
				const CdbOid* publishedOid = oidLog->getOid(oid);
				record->setOid(publishedOid);
				oid = record->getOid();
			}

			store->validateRecord(record);

			// make undo log before update
			{
				CdbRecord* lastRecord = store->findRecord(oid); __STP(lastRecord);
				if(lastRecord != nullptr){
					UndoUpdateRecordPart partLog;
					partLog.setRecord(lastRecord);
					session->addUndoLogPart(&partLog);
				}
			}

			// UPDATE
			store->remove(oid);
			store->insert(record);
		}
	}

	this->dbsession->addcommittedCommand(cmd);
}

void CdbTransactionManager::commitDelete(DeleteLog* cmd, CdbTransaction* trx, CdbDatabaseSession* session) {
	CdbTableIdentifier* tableIdentifier = cmd->getCdbTableIdentifier();
	const UnicodeString* schemaName = tableIdentifier->getSchema();
	const UnicodeString* tableName = tableIdentifier->getTable();

	Schema* schema = this->schemaManager->getSchema(schemaName);
	if(schema == nullptr){
		throw new CdbException(L"Schema to DELETE TABLE does not exists.", __FILE__, __LINE__);
	}

	CdbTable* table = schema->getCdbTableByName(tableName);
	if(table == nullptr){
		throw new CdbException(L"Table to DELETE TABLE does not exists.", __FILE__, __LINE__);
	}

	const CdbOid* oid = table->getOid();

	CdbStorageManager* storeManager = this->db->getStorageManager();
	TableStore* store = storeManager->getTableStore(oid);
	assert(store != nullptr);

	LocalOid2PublishedOidLog* oidLog = trx->getOidPublishLog();

	{
		AbstractLockHandle* lockH = store->writeLock();
		StackDbLockUnlocker unlocker(lockH);

		const ArrayList<CdbOid>* list = cmd->getOidsList();

		int maxLoop = list->size();
		for(int i = 0; i != maxLoop; ++i){
			const CdbOid* oid = list->get(i);

			if(oid->isLocal()){
				oid = oidLog->getOid(oid);
			}

			// undo part
			{
				CdbRecord* lastRecord = store->findRecord(oid); __STP(lastRecord);
				if(lastRecord != nullptr){
					UndoDeleteRecordPart logPart;
					logPart.setRecord(lastRecord);
					session->addUndoLogPart(&logPart);
				}
			}

			// DELETE
			store->remove(oid);
		}
	}

	this->dbsession->addcommittedCommand(cmd);
}

Schema* CdbTransactionManager::getSchema(const UnicodeString* name) const noexcept {
	return this->schemaManager->getSchema(name);
}

CdbStorageManager* CdbTransactionManager::getStorageManager() const noexcept {
	return this->db->getStorageManager();
}

SchemaManager* CdbTransactionManager::getSchemaManager() const noexcept {
	return this->schemaManager;
}

ReadLockHandle* CdbTransactionManager::databaseReadLock() {
	return this->db->databaseReadLock();
}

WriteLockHandle* CdbTransactionManager::databaseWriteLock() {
	return this->db->databaseWriteLock();
}

} /* namespace codablecash */
