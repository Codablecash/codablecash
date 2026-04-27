/*
 * SmartContract.cpp
 *
 *  Created on: 2018/12/31
 *      Author: iizuka
 */

#include "engine/sc/SmartContract.h"
#include "engine/sc/CompilationUnit.h"

#include "engine/compiler/SmartContractParser.h"

#include "engine/sc_analyze/AnalyzeContext.h"
#include "engine/sc_analyze/PackageSpace.h"
#include "engine/sc_analyze/AnalyzedClass.h"
#include "engine/sc_analyze/TypeResolver.h"

#include "engine/sc_analyze_stack/AnalyzeStackPopper.h"
#include "engine/sc_analyze_stack/AnalyzeStackManager.h"

#include "base/UnicodeString.h"
#include "base/Exception.h"

#include "lang/sc_declare/ClassDeclare.h"
#include "lang/sc_declare/MethodDeclare.h"

#include "instance/VmClassInstance.h"

#include "instance/instance_exception/ExceptionInterrupt.h"

#include "instance/reserved_classes/ReservedClassRegistory.h"

#include "instance/instance_ref_class_static/StaticClassReferenceHolder.h"

#include "instance/instance_ref/ObjectReference.h"
#include "instance/instance_ref/VmRootReference.h"

#include "instance/instance_gc/GcManager.h"


#include "base_io_stream/FileInputStream.h"
#include "base_io/File.h"

#include "vm/exceptions.h"

#include "vm/stack/StackPopper.h"
#include "vm/stack/VmStack.h"


#include "engine/compiler/CompileError.h"
#include "engine/compiler/ParseErrorHandler.h"

#include "ext_arguments/AbstractFunctionExtArguments.h"

#include "bc/ExceptionThrower.h"

#include "base/StackRelease.h"

#include "vm/IInitializeCompilantUnitProvidor.h"
namespace alinous {

SmartContract::SmartContract() {
	this->actx = nullptr;
	this->mainPackage = nullptr;
	this->mainClass = nullptr;
	this->mainMethod = nullptr;
	this->rootReference = nullptr;
	this->initialized = false;
	this->reservedClassRegistory = nullptr;
}

SmartContract::~SmartContract() {
	this->progs.deleteElements();
	delete this->actx;
	delete this->mainPackage;
	delete this->mainClass;
	delete this->mainMethod;
	delete this->rootReference;
	delete this->reservedClassRegistory;

	this->compileErrorList.deleteElements();
}

void SmartContract::setMainMethod(const UnicodeString* mainPackage,
		const UnicodeString* mainClass, const UnicodeString* mainMethod) {
	if(mainPackage != nullptr){
		delete this->mainPackage;
		this->mainPackage = new UnicodeString(*mainPackage);
	}

	delete this->mainClass;
	delete this->mainMethod;

	this->mainClass = new UnicodeString(*mainClass);
	this->mainMethod = new UnicodeString(*mainMethod);
}

CompilationUnit* SmartContract::addCompilationUnit(InputStream* stream, int length, const File* base, File* source) {
	SmartContractParser parser(stream, length);

	CompilationUnit* unit = parser.parse();

	const ArrayList<CompileError>& errorList = parser.getParserErrorHandler()->getList();
	int maxLoop = errorList.size();
	for(int i = 0; i != maxLoop; ++i){
		CompileError* error = errorList.get(i);

		CompileError* newError = new CompileError(*error);
		this->compileErrorList.addElement(newError);
	}

	this->progs.addElement(unit);

	return unit;
}

CompilationUnit* SmartContract::addCompilationUnit(File* file, const File* base) {
	FileInputStream stream(file);

	int length = file->length();
	return addCompilationUnit(&stream, length, base, file);
}

void SmartContract::analyze(VirtualMachine* vm) {
	initBeforeAnalyze(vm);

	preAnalyze(vm);
	if(this->actx->hasError()){
		return;
	}

	preAnalyzeGenerics(vm);
	if(this->actx->hasError()){
		return;
	}

	analyzeType(vm);
	if(this->actx->hasError()){
		return;
	}

	analyzeMetadata(vm);
	if(this->actx->hasError()){
		return;
	}

	analyzeFinal(vm);
}


void SmartContract::initBeforeAnalyze(VirtualMachine *vm) {
	delete this->reservedClassRegistory;
	this->reservedClassRegistory = new ReservedClassRegistory();

	this->actx = new AnalyzeContext(this);
	this->actx->setVm(vm);
	this->actx->resigterReservedClasses();
}

void SmartContract::preAnalyze(VirtualMachine *vm) {
	const ArrayList<AnalyzedClass>* list = this->reservedClassRegistory->getReservedClassesList();

	// pre analyze
	int maxLoop = this->progs.size();
	for(int i = 0; i != maxLoop; ++i){
		CompilationUnit* unit = this->progs.get(i);
		unit->preAnalyze(this->actx);
	}
	maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		AnalyzedClass* cls = list->get(i);
		ClassDeclare* dec = cls->getClassDeclare();

		const UnicodeString* packageName = dec->getPackageName();
		CompilationUnit* reservedUnit = this->reservedClassRegistory->makeCompilantUnit(packageName);
		reservedUnit->addClassDeclare(dec);

		dec->setParent(reservedUnit);
		dec->preAnalyze(this->actx);
	}
}

void SmartContract::preAnalyzeGenerics(VirtualMachine *vm) {
	// generics
	this->actx->generateGenericsClasses();
	this->actx->preAnalyzeGenerics();
}

void SmartContract::analyzeType(VirtualMachine *vm) {
	const ArrayList<AnalyzedClass>* list = this->reservedClassRegistory->getReservedClassesList();

	int maxLoop = this->progs.size();
	for(int i = 0; i != maxLoop; ++i){
		CompilationUnit* unit = this->progs.get(i);
		unit->analyzeType(this->actx);
	}

	maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		AnalyzedClass* cls = list->get(i);
		ClassDeclare* dec = cls->getClassDeclare();

		dec->analyzeTypeRef(actx);
	}

	this->actx->analyzeTypeRefGenerics();
}

void SmartContract::analyzeMetadata(VirtualMachine *vm) {
	// static meta data
	this->actx->analyzeStaticVariables();

	// inheritance
	this->actx->analyzeClassInheritance();
}


void SmartContract::analyzeFinal(VirtualMachine *vm) {
	const ArrayList<AnalyzedClass>* list = this->reservedClassRegistory->getReservedClassesList();
	// make top stack
	AnalyzeStackManager* stackMgr = actx->getAnalyzeStackManager();
	AnalyzeStackPopper popper(stackMgr, true);
	stackMgr->addFunctionStack();

	int maxLoop = this->progs.size();
	for(int i = 0; i != maxLoop; ++i){
		CompilationUnit* unit = this->progs.get(i);
		unit->analyze(this->actx);
	}

	maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		AnalyzedClass* cls = list->get(i);
		ClassDeclare* dec = cls->getClassDeclare();

		dec->analyze(actx);
	}

	this->actx->analyzeGenerics();
}

bool SmartContract::hasError() noexcept {
	return this->actx->hasError();
}

VmRootReference* SmartContract::getRootReference() const noexcept {
	return this->rootReference;
}

void SmartContract::clearRootReference(VirtualMachine* vm) noexcept {
	this->rootReference->resetOnGc();
	delete this->rootReference;
	this->rootReference = nullptr;

	vm->setVmRootReference(nullptr);
	this->initialized = false;
}
void SmartContract::releaseMainInstance(GcManager* gc) noexcept {
	this->rootReference->releaseMainInstance(gc);
}


VmClassInstance* SmartContract::createInstance(VirtualMachine* vm, ArrayList<IInitializeCompilantUnitProvidor>* exprogs) {
	initialize(vm, exprogs);

	PackageSpace* space = this->actx->getPackegeSpace(this->mainPackage);
	ExceptionThrower<VmClassNotFoundException>::throwExceptionIfCondition(space == nullptr, L"The package space does not exist.", __FILE__, __LINE__);

	AnalyzedClass* clazz = space->getClass(this->mainClass);
	ExceptionThrower<VmClassNotFoundException>::throwExceptionIfCondition(clazz == nullptr, L"The Main Class does not exist.", __FILE__, __LINE__);

	MethodDeclare* defConstructor = clazz->getDefaultConstructor();

	GcManager* gc = vm->getGc();
	vm->newStack();
	StackPopper popStack(vm);
	VmStack* stack = vm->topStack();

	VmClassInstance* inst = nullptr;

	try{
		inst = VmClassInstance::createObject(clazz, vm);
	}
	catch(ExceptionInterrupt* e){
		delete e;
		return nullptr;
	}


	this->rootReference->setMainInstance(inst);

	ObjectReference* instRef = ObjectReference::createObjectReference(stack, inst, vm);
	stack->addInnerReference(instRef);

	// exec constructor
	try{
		ArrayList<AbstractFunctionExtArguments> arguments;
		vm->interpret(defConstructor, inst, &arguments, exprogs);
	}
	catch(Exception* e){
		throw e;
	}


	return inst;
}

void SmartContract::initialize(VirtualMachine* vm, ArrayList<IInitializeCompilantUnitProvidor>* exprogs) {
	if(this->initialized){
		return;
	}

	this->rootReference = new(vm) VmRootReference(vm);
	vm->setVmRootReference(this->rootReference);

	int maxLoop = this->progs.size();
	for(int i = 0; i != maxLoop; ++i){
		CompilationUnit* unit = this->progs.get(i);
		unit->init(vm);
	}

	if(exprogs != nullptr){
		initializeExprogs(vm, exprogs);
	}

	StaticClassReferenceHolder* staticClassHolder = this->rootReference->getStaticClassReferenceHolder();
	staticClassHolder->init(vm, actx);

	this->initialized = true;
}

void SmartContract::initializeExprogs(VirtualMachine *vm, ArrayList<IInitializeCompilantUnitProvidor> *exprogs) {
	int maxLoop = exprogs->size();
	for(int i = 0; i != maxLoop; ++i){
		IInitializeCompilantUnitProvidor* providor = exprogs->get(i);

		providor->initCompilantUnits(vm);
	}
}

AnalyzeContext* SmartContract::getAnalyzeContext() const noexcept {
	return this->actx;
}

const ArrayList<CompileError>* SmartContract::getCompileErrors() const noexcept {
	return &this->compileErrorList;
}

int SmartContract::getNumCompilationUnit() const noexcept {
	return this->progs.size();
}

CompilationUnit* SmartContract::getCompilationUnit(int pos) const noexcept {
	return this->progs.get(pos);
}

void SmartContract::addCompilationUnit(CompilationUnit *unit) {
	this->progs.addElement(unit);
}

ReservedClassRegistory* SmartContract::getReservedClassRegistory() const noexcept {
	return this->reservedClassRegistory;
}

ClassDeclare* SmartContract::getClassDeclareByFqn(const UnicodeString *fqn) const {
	ClassDeclare* result = nullptr;

	int maxLoop = this->progs.size();
	for(int i = 0; i != maxLoop; ++i){
		CompilationUnit* unit = this->progs.get(i);

		UnicodeString* package = TypeResolver::getPackageName(fqn); __STP(package);
		UnicodeString* className = TypeResolver::getClassName(fqn); __STP(className);

		const UnicodeString* unitPackage = unit->getPackageName();

		if((package != nullptr && package->equals(unitPackage)) || (package == nullptr && unitPackage == nullptr)){
			ClassDeclare* dec = unit->getClassDeclare(className);

			if(dec != nullptr){
				result = dec;
				break;
			}
		}
	}

	return result;
}

UnicodeString* SmartContract::getMainClassFqn() const noexcept {
	UnicodeString* fqn = new UnicodeString(L"");

	if(this->mainPackage != nullptr){
		fqn->append(this->mainPackage);
		fqn->append(L".");
	}

	fqn->append(this->mainClass);

	return fqn;
}

} /* namespace alinous */
