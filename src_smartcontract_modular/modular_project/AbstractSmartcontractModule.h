/*
 * AbstractModule.h
 *
 *  Created on: Sep 19, 2025
 *      Author: iizuka
 */

#ifndef MODULAR_PROJECT_ABSTRACTSMARTCONTRACTMODULE_H_
#define MODULAR_PROJECT_ABSTRACTSMARTCONTRACTMODULE_H_

#include "base/ArrayList.h"

namespace alinous {
class UnicodeString;
class File;
}
using namespace alinous;

namespace codablecash {

class SoftwareVersion;
class DependencyConfig;
class JsonObject;
class ModularInstanceConfig;
class AbstractExecutableModuleInstance;

class AbstractSmartcontractModule {
public:
	static constexpr const wchar_t* CONFIG_JSON = L"config.json";

	static constexpr const wchar_t* SOURCE_FOLDERS = L"sourceFolders";
	static constexpr const wchar_t* VERSION = L"version";
	static constexpr const wchar_t* INSTANCE = L"instance";
	static constexpr const wchar_t* DEPENDENCIES = L"dependencies";


	explicit AbstractSmartcontractModule(const UnicodeString* projectRelativePath);
	virtual ~AbstractSmartcontractModule();

	virtual void load(const File* modulePath) = 0;
	virtual AbstractExecutableModuleInstance* toInstance() const = 0;

protected:
	virtual void analyzeJsonObject(const JsonObject* object);
	void setupInstance(AbstractExecutableModuleInstance* inst) const;

protected:
	const UnicodeString* projectRelativePath;

	ArrayList<UnicodeString>* sourceFolders;
	SoftwareVersion* version;

	ModularInstanceConfig* instance;

	DependencyConfig* dependencies;
};

} /* namespace codablecash */

#endif /* MODULAR_PROJECT_ABSTRACTSMARTCONTRACTMODULE_H_ */
