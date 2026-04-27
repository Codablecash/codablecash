/*
 * VirtualMachine.h
 *
 *  Created on: 2018/12/31
 *      Author: iizuka
 */

#ifndef VM_VIRTUALMACHINE_H_
#define VM_VIRTUALMACHINE_H_
#include <cstdint>

#include "base/ArrayList.h"

namespace codablecash {
class CodableDatabase;
class SelectScanPlanner;
class EscapeTargetCondition;
}
using namespace codablecash;

namespace alinous {
class UnicodeString;

class VmInstanceStack;
class SmartContract;
class VmMemoryManager;
class VmMalloc;
class GcManager;
class VmStackManager;
class VmStack;

class MethodDeclare;
class VmClassInstance;
class FunctionArguments;
class VmRootReference;
class AbstractReference;

class VmExceptionInstance;
class AbstractFunctionExtArguments;
class ExecControlManager;

class Exception;
class CodeElement;
class AnalyzedClass;
class ObjectReference;
class ExtExceptionObject;
class ReservedClassRegistory;

class VmTransactionHandler;
class File;

class AnalyzeContext;
class FunctionScoreCalc;
class MethodScore;
class AnalyzedType;
class AbstractExpression;
class IInitializeCompilantUnitProvidor;

class VirtualMachine {
public:
	explicit VirtualMachine(uint64_t memCapacity);
	virtual ~VirtualMachine();

	void loadSmartContract(SmartContract* sc);
	void analyze();
	bool hasError() noexcept;
	bool hasAnalyzeError(int code) noexcept;
	bool hasAnalyzeError(int code, AnalyzeContext* actx) noexcept;

	VmClassInstance* createScInstance(ArrayList<IInitializeCompilantUnitProvidor>* exprogs);

	MethodDeclare* interpretMainObjectMethod(const UnicodeString* method, ArrayList<AbstractFunctionExtArguments>* arguments);
	MethodDeclare* interpretMainObjectMethod(const UnicodeString* method, ArrayList<AbstractFunctionExtArguments>* arguments, FunctionArguments* args);
	MethodDeclare* interpretMainObjectMethodProxy(VirtualMachine* callerVm, const UnicodeString* method, FunctionArguments* args);

	void interpret(const UnicodeString* method);
	void interpret(MethodDeclare* method, VmClassInstance* _this, ArrayList<AbstractFunctionExtArguments>* arguments, ArrayList<IInitializeCompilantUnitProvidor>* exprogs);

	void markStackbyMethod(MethodDeclare* method);
	void markStackEntryPoint(AbstractExpression* exp);

	void newStack();
	void popStack();
	VmStack* topStack() const noexcept;
	int topStackIndex() const noexcept;
	VmStack* getStackAt(int pos) const noexcept;
	void clearStack() noexcept;

	VmMemoryManager* getMemory() noexcept;
	VmMalloc* getAlloc() noexcept;
	GcManager* getGc() noexcept;

	ExecControlManager* getCtrl() const noexcept;
	void throwException(VmExceptionInstance* exception, const CodeElement* element) noexcept;
	ObjectReference* catchException(AnalyzedClass* exClass) noexcept;
	void releaseMainInstance() noexcept;

	void setFunctionArguments(FunctionArguments* args) noexcept;
	FunctionArguments* getFunctionArguments() const noexcept;

	void setVmRootReference(VmRootReference* rootReference) noexcept;
	VmRootReference* getVmRootReference() const noexcept;

	SmartContract* getSmartContract() const noexcept;

	void initialize(ArrayList<IInitializeCompilantUnitProvidor>* exprogs);
	void destroy() noexcept;

	ArrayList<Exception>& getExceptions() noexcept;
	ExtExceptionObject* getUncaughtException() noexcept;
	ObjectReference* getUncaughtExceptionProxy() noexcept;
	void clearUncoughtException() noexcept;

	ReservedClassRegistory* getReservedClassRegistory() const noexcept;

	// catch statement
	void setCaught(bool caught) noexcept;
	bool isCaught() const noexcept;

	// database and transaction
	VmTransactionHandler* getTransactionHandler() const noexcept;
	void loadDatabase(const File* dbdir, const File* undoDir);

	const CodeElement* getLastElement() const noexcept {
		return lastElement;
	}
	void setLastElement(const CodeElement* lastElement) {
		this->lastElement = lastElement;
	}

	void setSelectPlanner(SelectScanPlanner* planner) noexcept;
	void popSelectPlanner() noexcept;
	SelectScanPlanner* getSelectPlanner() const noexcept;
	bool isSelectPlanning() const noexcept;

	void checkUncaughtException();

	CodableDatabase* getDb() const noexcept {
		return db;
	}

	const UnicodeString* getCurrentSchema() const noexcept;


	EscapeTargetCondition* getEscapeTargetCondition() const noexcept {
		return this->espaceTargetCondition;
	}
	void setEscapeTargetCondition(EscapeTargetCondition* cond) noexcept;

	uint64_t publishInstanceSerial() noexcept;
	AnalyzedClass* getStringAnalyzedClass() const noexcept;

private:
	MethodScore* calcScore(FunctionScoreCalc* calc, const UnicodeString *method, ArrayList<AnalyzedType>* typeList);

private:
	SmartContract* sc;

	VmMemoryManager* memory;
	VmStackManager* stackManager;

	VmMalloc* alloc;
	GcManager* gc;
	ExecControlManager* ctrl;

	FunctionArguments* argsRegister;
	VmRootReference* rootReference;

	bool destried;
	bool initialized;

	ArrayList<Exception> exceptions;
	ObjectReference* uncaughtException;

	const CodeElement* lastElement;

	// catch test
	bool caught;

	// database engine
	CodableDatabase* db;
	VmTransactionHandler* trxHandler;

	ArrayList<SelectScanPlanner> selectPlannerList;

	EscapeTargetCondition* espaceTargetCondition;
};

} /* namespace alinous */

#endif /* VM_VIRTUALMACHINE_H_ */
