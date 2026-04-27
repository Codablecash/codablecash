/*
 * ModularSmartcontractInstance.h
 *
 *  Created on: Sep 22, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_INSTANCE_MODULARSMARTCONTRACTINSTANCE_H_
#define SMARTCONTRACT_INSTANCE_MODULARSMARTCONTRACTINSTANCE_H_

#include "base/HashMap.h"
#include "base/ArrayList.h"

namespace alinous {
class ByteBuffer;
class File;
class VirtualMachine;
class AbstractFunctionExtArguments;
}
using namespace alinous;

namespace codablecash {

class ModularProjectConfig;
class ExecutableModuleInstance;
class LibraryExectableModuleInstance;
class SmartcontractProjectData;
class SmartcontractProjectId;
class SmartcontractInstanceAddress;
class SmartcontractExecResult;
class CdbDatabaseSessionId;

class ModularSmartcontractInstance {
public:
	static const constexpr wchar_t* DB_DIR{L"main_data"};
	static const constexpr wchar_t* UNDO_DIR{L"undo_data"};

	ModularSmartcontractInstance();
	virtual ~ModularSmartcontractInstance();

	void setModularProjectConfig(const ModularProjectConfig* config);
	void setExecutableModuleInstance(ExecutableModuleInstance* inst);
	void addLibraryModuleInstance(const UnicodeString* name, LibraryExectableModuleInstance* inst);

	bool libraryExists(const UnicodeString* name) const noexcept;
	LibraryExectableModuleInstance* getLibraryModuleInstance(const UnicodeString* modName) const noexcept;

	void loadCompilantUnits(const File* projectBaseDir);
	bool hasCompileError() const noexcept;

	bool preAnalyzeGoup();
	bool analyzeTypeRefGroup();
	bool analyzeMetadataGroup();
	bool analyzeGroup();

	bool checkDirectAccess();

	void setMainInstance();
	bool createMainInstance();
	bool interpretInitializer();

	void resetRootReference();

	// database
	void setDatabaseDir(const File* baseDir);
	void createDatabase();
	void loadDatabase();
	void cleanDbRoot();

	void newSession(const CdbDatabaseSessionId *sessionId);
	const CdbDatabaseSessionId* getDatabaseSessionId() const noexcept;
	void undoCurrentSession();

	// binary
	int binarySize() const;
	void toBinary(ByteBuffer* out) const;
	static ModularSmartcontractInstance* createFromBinary(ByteBuffer* in);

	// modular
	bool generateInterModularCommunicationClasses();
	bool preAnalyzeInterModularCommunicationClasses();
	bool analyzeTypeRefInterModularCommunicationClasses();
	bool analyzeInterModularCommunicationClasses();

	void invokeModularProxyListnerMethod();


	// create data
	SmartcontractProjectData* createData() const;
	SmartcontractProjectId* getProjectId() const;

	void setSmartcontractInstanceAddress(const SmartcontractInstanceAddress* address);
	const SmartcontractInstanceAddress* getSmartContractInstanceAddress() const noexcept {
		return this->instanceAddress;
	}

	// invoke method
	SmartcontractExecResult* invokeMainObjectMethod(UnicodeString *moduleName, UnicodeString *methodName, ArrayList<AbstractFunctionExtArguments>* args);


	ModularProjectConfig* getModularProjectConfig() const noexcept {
		return this->config;
	}

private:
	SmartcontractProjectId* __getProjectId(ByteBuffer* buff) const;

	ByteBuffer* createBinary() const;

	void initBeforeAnalyze();
	void preAnalyze();
	void preAnalyzeGenerics();
	void analyzeType();
	void analyzeFinal();

	// dependency
	void loadDependency();
	void preAnalyzeDependency();
	void analyzeTypeDependency();
	void analyzeDependency();

private:
	ModularProjectConfig* config;

	ExecutableModuleInstance* execModule;

	HashMap<UnicodeString, LibraryExectableModuleInstance>* libraries;
	ArrayList<LibraryExectableModuleInstance>* libArray;

	SmartcontractInstanceAddress* instanceAddress;

	File* dbRoot;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_INSTANCE_MODULARSMARTCONTRACTINSTANCE_H_ */
