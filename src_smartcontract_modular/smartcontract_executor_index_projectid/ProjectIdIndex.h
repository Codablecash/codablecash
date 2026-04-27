/*
 * ProjectIdIndex.h
 *
 *  Created on: Jan 29, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_INDEX_PROJECTID_PROJECTIDINDEX_H_
#define SMARTCONTRACT_EXECUTOR_INDEX_PROJECTID_PROJECTIDINDEX_H_

namespace alinous {
class UnicodeString;
class DiskCacheManager;
class Btree;
class BtreeScanner;
class File;
}
using namespace alinous;

namespace codablecash {

class ProjectIdIndexKey;
class ProjectIndexData;
class SmartcontractModuleAddress;
class SmartcontractProjectId;

class ProjectIdIndex {
public:
	static const constexpr wchar_t* DIR_NAME{L"projectid_index"};
	static const constexpr wchar_t* MAIN_DATA{L"main_data"};

	explicit ProjectIdIndex(const File* baseDir);
	virtual ~ProjectIdIndex();

	void createBlankDatabase();
	void open();
	void close();

	void add(const ProjectIdIndexKey* key, const ProjectIndexData* data);

	SmartcontractProjectId* getLatestProjectId(const SmartcontractModuleAddress* moduleAddress);

private:
	File* baseDir;
	DiskCacheManager* cacheManager;
	Btree* dataStore;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_INDEX_PROJECTID_PROJECTIDINDEX_H_ */
