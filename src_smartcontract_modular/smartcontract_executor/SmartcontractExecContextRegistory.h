/*
 * SmartcontractExecContextRegistory.h
 *
 *  Created on: Nov 28, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_SMARTCONTRACTEXECCONTEXTREGISTORY_H_
#define SMARTCONTRACT_EXECUTOR_SMARTCONTRACTEXECCONTEXTREGISTORY_H_

namespace alinous {
class UnicodeString;
class DiskCacheManager;
class Btree;
class BtreeScanner;
class File;
}
using namespace alinous;

namespace codablecash {

class SmartcontractExecContextKey;
class SmartcontractExecContextData;
class CdbDatabaseSessionId;

class SmartcontractExecContextRegistory {
public:
	static const constexpr wchar_t* DIR_NAME{L"contexts_registory"};
	static const constexpr wchar_t* MAIN_DATA{L"main_data"};

	explicit SmartcontractExecContextRegistory(const File* baseDir);
	virtual ~SmartcontractExecContextRegistory();

	void createBlankDatabase();
	void open();
	void close();

	void addExecContext(const SmartcontractExecContextKey* key, const SmartcontractExecContextData* data);

	SmartcontractExecContextData* getExecContext(const CdbDatabaseSessionId* trxId);
	SmartcontractExecContextData* getExecContext(const SmartcontractExecContextKey* key);

private:
	File* baseDir;
	DiskCacheManager* cacheManager;
	Btree* dataStore;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_SMARTCONTRACTEXECCONTEXTREGISTORY_H_ */
