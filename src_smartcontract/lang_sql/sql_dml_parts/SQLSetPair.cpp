/*
 * SQLSetPair.cpp
 *
 *  Created on: 2019/02/22
 *      Author: iizuka
 */

#include "lang_sql/sql_dml_parts/SQLSetPair.h"

#include "lang_sql/sql_expression/SQLColumnIdentifier.h"
#include "lang_sql/sql/AbstractSQLExpression.h"

#include "vm/VirtualMachine.h"

#include "scan_select/scan_planner/base/UpdateColumnPolicy.h"
#include "scan_select/scan_planner/base/SelectScanPlanner.h"

#include "scan_select/scan_planner/analyze/AnalyzedScanPlan.h"

#include "scan_select/scan_planner/scanner/factory/AbstractScannerFactory.h"

#include "trx/scan/transaction_scan_result/ScanResultFieldMetadata.h"

#include "scan_select/scan_condition/params/ColumnIdentifierScanParam.h"

#include "trx/scan/transaction_scan_result/ScanResultMetadata.h"
namespace alinous {

SQLSetPair::SQLSetPair() : AbstractSQLPart(CodeElement::SQL_PART_SET_PAIR) {
	this->colId = nullptr;
	this->exp = nullptr;
}

SQLSetPair::~SQLSetPair() {
	delete this->colId;
	delete this->exp;
}

void SQLSetPair::setColumn(SQLColumnIdentifier* colId) noexcept {
	this->colId = colId;
}

void SQLSetPair::setExpression(AbstractSQLExpression* exp) noexcept {
	this->exp = exp;
}

int SQLSetPair::binarySize() const {
	checkNotNull(this->colId);
	checkNotNull(this->exp);

	int total = sizeof(uint16_t);
	total += this->colId->binarySize();
	total += this->exp->binarySize();

	total += positionBinarySize();

	return total;
}

void SQLSetPair::toBinary(ByteBuffer* out) const {
	checkNotNull(this->colId);
	checkNotNull(this->exp);

	out->putShort(CodeElement::SQL_PART_SET_PAIR);
	this->colId->toBinary(out);
	this->exp->toBinary(out);

	positionToBinary(out);
}

void SQLSetPair::fromBinary(ByteBuffer* in) {
	CodeElement* element = createFromBinary(in);
	checkKind(element, CodeElement::SQL_EXP_COLUMN_ID);
	this->colId = dynamic_cast<SQLColumnIdentifier*>(element);

	element = createFromBinary(in);
	checkIsSQLExp(element);
	this->exp = dynamic_cast<AbstractSQLExpression*>(element);

	positionFromBinary(in);
}

void SQLSetPair::preAnalyze(AnalyzeContext* actx) {
	this->colId->setParent(this);
	this->colId->preAnalyze(actx);

	this->exp->setParent(this);
	this->exp->preAnalyze(actx);
}

void SQLSetPair::analyzeTypeRef(AnalyzeContext* actx) {
	this->colId->analyzeTypeRef(actx);

	this->exp->analyzeTypeRef(actx);
}

void SQLSetPair::analyze(AnalyzeContext* actx) {
	this->colId->analyze(actx);

	this->exp->analyze(actx);
}

void SQLSetPair::init(VirtualMachine* vm) {
	this->colId->init(vm);

	this->exp->init(vm);
}

UpdateColumnPolicy* SQLSetPair::makeUpdateColumnPolicy(VirtualMachine* vm) {
	SelectScanPlanner* planner = vm->getSelectPlanner();

	UpdateColumnPolicy* policy = new UpdateColumnPolicy();

	// column
	ColumnIdentifierScanParam colIdParam(this->colId);
	colIdParam.analyzeConditions(vm, planner);

	AbstractScannerFactory* scanFactory = planner->getPlan()->getScanFactory();
	const ScanResultMetadata* metadata = scanFactory->getMetadata();
	const ScanResultFieldMetadata* fld = metadata->findField(&colIdParam);
	assert(fld != nullptr);

	policy->setFieldMetadata(new ScanResultFieldMetadata(*fld));

	// value
	this->exp->interpret(vm);
	AbstractScanConditionElement* vp = planner->pop();
	policy->setValueProvider(vp);

	return policy;
}

} /* namespace alinous */
