/*
 * ModularSmartcontractProject.h
 *
 *  Created on: Sep 19, 2025
 *      Author: iizuka
 */

#ifndef MODULAR_PROJECT_MODULARSMARTCONTRACTPROJECT_H_
#define MODULAR_PROJECT_MODULARSMARTCONTRACTPROJECT_H_

#include "base/ArrayList.h"

namespace alinous {
class File;
}
using namespace alinous;


namespace codablecash {

class ExecutableSmartcontractModule;
class LibrarySmartcontractModule;
class ModularProjectConfig;
class ModularSmartcontractInstance;
class SmartcontractProjectId;

class ModularSmartcontractProject {
public:
	static constexpr const wchar_t* PROJECT_CONFIG_FILE_NAME{L"projectconfig.json"};
	static constexpr const wchar_t* CONFIG_FILE_NAME{L"config.json"};

	static constexpr const wchar_t* MODULE_DIR_NAME{L"modules"};

	explicit ModularSmartcontractProject(const File* baseDir);
	virtual ~ModularSmartcontractProject();

	void loadProject();

	ModularSmartcontractInstance* toInstance() const;

	SmartcontractProjectId* getProjectId() const;

	const File* getProjectBaseDir() const noexcept {
		return this->baseDir;
	}

private:
	void __loadProject();

	void loadExecutable(const UnicodeString* path);
	void loadLibrary(const File* libraryPath, const UnicodeString* libname);

private:
	File* baseDir;

	ExecutableSmartcontractModule* executableModule;
	ArrayList<LibrarySmartcontractModule>* libModules;

	ModularProjectConfig* config;
};

} /* namespace codablecash */

#endif /* MODULAR_PROJECT_MODULARSMARTCONTRACTPROJECT_H_ */
