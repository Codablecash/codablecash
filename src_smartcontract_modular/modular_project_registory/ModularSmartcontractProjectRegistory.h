/*
 * ModularSmartcontractProjectRegistory.h
 *
 *  Created on: Nov 10, 2025
 *      Author: iizuka
 */

#ifndef MODULAR_PROJECT_REGISTORY_MODULARSMARTCONTRACTPROJECTREGISTORY_H_
#define MODULAR_PROJECT_REGISTORY_MODULARSMARTCONTRACTPROJECTREGISTORY_H_

namespace alinous {
class UnicodeString;
class DiskCacheManager;
class Btree;
class BtreeScanner;
class File;
}
using namespace alinous;

namespace codablecash {

class ProjectIdKey;
class SmartcontractProjectData;

class ModularSmartcontractProjectRegistory {
public:
	static const constexpr wchar_t* DIR_NAME{L"smartcontract_projects"};
	static const constexpr wchar_t* MAIN_DATA{L"main_data"};

	explicit ModularSmartcontractProjectRegistory(const File* baseDir);
	virtual ~ModularSmartcontractProjectRegistory();

	void createBlankDatabase();
	void open();
	void close();

	void put(const ProjectIdKey* key, const SmartcontractProjectData* data);
	SmartcontractProjectData* findProjectById(const ProjectIdKey* key);

private:
	File* baseDir;
	DiskCacheManager* cacheManager;
	Btree* dataStore;
};

} /* namespace codablecash */

#endif /* MODULAR_PROJECT_REGISTORY_MODULARSMARTCONTRACTPROJECTREGISTORY_H_ */
