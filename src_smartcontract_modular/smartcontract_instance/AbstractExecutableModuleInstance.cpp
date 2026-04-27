/*
 * AbstractExecutableModule.cpp
 *
 *  Created on: Nov 11, 2025
 *      Author: iizuka
 */
#include "smartcontract_instance/AbstractExecutableModuleInstance.h"
#include "smartcontract_instance/InstanceDependencyHandler.h"
#include "smartcontract_instance/ExecutableModuleInstance.h"
#include "smartcontract_instance/LibraryExectableModuleInstance.h"
#include "smartcontract_instance/ModuleInstanceClassLoader.h"
#include "smartcontract_instance/InstanceDependencyContext.h"
#include "smartcontract_instance/ClassNotFoundException.h"

#include "modular_project/ModularInstanceConfig.h"
#include "modular_project/DependencyConfig.h"
#include "modular_project/ModularConfigException.h"

#include "instance/instance_exception_class/VmExceptionInstance.h"

#include "smartcontract_executor/SmartcontractExecResult.h"

#include "inter_modular_access/ModularProxyObjectInstanceFactory.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"
#include "base/ArrayList.h"

#include "base_io/ByteBuffer.h"
#include "base_io/File.h"

#include "base_io_stream/exceptions.h"
#include "base_io_stream/FileInputStream.h"

#include "bc/ExceptionThrower.h"
#include "bc/SoftwareVersion.h"

#include "engine/CodableDatabase.h"

#include "engine/sc/SmartContract.h"
#include "engine/sc/CompilationUnit.h"

#include "engine/compiler/CompileError.h"

#include "engine/sc_analyze/AnalyzeContext.h"
#include "engine/sc_analyze/TypeResolver.h"
#include "engine/sc_analyze/AnalyzedType.h"
#include "engine/sc_analyze/AnalyzedClass.h"

#include "vm/VirtualMachine.h"
#include "vm/stack/StackPopper.h"

#include "json_object/AbstractJsonValue.h"

#include "vm/variable_access/FunctionArguments.h"

#include "ext_arguments/AbstractFunctionExtArguments.h"

#include "instance/instance_gc/GcManager.h"
#include "instance/AbstractVmInstance.h"
#include "instance/VmInstanceTypesConst.h"

#include "lang/sc_declare/ClassDeclare.h"
#include "lang/sc_declare/MethodDeclare.h"

#include "bc_base/BinaryUtils.h"

#include "ext_binary/ExtNullPtrObject.h"

#include "instance/instance_ref/ObjectReference.h"

#include "instance/reserved_classes/ReservedClassRegistory.h"
#include "instance/reserved_classes/object/ObjectClassDeclare.h"

#include "inter_modular_access/ModularProxyClassDeclare.h"

#include "instance/instance_gc/StackFloatingVariableHandler.h"

#include "instance/instance_string/VmStringInstance.h"

#include "modular_interfaces/ModularProxyListnerClassDeclare.h"
#include "modular_interfaces/ModuleDetectedMethodDeclare.h"

#include "instance/instance_exception/ExceptionInterrupt.h"

#include "trx/transaction/CdbTransactionManager.h"

#include "trx/session/base/CdbDatabaseSession.h"

#include "trx/session/base/CdbDatabaseSessionId.h"

namespace codablecash {

AbstractExecutableModuleInstance::AbstractExecutableModuleInstance(uint8_t kind) {
	this->kind = kind;
	this->name = nullptr;
	this->projectRelativePath = nullptr;

	this->sourceFolders = new ArrayList<UnicodeString>();
	this->version = nullptr;
	this->instanceConfig = nullptr;
	this->dependencyConfig = nullptr;

	this->vm = nullptr;
	this->compile_errors = nullptr;
	this->mainInst = nullptr;
	this->dependencyHandler = new InstanceDependencyHandler();
	this->factory = new ModularProxyObjectInstanceFactory();

	this->dbDir = nullptr;
	this->undodbDir = nullptr;
}

AbstractExecutableModuleInstance::~AbstractExecutableModuleInstance() {
	delete this->name;
	delete this->projectRelativePath;

	this->sourceFolders->deleteElements();
	delete this->sourceFolders;

	delete this->version;
	delete this->instanceConfig;
	delete this->dependencyConfig;

	vm->destroy();
	delete this->vm;

	this->mainInst = nullptr;
	delete this->dependencyHandler;
	delete this->factory;

	delete this->dbDir;
	delete this->undodbDir;
}

void AbstractExecutableModuleInstance::setName(const UnicodeString *name) noexcept {
	delete this->name;
	this->name = new UnicodeString(name);
}

void AbstractExecutableModuleInstance::setProjectRelativePath(const UnicodeString *path) noexcept {
	delete this->projectRelativePath;
	this->projectRelativePath = new UnicodeString(path);
}

void AbstractExecutableModuleInstance::addSourceFolders(const UnicodeString *folder) {
	this->sourceFolders->addElement(new UnicodeString(folder));
}

void AbstractExecutableModuleInstance::setSoftwareVersion(const SoftwareVersion *v) {
	delete this->version;
	this->version = new SoftwareVersion(*v);
}

void AbstractExecutableModuleInstance::setInstanceConfig(const ModularInstanceConfig *config) {
	delete this->instanceConfig;

	int cap = config->binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);
	config->toBinary(buff);

	buff->position(0);
	this->instanceConfig = ModularInstanceConfig::createFromBinary(buff);
}

void AbstractExecutableModuleInstance::setDependencyConfig(const DependencyConfig *config) {
	delete this->dependencyConfig;
	this->dependencyConfig = new DependencyConfig(*config);
}

void AbstractExecutableModuleInstance::resetContract() {
	delete this->vm;
	this->vm = new VirtualMachine(1024*1024);

	SmartContract* contract = new SmartContract();
	this->vm->loadSmartContract(contract);

	this->compile_errors = nullptr;
}

void AbstractExecutableModuleInstance::parseSourceFolders(const File *projectBaseDir) {
	File* baseDir = projectBaseDir->get(this->projectRelativePath); __STP(baseDir);

	ExceptionThrower<FileIOException>::throwExceptionIfCondition(!baseDir->exists(), L"Module base folder does not exists.", __FILE__, __LINE__);
	ExceptionThrower<FileIOException>::throwExceptionIfCondition(!baseDir->isDirectory(), L"Module base folder id not a directory.", __FILE__, __LINE__);

	int maxLoop = this->sourceFolders->size();
	for(int i = 0; i != maxLoop; ++i){
		const UnicodeString* folder = this->sourceFolders->get(i);

		scanSourceFolder(baseDir, folder, projectBaseDir);
	}
}

void AbstractExecutableModuleInstance::scanSourceFolder(File *baseDir, const UnicodeString *folder, const File *projectBaseDir) {
	File* dir = baseDir->get(folder); __STP(dir);
	scanFiles(dir, projectBaseDir);

	SmartContract* contract = this->vm->getSmartContract();
	this->compile_errors = contract->getCompileErrors();
}

void AbstractExecutableModuleInstance::scanFiles(File *folder, const File *projectBaseDir) {
	ArrayList<UnicodeString>* filesList = folder->list(); __STP(filesList);
	filesList->setDeleteOnExit();

	int maxLoop = filesList->size();
	for(int i = 0; i != maxLoop; ++i){
		UnicodeString* path = filesList->get(i);

		File* f = folder->get(path); __STP(f);
		if(f->isDirectory()){
			scanFiles(f, projectBaseDir);
		} else{
			addCompilantUnit(f, folder, projectBaseDir);
		}
	}
}

void AbstractExecutableModuleInstance::addCompilantUnit(File *file, File *base, const File *projectBaseDir) {
	int length = file->length();
	FileInputStream stream(file);

	SmartContract* contract = this->vm->getSmartContract();
	CompilationUnit* unit = contract->addCompilationUnit(&stream, length, base, file);

	const UnicodeString* projectPath = projectBaseDir->getAbsolutePath(); __STP(projectPath);
	const UnicodeString* filePath = file->getAbsolutePath(); __STP(filePath);

	UnicodeString* path = filePath->substring(projectPath->length()); __STP(path);

	unit->setProjectRelativePath(path);
}

bool AbstractExecutableModuleInstance::hasCompileError() const noexcept {
	return !this->compile_errors->isEmpty();
}


void AbstractExecutableModuleInstance::setMainInstance() {
	const UnicodeString* mainPackage = this->instanceConfig->getMainPackage();
	const UnicodeString* mainClass = this->instanceConfig->getMainClass();
	const UnicodeString* initializerMethod = this->instanceConfig->getInitializerMethod();

	SmartContract* contract = this->vm->getSmartContract();
	contract->setMainMethod(mainPackage, mainClass, initializerMethod);
}

bool AbstractExecutableModuleInstance::createMainInstance() {
	// init modular classes

	ArrayList<IInitializeCompilantUnitProvidor> exprogs;
	{
		ReservedClassRegistory* reservedReg = this->vm->getReservedClassRegistory();
		exprogs.addElement(reservedReg);
		exprogs.addElement(this->factory);

		ArrayList<InstanceDependencyContext>* list = this->dependencyHandler->getContextList();
		int maxLoop = list->size();
		for(int i = 0; i != maxLoop; ++i){
			InstanceDependencyContext* dctx = list->get(i);
			ModuleInstanceClassLoader* loader = dctx->getClassLoader();

			exprogs.addElement(loader);
		}
	}


	// init in the Smartcontract::createInstance(VirtualMachine* vm) by initialize(vm);
	this->mainInst = this->vm->createScInstance(&exprogs);
	ArrayList<Exception>& exceptions = this->vm->getExceptions();

	return !this->vm->hasError() && exceptions.isEmpty();
}

bool AbstractExecutableModuleInstance::interpretInitializer() {
	ArrayList<AbstractFunctionExtArguments> arguments;
	arguments.setDeleteOnExit();
	{
		const ArrayList<AbstractJsonValue>* argList = this->instanceConfig->getInitializerMethodArguments();
		int maxLoop = argList->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractJsonValue* jval = argList->get(i);

			AbstractFunctionExtArguments* arg = jval->toFunctionExtArgument();
			arguments.addElement(arg);
		}
	}

	const UnicodeString* initializerMethod = this->instanceConfig->getInitializerMethod();

	this->vm->interpretMainObjectMethod(initializerMethod, &arguments);

	ArrayList<Exception>& exceptions = this->vm->getExceptions();
	return !this->vm->hasError() && exceptions.isEmpty();
}

void AbstractExecutableModuleInstance::resetRootReference() {
	SmartContract* contract = this->vm->getSmartContract();

	if(contract->isInitialized()){
		this->mainInst = nullptr;

		contract->clearRootReference(this->vm);

		GcManager* gc = this->vm->getGc();
		gc->garbageCollect();

		assert(gc->isEmpty());
	}
}

bool AbstractExecutableModuleInstance::initBeforeAnalyze() {
	SmartContract* contract = this->vm->getSmartContract();
	contract->initBeforeAnalyze(this->vm);

	AnalyzeContext* actx = contract->getAnalyzeContext();
	return actx->hasError();
}

bool AbstractExecutableModuleInstance::preAnalyze() {
	SmartContract* contract = this->vm->getSmartContract();
	contract->preAnalyze(this->vm);

	AnalyzeContext* actx = contract->getAnalyzeContext();
	return actx->hasError();
}

bool AbstractExecutableModuleInstance::preAnalyzeGenerics() {
	SmartContract* contract = this->vm->getSmartContract();
	contract->preAnalyzeGenerics(this->vm);

	AnalyzeContext* actx = contract->getAnalyzeContext();
	return actx->hasError();
}

bool AbstractExecutableModuleInstance::analyzeType() {
	SmartContract* contract = this->vm->getSmartContract();
	contract->analyzeType(this->vm);

	AnalyzeContext* actx = contract->getAnalyzeContext();
	return actx->hasError();
}


bool AbstractExecutableModuleInstance::analyzeMetadata() {
	SmartContract* contract = this->vm->getSmartContract();
	contract->analyzeMetadata(this->vm);

	AnalyzeContext* actx = contract->getAnalyzeContext();
	return actx->hasError();
}

bool AbstractExecutableModuleInstance::analyzeFinal() {
	SmartContract* contract = this->vm->getSmartContract();
	contract->analyzeFinal(this->vm);

	AnalyzeContext* actx = contract->getAnalyzeContext();
	return actx->hasError();
}

bool AbstractExecutableModuleInstance::loadDependency(ModularSmartcontractInstance* parent) {
	SmartContract* contract = this->vm->getSmartContract();
	AnalyzeContext* actx = contract->getAnalyzeContext();

	if(this->dependencyConfig != nullptr){
		ArrayList<AbstractDependencyConfig>* list = this->dependencyConfig->getList();
		int maxLoop = list->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractDependencyConfig* config = list->get(i);

			this->dependencyHandler->registerDependentInstance(config, parent, actx);
		}
	}

	this->dependencyHandler->importExportedClasses(actx);

	return actx->hasError();
}

bool AbstractExecutableModuleInstance::preAnalyzeDependency() {
	SmartContract* contract = this->vm->getSmartContract();
	AnalyzeContext* actx = contract->getAnalyzeContext();

	this->dependencyHandler->preAnalyze(actx);

	return actx->hasError();
}

bool AbstractExecutableModuleInstance::analyzeTypeDependency() {
	SmartContract* contract = this->vm->getSmartContract();
	AnalyzeContext* actx = contract->getAnalyzeContext();

	this->dependencyHandler->analyzeType(actx);

	return actx->hasError();
}

bool AbstractExecutableModuleInstance::analyzeDependency() {
	SmartContract* contract = this->vm->getSmartContract();
	AnalyzeContext* actx = contract->getAnalyzeContext();

	this->dependencyHandler->analyze(actx);

	return actx->hasError();
}

void AbstractExecutableModuleInstance::loadLibExportInterfaceUnits(ModuleInstanceClassLoader* clazzloader) {
	SmartContract* contract = this->vm->getSmartContract();

	ArrayList<UnicodeString>* liblist = this->instanceConfig->getLibExport();
	int maxLoop = liblist->size();
	for(int i = 0; i != maxLoop; ++i){
		UnicodeString* fqn = liblist->get(i);

		// load class
		clazzloader->loadClass(fqn);

		const ClassDeclare* dec = clazzloader->getClassDeclare(fqn);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(dec == nullptr, L"Module does not exists.", __FILE__, __LINE__);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(!dec->isInterface(), L"libExport must be an interface, not a class.", __FILE__, __LINE__);
	}
}

int AbstractExecutableModuleInstance::binarySize() const {
	BinaryUtils::checkNotNull(this->name);
	BinaryUtils::checkNotNull(this->projectRelativePath);
	BinaryUtils::checkNotNull(this->version);
	BinaryUtils::checkNotNull(this->instanceConfig);


	int total = sizeof(uint8_t);
	total += BinaryUtils::stringSize(this->name);
	total += BinaryUtils::stringSize(this->projectRelativePath);

	int maxLoop = this->sourceFolders->size();
	total += sizeof(uint16_t);
	for(int i = 0; i != maxLoop; ++i){
		UnicodeString* folder = this->sourceFolders->get(i);
		total += BinaryUtils::stringSize(folder);
	}

	total += this->version->binarySize();
	total += this->instanceConfig->binarySize();

	total += sizeof(uint8_t);
	if(this->dependencyConfig != nullptr){
		total += this->dependencyConfig->binarySize();
	}

	// vm to binary
	SmartContract* contract = this->vm->getSmartContract();

	maxLoop = contract->getNumCompilationUnit();
	total += sizeof(uint32_t);

	for(int i = 0; i != maxLoop; ++i){
		CompilationUnit* unit = contract->getCompilationUnit(i);
		total += unit->binarySize();
	}

	return total;
}

void AbstractExecutableModuleInstance::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->name);
	BinaryUtils::checkNotNull(this->projectRelativePath);
	BinaryUtils::checkNotNull(this->version);
	BinaryUtils::checkNotNull(this->instanceConfig);

	out->put(this->kind);
	BinaryUtils::putString(out, this->name);
	BinaryUtils::putString(out, this->projectRelativePath);

	int maxLoop = this->sourceFolders->size();
	out->putShort(maxLoop);
	for(int i = 0; i != maxLoop; ++i){
		UnicodeString* folder = this->sourceFolders->get(i);
		BinaryUtils::putString(out, folder);
	}

	this->version->toBinary(out);
	this->instanceConfig->toBinary(out);

	uint8_t bl = (this->dependencyConfig != nullptr) ? 1 : 0;
	out->put(bl);
	if(bl > 0){
		this->dependencyConfig->toBinary(out);
	}

	// vm to binary
	SmartContract* contract = this->vm->getSmartContract();

	maxLoop = contract->getNumCompilationUnit();
	out->putInt(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		CompilationUnit* unit = contract->getCompilationUnit(i);
		unit->toBinary(out);
	}

}

void AbstractExecutableModuleInstance::fromBinary(ByteBuffer *in) {
	this->name = BinaryUtils::getString(in);
	this->projectRelativePath = BinaryUtils::getString(in);

	int maxLoop = in->getShort();
	for(int i = 0; i != maxLoop; ++i){
		UnicodeString* folder = BinaryUtils::getString(in);
		this->sourceFolders->addElement(folder);
	}

	this->version = SoftwareVersion::createFromBinary(in);
	this->instanceConfig = ModularInstanceConfig::createFromBinary(in);

	uint8_t bl = in->get();
	if(bl > 0){
		this->dependencyConfig = DependencyConfig::createFromBinary(in);
	}

	// vm
	resetContract();
	SmartContract* contract = this->vm->getSmartContract();

	maxLoop = in->getInt();
	for(int i = 0; i != maxLoop; ++i){
		CodeElement* element = CodeElement::createFromBinary(in); __STP(element);
		CompilationUnit* unit = dynamic_cast<CompilationUnit*>(element);
		BinaryUtils::checkNotNull(unit);

		__STP_MV(element);
		contract->addCompilationUnit(unit);
	}
}

AbstractExecutableModuleInstance* AbstractExecutableModuleInstance::createFromBinary(ByteBuffer *in) {
	AbstractExecutableModuleInstance* inst = nullptr;

	uint8_t kind = in->get();
	if(kind == TYPE_EXEC){
		inst = new ExecutableModuleInstance();
	}
	else{
		inst = new LibraryExectableModuleInstance();
	}
	__STP(inst);

	inst->fromBinary(in);

	return __STP_MV(inst);
}

void AbstractExecutableModuleInstance::setDatabaseDir(const File *baseDir) {
	File* dbRoot = baseDir->get(this->name); __STP(dbRoot);

	this->dbDir = dbRoot->get(DB_DIR);
	this->undodbDir = dbRoot->get(UNDO_DIR);
}

void AbstractExecutableModuleInstance::createDatabase() {
	CodableDatabase db;
	db.createDatabase(this->dbDir, this->undodbDir);
}

void AbstractExecutableModuleInstance::loadDatabase() {
	this->vm->loadDatabase(this->dbDir, this->undodbDir);
}

void AbstractExecutableModuleInstance::newSession(const CdbDatabaseSessionId *sessionId) {
	CodableDatabase* database = this->vm->getDb();

	database->newSession(sessionId);
}

const CdbDatabaseSessionId* AbstractExecutableModuleInstance::getDatabaseSessionId() const noexcept {
	CodableDatabase* database = this->vm->getDb();

	CdbTransactionManager* trxManager = database->getTransactionxManager();
	CdbDatabaseSession* session = trxManager->getCdbDatabaseSession();

	const CdbDatabaseSessionId* sessionId = session->getSessionId();
	return sessionId;
}

void AbstractExecutableModuleInstance::undoCurrentSession() {
	CodableDatabase* database = this->vm->getDb();

	CdbTransactionManager* trxManager = database->getTransactionxManager();
	CdbDatabaseSession* session = trxManager->getCdbDatabaseSession();

	const CdbDatabaseSessionId* sessionId = session->getSessionId();
	database->undoSession(sessionId);
}

bool AbstractExecutableModuleInstance::checkDirectAccess() {
	SmartContract* contract = this->vm->getSmartContract();
	AnalyzeContext* actx = contract->getAnalyzeContext();
	TypeResolver* resolver = actx->getTypeResolver();

	// main instance class
	UnicodeString* mainClassFqn = contract->getMainClassFqn(); __STP(mainClassFqn);
	ClassDeclare* dec = contract->getClassDeclareByFqn(mainClassFqn);
	ExceptionThrower<ClassNotFoundException>::throwExceptionIfCondition(dec == nullptr, L"Main class does not exists.", __FILE__, __LINE__);

	AnalyzedType* atypeDec = resolver->getClassType(dec); __STP(atypeDec);
	AnalyzedClass* aclazzDec = atypeDec->getAnalyzedClass();
	const ArrayList<AnalyzedClass>* implememtsList = aclazzDec->getImplements();

	ArrayList<UnicodeString>* list = this->instanceConfig->getDirectAccess();
	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		UnicodeString* clazzFqn = list->get(i);

		ClassDeclare* interface = contract->getClassDeclareByFqn(clazzFqn);
		ExceptionThrower<ClassNotFoundException>::throwExceptionIfCondition(interface == nullptr, L"Interface does not exists.", __FILE__, __LINE__);
		ExceptionThrower<ClassNotFoundException>::throwExceptionIfCondition(!interface->isInterface(), L"Interface class must be interface.", __FILE__, __LINE__);
		ExceptionThrower<ClassNotFoundException>::throwExceptionIfCondition(interface->isGenerics(), L"Interface class must not use Generics.", __FILE__, __LINE__);

		AnalyzedType* atype = resolver->getClassType(interface); __STP(atype);
		AnalyzedClass* aclazz = atype->getAnalyzedClass();

		bool bl = hasInterface(implememtsList, aclazz);
		ExceptionThrower<ClassNotFoundException>::throwExceptionIfCondition(!bl, L"The main class does not implements the directAccess Interface.", __FILE__, __LINE__);
	}

	return false;
}

bool AbstractExecutableModuleInstance::hasInterface(const ArrayList<AnalyzedClass> *implememtsList, AnalyzedClass *aclazz) {
	bool result = false;

	int maxLoop = implememtsList->size();
	for(int i = 0; i != maxLoop; ++i){
		AnalyzedClass* cls = implememtsList->get(i);

		if(aclazz->equals(cls)){
			result = true;
			break;
		}
	}

	return result;
}

SmartcontractExecResult* AbstractExecutableModuleInstance::invokeMainObjectMethodProxy(UnicodeString *methodName,
		ArrayList<AbstractFunctionExtArguments> *args) {
	SmartContract* contract = this->vm->getSmartContract();
	AnalyzeContext* actx = contract->getAnalyzeContext();
	TypeResolver* resolver = actx->getTypeResolver();

	FunctionArguments innerArguments;
	MethodDeclare* method = this->vm->interpretMainObjectMethod(methodName, args, &innerArguments);

	SmartcontractExecResult* result = new SmartcontractExecResult(); __STP(result);

	{
		AnalyzedType* atype = method->getReturnedType();
		if(!atype->isVoid()){
			AbstractVmInstance* vminst = innerArguments.getReturnedValue();

			UnicodeString name(L"ret");
			VTableRegistory* vreg = actx->getVtableRegistory();

			AbstractExtObject* retObj = vminst != nullptr ? vminst->toClassExtObject(&name, vreg) : new ExtNullPtrObject(&name, atype->getType());
			result->setReturnedValue(retObj);
		}
	}

	return __STP_MV(result);
}


AbstractExtObject* AbstractExecutableModuleInstance::invokeMainObjectMethodProxy(VirtualMachine* callerVm, UnicodeString *methodName, FunctionArguments *args) {
	AbstractExtObject* ret = nullptr;

	SmartContract* contract = this->vm->getSmartContract();
	AnalyzeContext* actx = contract->getAnalyzeContext();

	MethodDeclare* method = this->vm->interpretMainObjectMethodProxy(callerVm, methodName, args);

	AnalyzedType* atype = method->getReturnedType();
	ObjectReference* exObj = this->vm->getUncaughtExceptionProxy();
	if(!atype->isVoid() && exObj != nullptr){
		VTableRegistory* vreg = actx->getVtableRegistory();

		UnicodeString name(L"exception");
		ret = exObj->toClassExtObject(&name, vreg);

		this->vm->clearUncoughtException();
	}

	return ret;
}


bool AbstractExecutableModuleInstance::generateInterModularCommunicationClasses() {
	bool hasError = false;

	ArrayList<InstanceDependencyContext>* list = this->dependencyHandler->getContextList();
	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		InstanceDependencyContext* dctx = list->get(i);

		AbstractExecutableModuleInstance* importInst = dctx->getModuleInstance();
		const ModularInstanceConfig* instanceConfig = importInst->getInstanceConfig();

		const UnicodeString* mainPackage = instanceConfig->getMainPackage();
		const UnicodeString* mainClass = instanceConfig->getMainClass();
		UnicodeString* mainFqn = new UnicodeString(L""); __STP(mainFqn);
		if(mainPackage->length() > 0){
			mainFqn->append(mainPackage);
			mainFqn->append(L".");
		}
		mainFqn->append(mainClass);

		ArrayList<UnicodeString>* libExport = instanceConfig->getLibExport();

		generateLibExport(mainFqn, libExport, dctx);
	}

	return hasError;
}

bool AbstractExecutableModuleInstance::preAnalyzeInterModularCommunicationClasses() {
	SmartContract* contract = this->vm->getSmartContract();
	AnalyzeContext* actx = contract->getAnalyzeContext();

	this->factory->preAnalyze(actx);

	return actx->hasError();
}

bool AbstractExecutableModuleInstance::analyzeTypeRefInterModularCommunicationClasses() {
	SmartContract* contract = this->vm->getSmartContract();
	AnalyzeContext* actx = contract->getAnalyzeContext();

	this->factory->analyzeType(actx);



	return actx->hasError();
}

bool AbstractExecutableModuleInstance::analyzeInterModularCommunicationClasses() {
	SmartContract* contract = this->vm->getSmartContract();
	AnalyzeContext* actx = contract->getAnalyzeContext();

	this->factory->analyze(actx);

	return actx->hasError();
}

void AbstractExecutableModuleInstance::generateLibExport(UnicodeString* mainFqn, ArrayList<UnicodeString> *libExport, InstanceDependencyContext* dctx) {
	ModuleInstanceClassLoader* classLoader = dctx->getClassLoader();

	int maxLoop = libExport->size();
	for(int i = 0; i != maxLoop; ++i){
		UnicodeString* ifFqn = libExport->get(i);

		ClassDeclare* ifdec = classLoader->getClassDeclare(ifFqn);
		ExceptionThrower<ClassNotFoundException>::throwExceptionIfCondition(ifdec == nullptr, L"The main class does not implements the directAccess Interface.", __FILE__, __LINE__);
		ExceptionThrower<ClassNotFoundException>::throwExceptionIfCondition(ifdec->isGenerics(), L"The main class can not use Generics.", __FILE__, __LINE__);
		ExceptionThrower<ClassNotFoundException>::throwExceptionIfCondition(!ifdec->isInterface(), L"The main class must be interface.", __FILE__, __LINE__);

		this->factory->generateModularClass(mainFqn, ifdec, dctx);
	}
}

void AbstractExecutableModuleInstance::invokeModularProxyListnerMethod() {
	SmartContract* contract = this->vm->getSmartContract();
	AnalyzeContext* actx = contract->getAnalyzeContext();
	TypeResolver* typeResolver = actx->getTypeResolver();

	AnalyzedType* modularProxyListnerType = typeResolver->findClassType(&ModularProxyListnerClassDeclare::PACKAGE_NAME, &ModularProxyListnerClassDeclare::NAME); __STP(modularProxyListnerType);
	AnalyzedClass* modularProxyListnerAclass = modularProxyListnerType->getAnalyzedClass();

	// get main instance's runtime class
	AnalyzedClass* aclazz = this->mainInst->getAnalyzedClass();
	ClassDeclare* dec = aclazz->getClassDeclare();

	bool bl = aclazz->hasImplements(modularProxyListnerAclass);
	if(bl){
		ArrayList<InstanceDependencyContext>* list = this->dependencyHandler->getContextList();

		int maxLoop = list->size();
		for(int i = 0; i != maxLoop; ++i){
			InstanceDependencyContext* dctx = list->get(i);
			fireListner(dctx);
		}
	}
}

void AbstractExecutableModuleInstance::fireListner(InstanceDependencyContext *dctx) {
	AbstractExecutableModuleInstance* libInst = dctx->getModuleInstance();
	const ModularInstanceConfig* instanceConfig = libInst->getInstanceConfig();

	ArrayList<UnicodeString>* list = instanceConfig->getLibExport();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		UnicodeString* exportInterface = list->get(i);

		doFireListner(dctx, exportInterface, instanceConfig);
	}
}

void AbstractExecutableModuleInstance::doFireListner(InstanceDependencyContext *dctx, const UnicodeString *exportInterface, const ModularInstanceConfig* instanceConfig) {
	AnalyzedClass* mainAclazz = this->mainInst->getAnalyzedClass();

	ClassDeclare* ifdec = dctx->getClassLoader()->getClassDeclare(exportInterface);
	ExceptionThrower<ClassNotFoundException>::throwExceptionIfCondition(ifdec == nullptr, L"The main class does not implements the directAccess Interface.", __FILE__, __LINE__);

	const UnicodeString* ifpackage = ModularProxyClassDeclare::getPackageFromIf(ifdec);

	const UnicodeString* mainClassName = instanceConfig->getMainClass();
	UnicodeString* className = ModularProxyClassDeclare::getNameFromInterface(mainClassName, exportInterface); __STP(className);

	UnicodeString fqn(ifpackage);
	fqn.append(L".");
	fqn.append(className);

	CompilationUnit* unit = this->factory->getCompilantUnit(&fqn);
	ExceptionThrower<ClassNotFoundException>::throwExceptionIfCondition(unit == nullptr, L"It can't find generated proxy class declare.", __FILE__, __LINE__);

	ClassDeclare* proxyClass = unit->getClassDeclare(0);
	IVmInstanceFactory* instFactory = proxyClass->getFactory();

	SmartContract* contract = this->vm->getSmartContract();
	AnalyzeContext* actx = contract->getAnalyzeContext();

	AnalyzedClass* aclazz = actx->getAnalyzedClass(proxyClass);
	VmClassInstance* proxyInstance = instFactory->createInstance(aclazz, this->vm);

	callModuleDetectedMethod(proxyClass, proxyInstance, &fqn);
}

void AbstractExecutableModuleInstance::callModuleDetectedMethod(ClassDeclare* proxyClass, VmClassInstance *proxyInstance, const UnicodeString* ifname) {
	SmartContract* contract = this->vm->getSmartContract();
	AnalyzeContext* actx = contract->getAnalyzeContext();
	TypeResolver* typeResolver = actx->getTypeResolver();

	// arguments
	StackFloatingVariableHandler releaser(this->vm->getGc());
	releaser.registerInstance(proxyInstance);

	VmStringInstance* vmstr = new(vm) VmStringInstance(vm, ifname);
	releaser.registerInstance(vmstr);

	FunctionArguments args;
	args.addSubstance(proxyInstance);
	args.addSubstance(vmstr);

	// this pointer
	args.setThisPtr(this->mainInst);

	// find method
	AnalyzedClass* aclazzThis = this->mainInst->getAnalyzedClass();

	MethodDeclare* method = nullptr;
	{
		ArrayList<AnalyzedType> list;


		UnicodeString objfqn(&ObjectClassDeclare::PACKAGE);
		objfqn.append(L".");
		objfqn.append(&ObjectClassDeclare::NAME);

		AnalyzedType* proxtType = typeResolver->findClassType(&ObjectClassDeclare::PACKAGE, &ObjectClassDeclare::NAME); __STP(proxtType);
		list.addElement(proxtType);

		AnalyzedType stringType = vmstr->getRuntimeType();
		list.addElement(&stringType);

		method = aclazzThis->findMethod(&ModuleDetectedMethodDeclare::METHOD_NAME, &list);
	}

	assert(method != nullptr);

	{
		// top stack
		this->vm->newStack();
		StackPopper popStack(this->vm);
		VmStack* stack = this->vm->topStack();

		method->interpret(&args, this->vm);

		ExceptionInterrupt::interruptPoint(this->vm);
	}
}

} /* namespace alinous */
