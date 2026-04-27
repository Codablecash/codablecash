/*
 * AbstractExecutableModule.h
 *
 *  Created on: Nov 11, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_INSTANCE_ABSTRACTEXECUTABLEMODULEINSTANCE_H_
#define SMARTCONTRACT_INSTANCE_ABSTRACTEXECUTABLEMODULEINSTANCE_H_
#include <cstdint>

namespace alinous {
class UnicodeString;
class File;
class SmartContract;
class VirtualMachine;
class CompileError;
class VmClassInstance;
class CompilationUnit;
class ByteBuffer;
class AnalyzedClass;
class AbstractFunctionExtArguments;
class FunctionArguments;
class AbstractExtObject;
class ClassDeclare;
}
using namespace alinous;

#include "base/ArrayList.h"

namespace codablecash {

class SoftwareVersion;
class ModularInstanceConfig;
class DependencyConfig;
class InstanceDependencyHandler;
class ModularSmartcontractInstance;
class ModuleInstanceClassLoader;
class SmartcontractExecResult;
class InstanceDependencyContext;
class ModularProxyObjectInstanceFactory;
class CdbDatabaseSessionId;


class AbstractExecutableModuleInstance {
public:
	static constexpr const uint8_t TYPE_EXEC = 1;
	static constexpr const uint8_t TYPE_LIBRARY= 2;

	static const constexpr wchar_t* DB_DIR{L"main_data"};
	static const constexpr wchar_t* UNDO_DIR{L"undo_data"};

	explicit AbstractExecutableModuleInstance(uint8_t kind);
	virtual ~AbstractExecutableModuleInstance();

	void setName(const UnicodeString* name) noexcept;
	void setProjectRelativePath(const UnicodeString* path) noexcept;
	void addSourceFolders(const UnicodeString* folder);
	void setSoftwareVersion(const SoftwareVersion* v);
	void setInstanceConfig(const ModularInstanceConfig* config);
	void setDependencyConfig(const DependencyConfig* config);

	virtual void loadCompilantUnits(const File* projectBaseDir) = 0;
	bool hasCompileError() const noexcept;

	void setMainInstance();
	bool createMainInstance();
	bool interpretInitializer();

	void resetRootReference();

	bool initBeforeAnalyze();
	bool preAnalyze();
	bool preAnalyzeGenerics();
	bool analyzeType();
	bool analyzeMetadata();
	bool analyzeFinal();

	bool checkDirectAccess();

	bool loadDependency(ModularSmartcontractInstance* parent);
	bool preAnalyzeDependency();
	bool analyzeTypeDependency();
	bool analyzeDependency();

	void loadLibExportInterfaceUnits(ModuleInstanceClassLoader* clazzloader);

	VirtualMachine* getVM() const noexcept {
		return this->vm;
	}

	// binary
	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	static AbstractExecutableModuleInstance* createFromBinary(ByteBuffer* in);

	// database
	void setDatabaseDir(const File* baseDir);
	void createDatabase();
	void loadDatabase();
	void newSession(const CdbDatabaseSessionId *sessionId);
	const CdbDatabaseSessionId* getDatabaseSessionId() const noexcept;
	void undoCurrentSession();

	// invoke method
	SmartcontractExecResult* invokeMainObjectMethodProxy(UnicodeString *methodName, ArrayList<AbstractFunctionExtArguments>* args);
	AbstractExtObject* invokeMainObjectMethodProxy(VirtualMachine* callerVm, UnicodeString *methodName, FunctionArguments* args);

	const UnicodeString* getName() const noexcept {
		return this->name;
	}
	const ModularInstanceConfig* getInstanceConfig() const noexcept {
		return this->instanceConfig;
	}

	// modular
	bool generateInterModularCommunicationClasses();
	bool preAnalyzeInterModularCommunicationClasses();
	bool analyzeTypeRefInterModularCommunicationClasses();
	bool analyzeInterModularCommunicationClasses();

	void invokeModularProxyListnerMethod();

protected:
	void callModuleDetectedMethod(ClassDeclare* proxyClass, VmClassInstance* proxyInstance, const UnicodeString* ifname);
	void fireListner(InstanceDependencyContext* dctx);
	void doFireListner(InstanceDependencyContext *dctx, const UnicodeString* exportInterface, const ModularInstanceConfig* instanceConfig);

	void generateLibExport(UnicodeString* mainFqn, ArrayList<UnicodeString>* libExport, InstanceDependencyContext* dctx);

	bool hasInterface(const ArrayList<AnalyzedClass>* implememtsList, AnalyzedClass* aclazz);
	void resetContract();
	void parseSourceFolders(const File *projectBaseDir);
	void scanSourceFolder(File* baseDir, const UnicodeString* folder, const File *projectBaseDir);

	void scanFiles(File* folder, const File *projectBaseDir);
	void addCompilantUnit(File* file, File* base, const File *projectBaseDir);

protected:
	uint8_t kind;
	UnicodeString* name;
	UnicodeString* projectRelativePath;

	ArrayList<UnicodeString>* sourceFolders;
	SoftwareVersion* version;

	ModularInstanceConfig* instanceConfig;
	DependencyConfig* dependencyConfig;

protected:
	// SmartContract* contract;
	VirtualMachine* vm;

	VmClassInstance* mainInst;

	const ArrayList<CompileError>* compile_errors;

	InstanceDependencyHandler* dependencyHandler;
	ModularProxyObjectInstanceFactory* factory;

	// database
	File* dbDir;
	File* undodbDir;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_INSTANCE_ABSTRACTEXECUTABLEMODULEINSTANCE_H_ */
