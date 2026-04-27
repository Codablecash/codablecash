/*
 * InstanceIdIndex.h
 *
 *  Created on: Nov 28, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_INDEX_INSTANCEIDINDEX_H_
#define SMARTCONTRACT_EXECUTOR_INDEX_INSTANCEIDINDEX_H_

#include "base/ArrayList.h"

namespace alinous {
class UnicodeString;
class DiskCacheManager;
class Btree;
class BtreeScanner;
class File;
}
using namespace alinous;

namespace codablecash {

class InstanceIdIndexData;
class InstanceIdIndexKey;
class InstanceSessionContext;
class SmartcontractInstanceAddress;
class IInstanceSessionContextListner;

class InstanceIdIndex {
public:
	static const constexpr wchar_t* DIR_NAME{L"instanceid_index"};
	static const constexpr wchar_t* MAIN_DATA{L"main_data"};


	explicit InstanceIdIndex(const File* baseDir);
	virtual ~InstanceIdIndex();

	void createBlankDatabase();
	void open();
	void close();

	void put(const InstanceIdIndexKey* key, const InstanceIdIndexData* data);
	InstanceIdIndexData* findData(const InstanceIdIndexKey* key);

	void remove(const InstanceIdIndexKey* key);

	void findHeads(const SmartcontractInstanceAddress* address, IInstanceSessionContextListner* listner);

private:
	File* baseDir;
	DiskCacheManager* cacheManager;
	Btree* dataStore;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_INDEX_INSTANCEIDINDEX_H_ */
