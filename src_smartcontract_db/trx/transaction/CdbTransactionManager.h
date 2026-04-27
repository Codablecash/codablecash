/*
 * CdbTransactionManager.h
 *
 *  Created on: 2020/05/09
 *      Author: iizuka
 */

#ifndef TRANSACTION_CDBTRANSACTIONMANAGER_H_
#define TRANSACTION_CDBTRANSACTIONMANAGER_H_
#include <cstdint>

#include "schema_table/schema/ISchemaUptateListner.h"

namespace alinous {
class File;
}
using namespace alinous;

namespace codablecash {

class CdbTransaction;
class CodableDatabase;
class SchemaObjectIdPublisher;
class RecordObjectIdPublisher;
class CreateTableLog;
class DropTableLog;
class AbstractTransactionLog;
class InsertLog;
class UpdateLog;
class DeleteLog;
class Schema;
class CdbStorageManager;
class ReadLockHandle;
class WriteLockHandle;
class AbstractAlterCommandLog;
class CdbDatabaseSession;
class CdbDatabaseSessionId;

class CdbTransactionManager : public ISchemaUptateListner {
public:
	CdbTransactionManager(CodableDatabase* db);
	virtual ~CdbTransactionManager();

	void loadSession(const File* undoDir);
	void newSession(const CdbDatabaseSessionId* sessionId) noexcept;
	void undoSession(const CdbDatabaseSessionId* sessionId) noexcept;
	CdbDatabaseSession* getDatabaseSession() const noexcept {
		return this->dbsession;
	}

	virtual void schemaLoaded(SchemaManager* sc);
	virtual void onCreateTable(SchemaManager* mgr, const CdbTable* table);
	virtual void onDropTable(SchemaManager* mgr, const CdbTable* table);

	virtual void onAlterModify(CdbTransactionManager* trxManager, SchemaManager* mgr, const CdbTable* table, const ColumnModifyContext* ctx);
	virtual void onDropPrimaryKey(CdbTransactionManager* trxManager, SchemaManager* mgr, const CdbTable* table, const CdbTableIndex* primaryKey);
	virtual void onAddPrimaryKey(CdbTransactionManager* trxManager, SchemaManager* mgr, const CdbTable* table, const CdbTableIndex* primaryKey);
	virtual void onAlterTableRenameTable(CdbTransactionManager* trxManager, SchemaManager* mgr, const CdbTable* table, TableRenameContext* ctx);
	virtual void onAddColumn(CdbTransactionManager* trxManager, SchemaManager* mgr, const CdbTable* table, const CdbTableColumn* newColumn, const CdbTableIndex* newUniqueIndex);
	virtual void onDropColumn(CdbTransactionManager* trxManager, SchemaManager* mgr, const CdbTable* table, const CdbTableColumn* removalColumn, const ArrayList<CdbTableIndex>* removalIndexes);
	virtual void onAddIndex(CdbTransactionManager* trxManager, SchemaManager* mgr, const CdbTable* table, const CdbTableIndex* newIndex);
	virtual void onDropIndex(CdbTransactionManager* trxManager, SchemaManager* mgr, const CdbTable* table, const CdbTableIndex* removalIndex);

	CdbTransaction* newTransaction(uint64_t transactionId);

	SchemaObjectIdPublisher* getSchemaObjectIdPublisher() const noexcept;
	RecordObjectIdPublisher* getRecordObjectIdPublisher() const noexcept;

	void commitCreateTable(CreateTableLog* cmd);
	void commitDropTable(DropTableLog* cmd);
	void commitAlterTable(AbstractAlterCommandLog* cmd);

	void commitInsert(InsertLog* cmd);
	void commitUpdate(UpdateLog* cmd, CdbTransaction* trx, CdbDatabaseSession* session);
	void commitDelete(DeleteLog*cmd, CdbTransaction* trx, CdbDatabaseSession* session);

	Schema* getSchema(const UnicodeString* name) const noexcept;

	CdbStorageManager* getStorageManager() const noexcept;
	SchemaManager* getSchemaManager() const noexcept;

	ReadLockHandle* databaseReadLock();
	WriteLockHandle* databaseWriteLock();

	CodableDatabase* getDb() const noexcept {
		return db;
	}

	CdbDatabaseSession* getCdbDatabaseSession() const noexcept {
		return this->dbsession;
	}

private:
	void handleAlterTableAddIndex(AbstractAlterCommandLog* cmd);
	void handleAlterTableAddColumn(AbstractAlterCommandLog* cmd);
	void handleAlterTableDropIndex(AbstractAlterCommandLog* cmd);
	void handleAlterTableDropColumn(AbstractAlterCommandLog* cmd);
	void handleAlterTableAddPrimaryKey(AbstractAlterCommandLog* cmd);
	void handleAlterTableDropPrimaryKey(AbstractAlterCommandLog* cmd);
	void handleAlterTableModify(AbstractAlterCommandLog* cmd);
	void handleAlterTableRenameColumn(AbstractAlterCommandLog* cmd);
	void handleAlterTableRenameTable(AbstractAlterCommandLog* cmd);

private:
	CodableDatabase* db;
	SchemaManager* schemaManager;
	SchemaObjectIdPublisher* schemaIdPublisher;
	RecordObjectIdPublisher* recordObjectIdPublisher;

	CdbDatabaseSession* dbsession;
};

} /* namespace codablecash */

#endif /* TRANSACTION_CDBTRANSACTIONMANAGER_H_ */
