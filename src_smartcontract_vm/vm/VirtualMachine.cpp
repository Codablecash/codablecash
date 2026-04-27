/*
 * VirtualMachine.cpp
 *
 *  Created on: 2018/12/31
 *      Author: iizuka
 */

#include "vm/VirtualMachine.h"
#include "engine/sc/SmartContract.h"

#include "vm/memory/VmMemoryManager.h"
#include "instance/instance_parts/VmMalloc.h"
#include "instance/instance_gc/GcManager.h"

#include "instance/IAbstractVmInstanceSubstance.h"
#include "instance/VmClassInstance.h"

#include "instance/instance_ref/AbstractReference.h"
#include "instance/instance_ref/ObjectReference.h"

#include "instance/instance_ref/VmRootReference.h"

#include "lang/sc_declare/MethodDeclare.h"
#include "lang/sc_statement/StatementBlock.h"

#include "engine/sc_analyze_functions/FunctionScoreCalc.h"
#include "engine/sc_analyze_functions/VTableRegistory.h"
#include "engine/sc_analyze_functions/VTableMethodEntry.h"
#include "engine/sc_analyze_functions/MethodScore.h"

#include "engine/sc_analyze/AnalyzedClass.h"
#include "engine/sc_analyze/AnalyzeContext.h"
#include "engine/sc_analyze/AnalyzedType.h"
#include "engine/sc_analyze/ValidationError.h"
#include "engine/sc_analyze/TypeResolver.h"

#include "ext_arguments/AbstractFunctionExtArguments.h"

#include "ext_binary/ExtExceptionObject.h"

#include "vm/exceptions.h"
#include "vm/IInitializeCompilantUnitProvidor.h"

#include "vm/stack/StackPopper.h"
#include "vm/stack/VmStackManager.h"
#include "vm/stack/VmStack.h"

#include "vm/vm_ctrl/ExecControlManager.h"
#include "vm/vm_ctrl/ExceptionControl.h"

#include "vm/variable_access/FunctionArguments.h"

#include "instance/reserved_classes/ReservedClassRegistory.h"
#include "instance/reserved_classes_string/StringClassDeclare.h"

#include "instance/instance_exception_class/ExceptionClassDeclare.h"
#include "instance/instance_exception_class/StackTraceElement.h"
#include "instance/instance_exception_class/VmExceptionInstance.h"

#include "engine/CodableDatabase.h"

#include "vm/vm_trx/VmTransactionHandler.h"

#include "scan_select/scan_planner/base/SelectScanPlanner.h"

#include "scan_select/scan_condition/exp_escape/EscapeTargetCondition.h"

#include "base/Exception.h"
#include "base/StackRelease.h"
#include "base/UnicodeString.h"

#include "instance/instance_gc/StackFloatingVariableHandler.h"

#include "ext_arguments/ArgumentType.h"

#include "engine/sc_analyze_functions/VTableClassEntry.h"
namespace alinous {

VirtualMachine::VirtualMachine(uint64_t memCapacity) {
	this->sc = nullptr;
	this->memory = new VmMemoryManager(memCapacity);
	this->alloc = new VmMalloc(this);
	this->gc = new GcManager();
	this->stackManager = nullptr;
	this->argsRegister = nullptr;
	this->destried = false;
	this->initialized = false;
	this->rootReference = nullptr;
	this->ctrl = new ExecControlManager();
	this->uncaughtException = nullptr;
	this->caught = false;
	this->lastElement = nullptr;
	this->db = new CodableDatabase();
	this->trxHandler = new VmTransactionHandler(this->db);
	this->espaceTargetCondition = nullptr;
}

VirtualMachine::~VirtualMachine() {
	if(!this->destried){
		destroy();
	}

	delete this->sc;

	delete this->stackManager;
	delete this->gc;

	delete this->memory;
	delete this->alloc;
	delete this->ctrl;

	this->argsRegister = nullptr;
	this->rootReference = nullptr;

	this->exceptions.deleteElements();
	delete this->uncaughtException;

	delete this->trxHandler;
	delete this->db;

	delete this->espaceTargetCondition;
}

void VirtualMachine::loadSmartContract(SmartContract* sc) {
	this->sc = sc;

	delete this->stackManager;
	this->stackManager = new VmStackManager();
}

void VirtualMachine::loadDatabase(const File* dbdir, const File* undoDir) {
	this->db->loadDatabase(dbdir, undoDir);
}

VmClassInstance* VirtualMachine::createScInstance(ArrayList<IInitializeCompilantUnitProvidor>* exprogs) {
	VmClassInstance* retInst = nullptr;

	initialize(exprogs);

	try{
		retInst = this->sc->createInstance(this, exprogs);
	}
	catch(Exception* e){
		this->exceptions.addElement(e);
	}

	// uncaught exception
	checkUncaughtException();

	return retInst;
}

void VirtualMachine::interpret(const UnicodeString* method) {
	ArrayList<AbstractFunctionExtArguments> list;
	interpretMainObjectMethod(method, &list);
}

MethodDeclare* VirtualMachine::interpretMainObjectMethod(const UnicodeString *method, ArrayList<AbstractFunctionExtArguments> *arguments) {
	FunctionArguments args;

	MethodDeclare* methodDec = interpretMainObjectMethod(method, arguments, &args);

	// floating
	StackFloatingVariableHandler floatHandler(this->gc);
	AbstractVmInstance* vmInst = args.getReturnedValue();
	floatHandler.registerInstance(vmInst);

	return methodDec;
}


MethodDeclare* VirtualMachine::interpretMainObjectMethod(const UnicodeString* method, ArrayList<AbstractFunctionExtArguments>* arguments, FunctionArguments* args) {
	VmClassInstance* _this = dynamic_cast<VmClassInstance*>(this->sc->getRootReference()->getInstance());
	AnalyzedClass* aclass = _this->getAnalyzedClass();

	const UnicodeString* fqn = aclass->getFullQualifiedName();

	AnalyzeContext* actx = this->sc->getAnalyzeContext();
	VTableRegistory* vreg = actx->getVtableRegistory();
	VTableClassEntry* classEntry = vreg->getClassEntry(fqn, aclass);

	FunctionScoreCalc calc(classEntry);

	// arguments type
	ArrayList<AnalyzedType> typeList;
	typeList.setDeleteOnExit();


	TypeResolver* resolver = actx->getTypeResolver();
	ClassDeclare* clazzDec = classEntry->getClassDeclare();

	int maxLoop = arguments->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractFunctionExtArguments* extArg = arguments->get(i);

		ArgumentType* argumentType = extArg->getType(); __STP(argumentType);
		AnalyzedType* at = argumentType->toAnalyzedType(resolver, clazzDec); __STP(at);


		typeList.addElement(new AnalyzedType(*at));
	}

	MethodScore* score = calcScore(&calc, method, &typeList);

	VTableMethodEntry* methodEntry = score->getEntry();
	MethodDeclare* methodDeclare = methodEntry->getMethod();

	args->setThisPtr(_this);

	for(int i = 0; i != maxLoop; ++i){
		AbstractFunctionExtArguments* extArg = arguments->get(i);

		AbstractVmInstance* inst = extArg->interpret(this);

		args->addSubstance(inst != nullptr ? inst->getInstance() : nullptr);
	}


	// top stack
	this->newStack();
	StackPopper popStack(this);
	VmStack* stack = this->topStack();

	methodDeclare->interpret(args, this);

	// uncaught exception
	checkUncaughtException();

	return methodDeclare;
}


MethodDeclare* VirtualMachine::interpretMainObjectMethodProxy(VirtualMachine* callerVm, const UnicodeString *method, FunctionArguments *args) {
	VmClassInstance* _this = dynamic_cast<VmClassInstance*>(this->sc->getRootReference()->getInstance());
	AnalyzedClass* aclass = _this->getAnalyzedClass();

	const UnicodeString* fqn = aclass->getFullQualifiedName();

	AnalyzeContext* actx = this->sc->getAnalyzeContext();
	VTableRegistory* vreg = actx->getVtableRegistory();
	VTableClassEntry* classEntry = vreg->getClassEntry(fqn, aclass);

	// calc type
	FunctionScoreCalc calc(classEntry);

	ArrayList<AnalyzedType> typeList;
	typeList.setDeleteOnExit();

	const ArrayList<IAbstractVmInstanceSubstance>* list = args->getArguments();
	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		IAbstractVmInstanceSubstance* vminst = list->get(i);

		AnalyzedType at = vminst->getRuntimeType();
		typeList.addElement(new AnalyzedType(at));
	}

	// score
	MethodScore* score = calcScore(&calc, method, &typeList);

	VTableMethodEntry* methodEntry = score->getEntry();
	MethodDeclare* methodDeclare = methodEntry->getMethod();

	// FIXME when vminst is object, copy caller's substance with callee vm(this)
	FunctionArguments localArguments;
	for(int i = 0; i != maxLoop; ++i){
		IAbstractVmInstanceSubstance* vminst = list->get(i);
		localArguments.addSubstance(vminst);
	}
	localArguments.setThisPtr(_this);

	// top stack
	this->newStack();
	StackPopper popStack(this);
	VmStack* stack = this->topStack();

	methodDeclare->interpret(&localArguments, this);

	// set return value local to arg
	{
		AbstractVmInstance* returnedValue = localArguments.getReturnedValue();
		if(returnedValue != nullptr){
			AnalyzeContext* actx = this->sc->getAnalyzeContext();
			VTableRegistory* vtableReg = actx->getVtableRegistory();

			IAbstractVmInstanceSubstance* sub = returnedValue->getInstance();

			UnicodeString ret(L"ret");
			AbstractExtObject* extObject = sub->instToClassExtObject(&ret, vtableReg); __STP(extObject);

			AbstractVmInstance* vmInst = extObject->toVmInstance(callerVm);
			args->setReturnedValue(vmInst);
		}
	}

	// uncaught exception
	checkUncaughtException();

	return methodDeclare;
}


MethodScore* VirtualMachine::calcScore(FunctionScoreCalc *calc, const UnicodeString *method, ArrayList<AnalyzedType> *typeList) {
	MethodScore* score = calc->findMethod(method, typeList);
	if(score == nullptr){
		throw new VmMethodNotFoundException(__FILE__, __LINE__);
	}
	return score;
}

void VirtualMachine::interpret(MethodDeclare* method, VmClassInstance* _this, ArrayList<AbstractFunctionExtArguments>* arguments, ArrayList<IInitializeCompilantUnitProvidor>* exprogs) {
	ERROR_POINT(L"VirtualMachine::interpret");
	CAUSE_ERROR_BY_THROW(L"VirtualMachine::interpret", new Exception(__FILE__, __LINE__));

	initialize(exprogs);

	FunctionArguments args;
	args.setThisPtr(_this);

	method->interpret(&args, this);

	AbstractVmInstance* inst = args.getReturnedValue();

	// floating
	StackFloatingVariableHandler floatHandler(this->gc);
	AbstractVmInstance* vmInst = args.getReturnedValue();
	floatHandler.registerInstance(vmInst);

	// uncaught exception
	checkUncaughtException();
}



ReservedClassRegistory* VirtualMachine::getReservedClassRegistory() const noexcept {
	return this->sc->getReservedClassRegistory();
}

VmTransactionHandler* VirtualMachine::getTransactionHandler() const noexcept {
	return this->trxHandler;
}

void VirtualMachine::checkUncaughtException() {
	if(this->uncaughtException != nullptr){
		return;
	}
	ReservedClassRegistory* reg = getReservedClassRegistory();

	UnicodeString fqn(AbstractExceptionClassDeclare::PACKAGE_NAME);
	fqn.append(L".");
	fqn.append(&ExceptionClassDeclare::NAME);

	AnalyzedClass* exclass = reg->getAnalyzedClass(&fqn);

	this->uncaughtException = catchException(exclass);

	if(this->uncaughtException != nullptr){
		this->gc->registerObject(this->uncaughtException);
	}
}


VmMemoryManager* VirtualMachine::getMemory() noexcept {
	return this->memory;
}

VmMalloc* VirtualMachine::getAlloc() noexcept {
	return this->alloc;
}

void VirtualMachine::analyze() {
	this->sc->analyze(this);
}

bool VirtualMachine::hasError() noexcept {
	return this->sc->hasError();
}

bool VirtualMachine::hasAnalyzeError(int code) noexcept {
	AnalyzeContext* actx = this->sc->getAnalyzeContext();
	return hasAnalyzeError(code, actx);
}

bool VirtualMachine::hasAnalyzeError(int code, AnalyzeContext* actx) noexcept {
	const ArrayList<ValidationError>* list = actx->getErrors();

	bool ret = false;

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		ValidationError* err = list->get(i);

		int cd = err->getErrorCode();
		if(cd == code){
			ret = true;
			break;
		}
	}

	return ret;
}

void VirtualMachine::markStackbyMethod(MethodDeclare *method) {
	this->stackManager->markStackbyMethod(method);
}

void VirtualMachine::markStackEntryPoint(AbstractExpression *exp) {
	this->stackManager->markStackEntryPoint(exp);
}

void VirtualMachine::newStack() {
	VmRootReference* root = this->sc->getRootReference();

	VmStack* stack = new(this) VmStack(root, this);
	this->stackManager->addStack(stack);

	this->gc->registerObject(stack);
}

void VirtualMachine::popStack() {
	VmStack* stack = this->stackManager->top();

	this->stackManager->popStack();

	this->gc->removeObject(stack);
}

VmStack* VirtualMachine::topStack() const noexcept {
	return this->stackManager->top();
}

int VirtualMachine::topStackIndex() const noexcept {
	return this->stackManager->size() - 1;
}

VmStack* VirtualMachine::getStackAt(int pos) const noexcept {
	return this->stackManager->get(pos);
}

void VirtualMachine::clearStack() noexcept {
	VmRootReference* root = this->sc->getRootReference();

	while(!this->stackManager->isEmpty()){
		VmStack* stack = this->stackManager->top();
		this->stackManager->popStack();

		this->gc->removeObject(stack);
		stack->resetOnGc();
	}
}

GcManager* VirtualMachine::getGc() noexcept {
	return this->gc;
}

FunctionArguments* VirtualMachine::getFunctionArguments() const noexcept {
	return this->argsRegister;
}

void VirtualMachine::setFunctionArguments(FunctionArguments* args) noexcept {
	this->argsRegister = args;
}

void VirtualMachine::initialize(ArrayList<IInitializeCompilantUnitProvidor>* exprogs) {
	if(this->initialized){
		return;
	}

	this->sc->initialize(this, exprogs);
	this->initialized = true;
}

void VirtualMachine::setVmRootReference(VmRootReference* rootReference) noexcept {
	this->rootReference = rootReference;
}

VmRootReference* VirtualMachine::getVmRootReference() const noexcept {
	return this->rootReference;
}

SmartContract* VirtualMachine::getSmartContract() const noexcept {
	return this->sc;
}

void VirtualMachine::destroy() noexcept {
	if(this->sc == nullptr || this->sc->getRootReference() == nullptr){
		return;
	}

	if(this->uncaughtException != nullptr){
		this->gc->removeObject(this->uncaughtException);
		delete this->uncaughtException;
		this->uncaughtException = nullptr;
	}

	clearStack();
	this->sc->getRootReference()->clearInnerReferences();
	this->gc->garbageCollect();

	this->sc->clearRootReference(this);

	this->destried = true;
}
void VirtualMachine::releaseMainInstance() noexcept {
	this->sc->releaseMainInstance(this->gc);
}

ExecControlManager* VirtualMachine::getCtrl() const noexcept {
	return this->ctrl;
}

void VirtualMachine::throwException(VmExceptionInstance* exception, const CodeElement* element) noexcept {
	ExecControlManager* ctrl = this->ctrl;

	// make stack trace
	int maxLoop = this->stackManager->size();
	for(int i = maxLoop - 1; i != -1; --i){
		VmStack* stack = this->stackManager->get(i);

		StackTraceElement* element = new(this) StackTraceElement(publishInstanceSerial());

		MethodDeclare* currentMethod = stack->getCurrentMethod();
		AbstractExpression* exp = stack->getEntryPoint();

		if(currentMethod != nullptr){
			element->setCurrentMethod(currentMethod);
			element->setEntryPoint(exp);
			exception->addStacktrace(element);
		}
	}

	exception->setCodeElement(element);

	VmRootReference* rootRef = this->sc->getRootReference();
	AbstractReference* ref = exception->wrap(rootRef, this);
	this->gc->registerObject(ref);

	ExceptionControl* exceptionCtrl = new ExceptionControl(ref);
	ctrl->setInstruction(exceptionCtrl);
}

ObjectReference* VirtualMachine::catchException(AnalyzedClass* exClass) noexcept {
	ExecControlManager* ctrl = this->ctrl;

	ObjectReference* ref = ctrl->getException();
	if(ref == nullptr){
		return nullptr;
	}

	IAbstractVmInstanceSubstance* sub = ref->getInstance();
	VmExceptionInstance* ex = dynamic_cast<VmExceptionInstance*>(sub);

	AnalyzedClass* cls = ex->getAnalyzedClass();
	if(!cls->hasBaseClass(exClass)){
		return nullptr;
	}

	ctrl->consumeException(this);

	return ref;
}

ArrayList<Exception>& VirtualMachine::getExceptions() noexcept {
	return this->exceptions;
}

ExtExceptionObject* VirtualMachine::getUncaughtException() noexcept {
	if(this->uncaughtException == nullptr){
		return nullptr;
	}

	VTableRegistory* vreg = this->sc->getAnalyzeContext()->getVtableRegistory();

	IAbstractVmInstanceSubstance* sub = this->uncaughtException->getInstance();

	UnicodeString oname(L"uncaught");
	AbstractExtObject* extObj = sub->instToClassExtObject(&oname, vreg);

	return dynamic_cast<ExtExceptionObject*>(extObj);
}

ObjectReference* VirtualMachine::getUncaughtExceptionProxy() noexcept {
	return this->uncaughtException;
}

void VirtualMachine::clearUncoughtException() noexcept {
	if(this->uncaughtException != nullptr){
		this->gc->removeObject(this->uncaughtException);
		delete this->uncaughtException;
		this->uncaughtException = nullptr;
	}
}

void VirtualMachine::setCaught(bool caught) noexcept {
	this->caught = caught;
}

bool VirtualMachine::isCaught() const noexcept {
	return this->caught;
}

void VirtualMachine::setSelectPlanner(SelectScanPlanner* planner) noexcept {
	this->selectPlannerList.addElement(planner);
}


void VirtualMachine::popSelectPlanner() noexcept {
	int lastIndex = this->selectPlannerList.size() - 1;
	this->selectPlannerList.remove(lastIndex);
}

SelectScanPlanner* VirtualMachine::getSelectPlanner() const noexcept {
	int lastIndex = this->selectPlannerList.size() - 1;
	return this->selectPlannerList.get(lastIndex);
}

bool VirtualMachine::isSelectPlanning() const noexcept {
	return !this->selectPlannerList.isEmpty();
}

const UnicodeString* VirtualMachine::getCurrentSchema() const noexcept {
	return this->trxHandler->getCurrentSchema();
}

void VirtualMachine::setEscapeTargetCondition(EscapeTargetCondition* cond) noexcept {
	delete this->espaceTargetCondition;
	this->espaceTargetCondition = cond;
}

uint64_t VirtualMachine::publishInstanceSerial() noexcept {
	uint64_t serial = 0;

	if(this->sc != nullptr && this->rootReference != nullptr){
		VmRootReference* rootReference = this->sc->getRootReference();
		serial = rootReference->publishInstanceSerial();
	}

	return serial;
}

AnalyzedClass* VirtualMachine::getStringAnalyzedClass() const noexcept {
	if(this->sc == nullptr){
		return StringClassDeclare::getStaticAnalyzedClass();
	}
	AnalyzeContext* actx = this->sc->getAnalyzeContext();
	TypeResolver* resolver = actx->getTypeResolver();

	AnalyzedType* atype = resolver->findBasicType(&StringClassDeclare::NAME); __STP(atype);
	AnalyzedClass* clazz = atype->getAnalyzedClass();

	return clazz;
}

} /* namespace alinous */
