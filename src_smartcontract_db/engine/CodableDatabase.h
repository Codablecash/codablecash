/*
 * CodableDatabase.h
 *
 *  Created on: 2020/05/09
 *      Author: iizuka
 */

#ifndef ENGINE_CODABLEDATABASE_H_
#define ENGINE_CODABLEDATABASE_H_

#include <cstdint>

namespace alinous {
class File;
class UnicodeString;
}
using namespace alinous;

namespace codablecash {
class CdbTransactionManager;
class SchemaManager;
class CdbTransaction;
class CdbStorageManager;
class ReadLockHandle;
class WriteLockHandle;
class DatabaseLevelLock;
class CdbLocalCacheManager;
class LocalOidFactory;
class SQLFunctionRegistory;
class ISnapshotHandler;
class CdbDatabaseSessionId;

class CodableDatabase {
public:
	CodableDatabase();
	virtual ~CodableDatabase();

	void createBlankDatabase(File* dbdir, const File* undoDir);
	void createDatabase(File* dbdir, const File* undoDir);
	bool loadDatabase(const File* dbdir, const File* undoDir);
	bool loadDatabase(const File* dbdir, const File* tmpdir, const File* undoDir);
	void closeDatabase() noexcept;

	void newSession(const CdbDatabaseSessionId* sessionId) noexcept;
	CdbTransaction* newTransaction();

	void undoSession(const CdbDatabaseSessionId* sessionId);

	CdbStorageManager* getStorageManager() const noexcept;
	SchemaManager* getSchemaManager() const noexcept;

	uint64_t getSchemaObjectVersionId() const noexcept;

	ReadLockHandle* databaseReadLock();
	WriteLockHandle* databaseWriteLock();

	CdbTransactionManager* getTransactionxManager() const noexcept {
		return trxManager;
	}

	CdbLocalCacheManager* getLocalCacheManager() const noexcept {
		return this->localCacheManager;
	}

	LocalOidFactory* getLocalOidFactory() const noexcept {
		return localOidFactory;
	}

	SQLFunctionRegistory* getSQLFunctionRegistory() const noexcept {
		return this->functionRegistory;
	}

	void snapshot(ISnapshotHandler* handler);


	void __testCheckIndexes();
	void __testCheckBinaryEquals(CodableDatabase* otherDb);

private:
	void checkDatabaseLoaded() const;

private:
	CdbTransactionManager* trxManager;
	SchemaManager* schema;
	CdbStorageManager* store;

	LocalOidFactory* localOidFactory;
	CdbLocalCacheManager* localCacheManager;

	DatabaseLevelLock* dbLevelLock;

	File* loadedFile;

	SQLFunctionRegistory* functionRegistory;
};

} /* namespace alinous */

#endif /* ENGINE_CODABLEDATABASE_H_ */
