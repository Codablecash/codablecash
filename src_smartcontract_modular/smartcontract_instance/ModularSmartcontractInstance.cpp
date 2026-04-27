/*
 * ModularSmartcontractInstance.cpp
 *
 *  Created on: Sep 22, 2025
 *      Author: iizuka
 */

#include "smartcontract_instance/ModularSmartcontractInstance.h"
#include "smartcontract_instance/ExecutableModuleInstance.h"
#include "smartcontract_instance/LibraryExectableModuleInstance.h"

#include "modular_project_registory/SmartcontractProjectData.h"

#include "modular_project/ModularProjectConfig.h"
#include "modular_project/SmartcontractProjectId.h"
#include "modular_project_registory/ProjectIdKey.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"
#include "base/Exception.h"
#include "base/exceptions.h"

#include "bc/ExceptionThrower.h"

#include "bc_base/BinaryUtils.h"
#include "bc_base/AbstractAddress.h"
#include "bc_base/AddressDescriptor.h"

#include "base_io/ByteBuffer.h"
#include "base_io/File.h"

#include "crypto/Sha256.h"

#include "engine/CodableDatabase.h"

#include "transaction/SmartcontractInstanceAddress.h"

#include "smartcontract_instance/SmartcontractExecutionException.h"


namespace codablecash {

ModularSmartcontractInstance::ModularSmartcontractInstance() {
	this->config = nullptr;
	this->execModule = nullptr;
	this->libraries = new HashMap<UnicodeString, LibraryExectableModuleInstance>();
	this->libArray = new ArrayList<LibraryExectableModuleInstance>();
	this->instanceAddress = nullptr;
	this->dbRoot = nullptr;
}

ModularSmartcontractInstance::~ModularSmartcontractInstance() {
	delete this->config;
	delete this->execModule;

	{
		Iterator<UnicodeString>* it = this->libraries->keySet()->iterator(); __STP(it);
		while(it->hasNext()){
			const UnicodeString* key = it->next();
			LibraryExectableModuleInstance* inst = this->libraries->get(key);

			delete inst;
		}
	}
	delete this->libraries;
	delete this->libArray;
	delete this->instanceAddress;

	delete this->dbRoot;
}

void ModularSmartcontractInstance::setModularProjectConfig(const ModularProjectConfig *config) {
	delete this->config;
	this->config = config->copy();
}

void ModularSmartcontractInstance::setExecutableModuleInstance(ExecutableModuleInstance *inst) {
	this->execModule = inst;
}

void ModularSmartcontractInstance::addLibraryModuleInstance(const UnicodeString *name, LibraryExectableModuleInstance *inst) {
	this->libraries->put(name, inst);
	this->libArray->addElement(inst);
}

void ModularSmartcontractInstance::loadCompilantUnits(const File *projectBaseDir) {
	this->execModule->loadCompilantUnits(projectBaseDir);

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* inst = this->libArray->get(i);
		inst->loadCompilantUnits(projectBaseDir);
	}
}

bool ModularSmartcontractInstance::libraryExists(const UnicodeString *name) const noexcept {
	LibraryExectableModuleInstance* value = this->libraries->get(name);
	return value != nullptr;
}

LibraryExectableModuleInstance* ModularSmartcontractInstance::getLibraryModuleInstance(const UnicodeString *modName) const noexcept {
	LibraryExectableModuleInstance* value = this->libraries->get(modName);
	return value;
}

bool ModularSmartcontractInstance::hasCompileError() const noexcept {
	bool hasError = this->execModule->hasCompileError();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* inst = this->libArray->get(i);
		hasError |= inst->hasCompileError();
	}

	return hasError;
}



bool ModularSmartcontractInstance::preAnalyzeGoup() {
	bool hasError = false;
	try{
		initBeforeAnalyze();
		preAnalyze();
		preAnalyzeGenerics();
		loadDependency();
		preAnalyzeDependency();
	}
	catch(CompilantUnitAnalyzeException* e){
		delete e;
		hasError = true;
	}

	return hasError;
}

bool ModularSmartcontractInstance::analyzeTypeRefGroup() {
	bool hasError = false;
	try{
		analyzeType();
		analyzeTypeDependency();
	}
	catch(CompilantUnitAnalyzeException* e){
		delete e;
		hasError = true;
	}

	return hasError;
}

bool ModularSmartcontractInstance::analyzeMetadataGroup() {
	bool hasError = this->execModule->analyzeMetadata();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* inst = this->libArray->get(i);
		hasError |= inst->analyzeMetadata();
	}

	return hasError;
}


bool ModularSmartcontractInstance::analyzeGroup() {
	bool hasError = false;

	try{
		analyzeFinal();
		analyzeDependency();
	}
	catch(CompilantUnitAnalyzeException* e){
		delete e;
		hasError = true;
	}

	return hasError;
}

bool ModularSmartcontractInstance::checkDirectAccess() {
	bool hasError = this->execModule->checkDirectAccess();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* inst = this->libArray->get(i);
		hasError |= inst->checkDirectAccess();
	}

	return hasError;
}

void ModularSmartcontractInstance::initBeforeAnalyze() {
	bool hasError = this->execModule->initBeforeAnalyze();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* inst = this->libArray->get(i);
		hasError |= inst->initBeforeAnalyze();
	}

	ExceptionThrower<CompilantUnitAnalyzeException>::throwExceptionIfCondition(hasError, L"initBeforeAnalyze() failed.", __FILE__, __LINE__);
}

void ModularSmartcontractInstance::preAnalyze() {
	bool hasError = this->execModule->preAnalyze();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* inst = this->libArray->get(i);
		hasError |= inst->preAnalyze();
	}

	ExceptionThrower<CompilantUnitAnalyzeException>::throwExceptionIfCondition(hasError, L"preAnalyze() failed.", __FILE__, __LINE__);
}

void ModularSmartcontractInstance::preAnalyzeGenerics() {
	bool hasError = this->execModule->preAnalyzeGenerics();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* inst = this->libArray->get(i);
		hasError |= inst->preAnalyzeGenerics();
	}

	ExceptionThrower<CompilantUnitAnalyzeException>::throwExceptionIfCondition(hasError, L"preAnalyzeGenerics() failed.", __FILE__, __LINE__);
}

void ModularSmartcontractInstance::analyzeType() {
	bool hasError = this->execModule->analyzeType();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* inst = this->libArray->get(i);
		hasError |= inst->analyzeType();
	}

	ExceptionThrower<CompilantUnitAnalyzeException>::throwExceptionIfCondition(hasError, L"analyzeType() failed.", __FILE__, __LINE__);
}

void ModularSmartcontractInstance::analyzeFinal() {
	bool hasError = this->execModule->analyzeFinal();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* inst = this->libArray->get(i);
		hasError |= inst->analyzeFinal();
	}

	ExceptionThrower<CompilantUnitAnalyzeException>::throwExceptionIfCondition(hasError, L"analyzeFinal() failed.", __FILE__, __LINE__);
}

void ModularSmartcontractInstance::loadDependency() {
	bool hasError = false;

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* inst = this->libArray->get(i);
		hasError |= inst->loadDependency(this);
	}

	hasError = this->execModule->loadDependency(this);

	ExceptionThrower<CompilantUnitAnalyzeException>::throwExceptionIfCondition(hasError, L"loadDependency() failed.", __FILE__, __LINE__);
}

void ModularSmartcontractInstance::preAnalyzeDependency() {
	bool hasError = false;

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* inst = this->libArray->get(i);
		hasError |= inst->preAnalyzeDependency();
	}

	hasError = this->execModule->preAnalyzeDependency();

	ExceptionThrower<CompilantUnitAnalyzeException>::throwExceptionIfCondition(hasError, L"preAnalyzeDependency() failed.", __FILE__, __LINE__);
}

void ModularSmartcontractInstance::analyzeTypeDependency() {
	bool hasError = false;

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* inst = this->libArray->get(i);
		hasError |= inst->analyzeTypeDependency();
	}

	hasError = this->execModule->analyzeTypeDependency();

	ExceptionThrower<CompilantUnitAnalyzeException>::throwExceptionIfCondition(hasError, L"preAnalyzeDependency() failed.", __FILE__, __LINE__);
}

void ModularSmartcontractInstance::analyzeDependency() {
	bool hasError = this->execModule->analyzeDependency();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* inst = this->libArray->get(i);
		hasError |= inst->analyzeDependency();
	}

	ExceptionThrower<CompilantUnitAnalyzeException>::throwExceptionIfCondition(hasError, L"analyzeDependency() failed.", __FILE__, __LINE__);
}

void ModularSmartcontractInstance::setMainInstance() {
	this->execModule->setMainInstance();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* inst = this->libArray->get(i);
		inst->setMainInstance();
	}
}

bool ModularSmartcontractInstance::createMainInstance() {
	bool hasError = !this->execModule->createMainInstance();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* inst = this->libArray->get(i);
		hasError |= (!inst->createMainInstance());
	}

	return hasError;
}

bool ModularSmartcontractInstance::interpretInitializer() {
	bool hasError = !this->execModule->interpretInitializer();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* inst = this->libArray->get(i);
		hasError |= (!inst->interpretInitializer());
	}

	return hasError;
}

void ModularSmartcontractInstance::resetRootReference() {
	this->execModule->resetRootReference();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* inst = this->libArray->get(i);
		inst->resetRootReference();
	}
}

int ModularSmartcontractInstance::binarySize() const {
	BinaryUtils::checkNotNull(this->config);
	BinaryUtils::checkNotNull(this->execModule);

	int total = this->config->binarySize();
	total += this->execModule->binarySize();

	int maxLoop = this->libArray->size();
	total += sizeof(uint16_t);

	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* libmod = this->libArray->get(i);
		total += libmod->binarySize();
	}

	return total;
}

void ModularSmartcontractInstance::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->config);
	BinaryUtils::checkNotNull(this->execModule);

	this->config->toBinary(out);
	this->execModule->toBinary(out);

	int maxLoop = this->libArray->size();
	out->putShort(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* libmod = this->libArray->get(i);
		libmod->toBinary(out);
	}
}

ModularSmartcontractInstance* ModularSmartcontractInstance::createFromBinary(ByteBuffer *in) {
	ModularSmartcontractInstance* inst = new ModularSmartcontractInstance(); __STP(inst);

	{
		ModularProjectConfig* config = ModularProjectConfig::createFromBinary(in); __STP(config);
		inst->setModularProjectConfig(config);
	}

	{
		AbstractExecutableModuleInstance* mod = AbstractExecutableModuleInstance::createFromBinary(in); __STP(mod);
		ExecutableModuleInstance* exec = dynamic_cast<ExecutableModuleInstance*>(mod);
		BinaryUtils::checkNotNull(exec);
		__STP_MV(mod);

		inst->setExecutableModuleInstance(exec);
	}

	int maxLoop = in->getShort();
	for(int i = 0; i != maxLoop; ++i){
		AbstractExecutableModuleInstance* mod = AbstractExecutableModuleInstance::createFromBinary(in); __STP(mod);
		LibraryExectableModuleInstance* libmod = dynamic_cast<LibraryExectableModuleInstance*>(mod);
		BinaryUtils::checkNotNull(libmod);
		__STP_MV(mod);

		const UnicodeString* name = libmod->getLibraryName();
		inst->addLibraryModuleInstance(name, libmod);
	}

	return __STP_MV(inst);
}

SmartcontractProjectData* ModularSmartcontractInstance::createData() const {
	SmartcontractProjectData* data = new SmartcontractProjectData(); __STP(data);

	ByteBuffer* buff = createBinary(); __STP(buff);
	buff->position(0);
	data->setData(buff);

	{
		SmartcontractProjectId* projectId = __getProjectId(buff); __STP(projectId);
		ProjectIdKey key;
		key.setProjectId(projectId);

		data->setKey(&key);
	}

	{
		ModularProjectConfig* mconfig = getModularProjectConfig();
		const SoftwareVersion* v = mconfig->getVersion();

		data->setVersion(v);
	}

	return __STP_MV(data);
}

SmartcontractProjectId* ModularSmartcontractInstance::getProjectId() const {
	ByteBuffer* buff = createBinary(); __STP(buff);

	return __getProjectId(buff);
}

SmartcontractProjectId* ModularSmartcontractInstance::__getProjectId(ByteBuffer *buff) const {
	buff->position(0);

	ByteBuffer* shaBuff = Sha256::sha256(buff, true); __STP(shaBuff);

	SmartcontractProjectId* projectId = new SmartcontractProjectId((const char*)shaBuff->array(), shaBuff->capacity()); __STP(projectId);

	return __STP_MV(projectId);
}

ByteBuffer* ModularSmartcontractInstance::createBinary() const {
	int cap = binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);

	toBinary(buff);

	return __STP_MV(buff);
}

void ModularSmartcontractInstance::setDatabaseDir(const File *baseDir) {
	AddressDescriptor* desc = this->instanceAddress->toAddressDescriptor(); __STP(desc);

	const char* cstr = desc->toCString();
	StackArrayRelease<const char> __st_cstr(cstr);
	UnicodeString seg(cstr);

	delete this->dbRoot;
	this->dbRoot = baseDir->get(&seg);

	this->execModule->setDatabaseDir(dbRoot);

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* lib = this->libArray->get(i);
		lib->setDatabaseDir(this->dbRoot);
	}
}

void ModularSmartcontractInstance::cleanDbRoot() {
	if(this->dbRoot != nullptr && this->dbRoot->exists()){
		this->dbRoot->deleteDir();
	}
}

void ModularSmartcontractInstance::createDatabase() {
	this->execModule->createDatabase();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* lib = this->libArray->get(i);
		lib->createDatabase();
	}
}

void ModularSmartcontractInstance::loadDatabase() {
	this->execModule->loadDatabase();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* lib = this->libArray->get(i);
		lib->loadDatabase();
	}
}

void ModularSmartcontractInstance::newSession(const CdbDatabaseSessionId *sessionId) {
	this->execModule->newSession(sessionId);

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* lib = this->libArray->get(i);
		lib->newSession(sessionId);
	}
}

void ModularSmartcontractInstance::undoCurrentSession() {
	this->execModule->undoCurrentSession();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* lib = this->libArray->get(i);
		lib->undoCurrentSession();
	}
}

const CdbDatabaseSessionId* ModularSmartcontractInstance::getDatabaseSessionId() const noexcept {
	const CdbDatabaseSessionId* sessionId = this->execModule->getDatabaseSessionId();
	return sessionId;
}

void ModularSmartcontractInstance::setSmartcontractInstanceAddress(const SmartcontractInstanceAddress *address) {
	delete this->instanceAddress;
	this->instanceAddress = dynamic_cast<SmartcontractInstanceAddress*>(address->copyData());
}

SmartcontractExecResult* ModularSmartcontractInstance::invokeMainObjectMethod(UnicodeString *moduleName, UnicodeString *methodName,
		ArrayList<AbstractFunctionExtArguments>* args) {

	AbstractExecutableModuleInstance* module = nullptr;

	const UnicodeString* name = this->execModule->getName();
	if(name->equals(moduleName)){
		module = this->execModule;
	}
	else{
		int maxLoop = this->libArray->size();
		for(int i = 0; i != maxLoop; ++i){
			LibraryExectableModuleInstance* lib = libArray->get(i);

			name = lib->getName();
			if(name->equals(moduleName)){
				module = lib;
				break;
			}
		}
	}
	ExceptionThrower<SmartcontractExecutionException>::throwExceptionIfCondition(module == nullptr, L"Module name is wrong.", __FILE__, __LINE__);

	return module->invokeMainObjectMethodProxy(methodName, args);
}

bool ModularSmartcontractInstance::generateInterModularCommunicationClasses() {
	bool hasError = this->execModule->generateInterModularCommunicationClasses();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* lib = libArray->get(i);

		hasError |= lib->generateInterModularCommunicationClasses();
	}

	return hasError;
}

bool ModularSmartcontractInstance::preAnalyzeInterModularCommunicationClasses() {
	bool hasError = this->execModule->preAnalyzeInterModularCommunicationClasses();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* lib = libArray->get(i);

		hasError |= lib->preAnalyzeInterModularCommunicationClasses();
	}

	return hasError;
}

bool ModularSmartcontractInstance::analyzeTypeRefInterModularCommunicationClasses() {
	bool hasError = this->execModule->analyzeTypeRefInterModularCommunicationClasses();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* lib = libArray->get(i);

		hasError |= lib->analyzeTypeRefInterModularCommunicationClasses();
	}

	return hasError;
}

bool ModularSmartcontractInstance::analyzeInterModularCommunicationClasses() {
	bool hasError = this->execModule->analyzeInterModularCommunicationClasses();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* lib = libArray->get(i);

		hasError |= lib->analyzeInterModularCommunicationClasses();
	}

	return hasError;
}

void ModularSmartcontractInstance::invokeModularProxyListnerMethod() {
	this->execModule->invokeModularProxyListnerMethod();

	int maxLoop = this->libArray->size();
	for(int i = 0; i != maxLoop; ++i){
		LibraryExectableModuleInstance* lib = libArray->get(i);
		lib->invokeModularProxyListnerMethod();
	}
}

} /* namespace codablecash */
