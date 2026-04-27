/*
 * DeleteStatement.cpp
 *
 *  Created on: 2019/02/16
 *      Author: iizuka
 */

#include "lang_sql/sql_dml/DeleteStatement.h"

#include "lang_sql/sql_dml_parts/SQLFrom.h"
#include "lang_sql/sql_dml_parts/SQLWhere.h"

#include "scan_select/scan_planner/base/SelectScanPlanner.h"
#include "scan_select/scan_planner/base/TablesHolder.h"

#include "vm/VirtualMachine.h"
#include "vm/VmSelectPlannerSetter.h"

#include "vm/vm_trx/VmTransactionHandler.h"

#include "trx/transaction/CdbTransaction.h"

#include "trx/transaction_exception/DatabaseExceptionClassDeclare.h"

#include "base/Exception.h"
#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "instance/instance_exception/ExceptionInterrupt.h"

#include "lang_sql/sql/AbstractJoinPart.h"

#include "schema_table/record/table_record/CdbTableIdentifier.h"

#include "scan_select/scan_planner/analyze/AnalyzedScanPlan.h"

#include "scan_select/scan_planner/scanner/factory/AbstractScannerFactory.h"

#include "scan_select/scan_table/TableScanTarget.h"

#include "schema_table/table/CdbTable.h"

#include "trx/scan/transaction_scan_result/ScanResultExecutor.h"

#include "trx/transaction_cache/OidKeyRecordCache.h"

#include "trx/transaction_log/DeleteLog.h"

#include "trx/transaction_cache/OidKeyRecordCacheScanner.h"

#include "schema_table/record/table_record/CdbRecord.h"

#include "schema_table/schema/SchemaManager.h"

#include "engine/CdbOid.h"
#include "engine/CodableDatabase.h"


namespace alinous {

DeleteStatement::DeleteStatement() : AbstractSQLStatement(CodeElement::DML_STMT_DELETE) {
	this->from = nullptr;
	this->where = nullptr;

	this->lastSchemaVersion = 0;
	this->planner = nullptr;
	this->tableIdentifier = nullptr;
}

DeleteStatement::~DeleteStatement() {
	delete this->from;
	delete this->where;

	delete this->planner;
	delete this->tableIdentifier;
}

void DeleteStatement::preAnalyze(AnalyzeContext* actx) {
	if(this->from != nullptr){
		this->from->setParent(this);
	}

	if(this->where != nullptr){
		this->where->setParent(this);
		this->where->preAnalyze(actx);
	}
}

void DeleteStatement::analyzeTypeRef(AnalyzeContext* actx) {
	if(this->where != nullptr){
		this->where->analyzeTypeRef(actx);
	}
}

void DeleteStatement::analyze(AnalyzeContext* actx) {
	if(this->where != nullptr){
		this->where->analyze(actx);
	}
}

void DeleteStatement::setFrom(SQLFrom* from) noexcept {
	this->from = from;
}

void DeleteStatement::setWhere(SQLWhere* where) noexcept {
	this->where = where;
}

int DeleteStatement::binarySize() const {
	checkNotNull(this->from);

	int total = sizeof(uint16_t);
	total += this->from->binarySize();

	total += sizeof(uint8_t);
	if(this->where != nullptr){
		total += this->where->binarySize();
	}

	total += positionBinarySize();

	return total;
}

void DeleteStatement::toBinary(ByteBuffer* out) const {
	checkNotNull(this->from);

	out->putShort(CodeElement::DML_STMT_DELETE);
	this->from->toBinary(out);

	out->put(this->where != nullptr ? 1 : 0);
	if(this->where != nullptr){
		this->where->toBinary(out);
	}

	positionToBinary(out);
}

void DeleteStatement::fromBinary(ByteBuffer* in) {
	CodeElement* element = createFromBinary(in);
	checkKind(element, CodeElement::SQL_PART_FROM);
	this->from = dynamic_cast<SQLFrom*>(element);

	int8_t bl = in->get();
	if(bl == 1){
		 element = createFromBinary(in);
		 checkKind(element, CodeElement::SQL_PART_WHERE);
		 this->where = dynamic_cast<SQLWhere*>(element);
	}

	positionFromBinary(in);
}

void DeleteStatement::init(VirtualMachine* vm) {
	if(this->where != nullptr){
		this->where->init(vm);
	}
}

void DeleteStatement::interpret(VirtualMachine* vm) {
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
				buildPlanner(vm, currentVer);
				updateSchemaInfo(vm, trxHandler);
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

void DeleteStatement::doUpdateScan(VirtualMachine* vm,	CdbTransaction* trx) {
	ScanResultExecutor* exec = this->planner->executeQueryScan(vm); __STP(exec);

	AnalyzedScanPlan* plan = this->planner->getPlan();
	AbstractScannerFactory* factory = plan->getScanFactory();
	const ScanResultMetadata* metadata = factory->getMetadata();

	OidKeyRecordCache* cache = exec->getOidKeyRecordCache();

	CodableDatabase* db = vm->getDb();
	SchemaManager* scManager = db->getSchemaManager();
	uint64_t commitId = scManager->newCommitId();

	DeleteLog* cmd = new DeleteLog(commitId); __STP(cmd);
	cmd->setTable(new CdbTableIdentifier(*this->tableIdentifier));

	OidKeyRecordCacheScanner* scanner = cache->getRecordScanner(); __STP(scanner);
	while(scanner->hasNext()){
		CdbRecord* record = scanner->next(); __STP(record);
		const CdbOid* oid = record->getOid();

		cmd->addOid(oid->copy());
	}

	trx->_delete(__STP_MV(cmd));
}

void DeleteStatement::updateSchemaInfo(VirtualMachine* vm,	VmTransactionHandler* trxHandler) {
	TablesHolder* tableHodler = this->planner->getTablesHolder();
	AbstractScanTableTarget* abtarget = tableHodler->getScanTarget(0);

	TableScanTarget* target = dynamic_cast<TableScanTarget*>(abtarget);
	const CdbTable* table = target->getTable();

	delete this->tableIdentifier;
	this->tableIdentifier = new CdbTableIdentifier();

	const UnicodeString* schema = table->getSchemaName();
	const UnicodeString* tableName = table->getName();

	this->tableIdentifier->setSchema(new UnicodeString(schema));
	this->tableIdentifier->setTable(new UnicodeString(tableName));
}

void DeleteStatement::buildPlanner(VirtualMachine* vm, uint64_t currentVer) {
	delete this->planner;
	this->planner = new SelectScanPlanner();

	VmSelectPlannerSetter setter(vm, this->planner);

	// From part
	AbstractJoinPart* tablePart = this->from->getTablePart();
	tablePart->interpret(vm);
	TablesHolder* tableHolder = this->planner->getTablesHolder();
	if(!tableHolder->isEmpty()){
		AbstractScanTableTarget* target = tableHolder->pop();
		tableHolder->addScanTarget(target);
	}

	if(this->where != nullptr){
		this->where->interpret(vm);
	}

	this->planner->makeplan(vm);
}


} /* namespace alinous */
