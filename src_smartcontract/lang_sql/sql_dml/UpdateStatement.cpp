/*
 * UpdateStatement.cpp
 *
 *  Created on: 2019/02/16
 *      Author: iizuka
 */

#include "lang_sql/sql_dml/UpdateStatement.h"
#include "lang_sql/sql_dml_parts/SQLWhere.h"
#include "lang_sql/sql_dml_parts/SQLSet.h"

#include "lang_sql/sql_join_parts/TableIdentifier.h"

#include "scan_select/scan_planner/base/SelectScanPlanner.h"

#include "vm/VirtualMachine.h"
#include "vm/VmSelectPlannerSetter.h"

#include "trx/transaction/CdbTransaction.h"

#include "trx/transaction_log/UpdateLog.h"

#include "trx/transaction_exception/DatabaseExceptionClassDeclare.h"

#include "vm/vm_trx/VmTransactionHandler.h"

#include "base/Exception.h"
#include "base/StackRelease.h"

#include "instance/instance_exception/ExceptionInterrupt.h"

#include "scan_select/scan_planner/base/TablesHolder.h"
#include "scan_select/scan_planner/base/UpdateColumnPlanner.h"

#include "trx/transaction_cache/OidKeyRecordCache.h"
#include "trx/transaction_cache/OidKeyRecordCacheScanner.h"

#include "trx/scan/transaction_scan_result/ScanResultExecutor.h"

#include "schema_table/record/table_record/CdbRecord.h"
#include "schema_table/record/table_record/CdbTableIdentifier.h"

#include "schema_table/schema/SchemaManager.h"

#include "base/UnicodeString.h"

#include "scan_select/scan_planner/analyze/AnalyzedScanPlan.h"

#include "scan_select/scan_planner/scanner/factory/AbstractScannerFactory.h"

#include "engine_lock/ReadLockHandle.h"
#include "engine_lock/StackDbLockUnlocker.h"

#include "engine/CodableDatabase.h"


namespace alinous {

UpdateStatement::UpdateStatement() : AbstractSQLStatement(CodeElement::DML_STMT_UPDATE) {
	this->tableId = nullptr;
	this->set = nullptr;
	this->where = nullptr;

	this->lastSchemaVersion = 0;
	this->planner = nullptr;
	this->updatePlanner = nullptr;
	this->tableIdentifier = nullptr;
}

UpdateStatement::~UpdateStatement() {
	delete this->tableId;
	delete this->set;
	delete this->where;
	delete this->planner;
	delete this->updatePlanner;
	delete this->tableIdentifier;
}

void UpdateStatement::preAnalyze(AnalyzeContext* actx) {
	if(this->tableId != nullptr){
		this->tableId->setParent(this);
		this->tableId->preAnalyze(actx);
	}

	if(this->set != nullptr){
		this->set->setParent(this);
		this->set->preAnalyze(actx);
	}

	if(this->where != nullptr){
		this->where->setParent(this);
		this->where->preAnalyze(actx);
	}
}

void UpdateStatement::analyzeTypeRef(AnalyzeContext* actx) {
	if(this->tableId != nullptr){
		this->tableId->analyzeTypeRef(actx);
	}

	if(this->set != nullptr){
		this->set->analyzeTypeRef(actx);
	}

	if(this->where != nullptr){
		this->where->analyzeTypeRef(actx);
	}
}

void UpdateStatement::analyze(AnalyzeContext* actx) {
	if(this->tableId != nullptr){
		this->tableId->analyze(actx);
	}

	if(this->set != nullptr){
		this->set->analyze(actx);
	}

	if(this->where != nullptr){
		this->where->analyze(actx);
	}
}

void UpdateStatement::setTable(TableIdentifier* tableId) noexcept {
	this->tableId = tableId;
}

void UpdateStatement::setSet(SQLSet* set) noexcept {
	this->set = set;
}

void UpdateStatement::setWhere(SQLWhere* where) noexcept {
	this->where = where;
}

int UpdateStatement::binarySize() const {
	checkNotNull(this->tableId);
	checkNotNull(this->set);

	int total = sizeof(uint16_t);
	total += this->tableId->binarySize();
	total += this->set->binarySize();

	total += sizeof(uint8_t);
	if(this->where != nullptr){
		total += this->where->binarySize();
	}

	total += positionBinarySize();

	return total;
}

void UpdateStatement::toBinary(ByteBuffer* out) const {
	checkNotNull(this->tableId);
	checkNotNull(this->set);

	out->putShort(CodeElement::DML_STMT_UPDATE);
	this->tableId->toBinary(out);
	this->set->toBinary(out);

	out->put(this->where != nullptr ? 1 : 0);
	if(this->where != nullptr){
		this->where->toBinary(out);
	}

	positionToBinary(out);
}

void UpdateStatement::fromBinary(ByteBuffer* in) {
	CodeElement* element = createFromBinary(in);
	checkKind(element, CodeElement::SQL_EXP_TABLE_ID);
	this->tableId = dynamic_cast<TableIdentifier*>(element);

	element = createFromBinary(in);
	checkKind(element, CodeElement::SQL_PART_SET);
	this->set = dynamic_cast<SQLSet*>(element);

	int8_t bl = in->get();
	if(bl == 1){
		 element = createFromBinary(in);
		 checkKind(element, CodeElement::SQL_PART_WHERE);
		 this->where = dynamic_cast<SQLWhere*>(element);
	}

	positionFromBinary(in);
}

void UpdateStatement::init(VirtualMachine* vm) {
	if(this->tableId != nullptr){
		this->tableId->init(vm);
	}

	if(this->set != nullptr){
		this->set->init(vm);
	}

	if(this->where != nullptr){
		this->where->init(vm);
	}
}

void UpdateStatement::interpret(VirtualMachine* vm) {
	VmTransactionHandler* trxHandler = vm->getTransactionHandler();
	CdbTransaction* trx = trxHandler->getTransaction();

	bool tmptrx = false;
	if(trx == nullptr){
		trxHandler->begin();
		trx = trxHandler->getTransaction();
		tmptrx = true;
	}

	try{
		uint64_t currentVer = trxHandler->getSchemaObjectVersionId();
		if(currentVer > this->lastSchemaVersion){
			try{
				CodableDatabase* db = vm->getDb();
				ReadLockHandle* lockH = db->databaseReadLock();
				StackDbLockUnlocker unclocker(lockH);

				updateSchemaInfo(vm, trxHandler);
				buildPlanner(vm, currentVer);
			}
			catch(ExceptionInterrupt* e){ // exception is already thrown
				trxHandler->rollback(true);

				delete e;

				return;
			}
		}

		doUpdateScan(vm, trx);
	}
	catch(Exception* e){
		trxHandler->rollback(true);

		DatabaseExceptionClassDeclare::throwException(e->getMessage(), vm, this);
		delete e;

		return;
	}

	if(tmptrx){
		trxHandler->commit();
	}
}

void UpdateStatement::updateSchemaInfo(VirtualMachine* vm,	VmTransactionHandler* trxHandler) {
	delete this->tableIdentifier;
	this->tableIdentifier = new CdbTableIdentifier();

	const UnicodeString* schema = this->tableId->getSchema();
	const UnicodeString* tableName = this->tableId->getTableName();
	if(schema == nullptr){
		schema = vm->getCurrentSchema();
	}

	this->tableIdentifier->setSchema(new UnicodeString(schema));
	this->tableIdentifier->setTable(new UnicodeString(tableName));
}

void UpdateStatement::doUpdateScan(VirtualMachine* vm, CdbTransaction* trx) {
	CodableDatabase* db = vm->getDb();
	ReadLockHandle* lockH = db->databaseReadLock();
	StackDbLockUnlocker unclocker(lockH);

	ScanResultExecutor* exec = this->planner->executeQueryScan(vm); __STP(exec);

	AnalyzedScanPlan* plan = this->planner->getPlan();
	AbstractScannerFactory* factory = plan->getScanFactory();
	const ScanResultMetadata* metadata = factory->getMetadata();


	OidKeyRecordCache* cache = exec->getOidKeyRecordCache();

	SchemaManager* scManager = db->getSchemaManager();
	uint64_t commitId = scManager->newCommitId();

	UpdateLog* cmd = new UpdateLog(commitId); __STP(cmd);
	cmd->setTable(new CdbTableIdentifier(*this->tableIdentifier));

	OidKeyRecordCacheScanner* scanner = cache->getRecordScanner(); __STP(scanner);
	while(scanner->hasNext()){
		CdbRecord* record = scanner->next(); __STP(record);

		doUpdate(cmd, record, vm, trx, metadata);
	}

	trx->update(__STP_MV(cmd));
}

void UpdateStatement::doUpdate(UpdateLog* cmd, const CdbRecord* record, VirtualMachine* vm, CdbTransaction* trx, const ScanResultMetadata* metadata) {
	CdbRecord* updatedRecord = this->updatePlanner->createUpdatedRecord(vm, record, metadata);

	cmd->addRecord(updatedRecord);
}


void UpdateStatement::buildPlanner(VirtualMachine* vm, uint64_t currentVer) {
	delete this->planner, this->planner = nullptr;
	this->planner = new SelectScanPlanner();

	VmSelectPlannerSetter setter(vm, this->planner);

	// From part
	this->tableId->interpret(vm);
	TablesHolder* tableHolder = this->planner->getTablesHolder();
	if(!tableHolder->isEmpty()){
		AbstractScanTableTarget* target = tableHolder->pop();
		tableHolder->addScanTarget(target);
	}

	if(this->where != nullptr){
		this->where->interpret(vm);
	}

	this->planner->makeplan(vm);

	// SET
	delete this->updatePlanner, this->updatePlanner = nullptr;
	this->updatePlanner = this->set->makeUpdateColumnPlanner(vm);
}

} /* namespace alinous */
