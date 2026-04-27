/*
 * SQLDistinctArgument.cpp
 *
 *  Created on: 2021/11/02
 *      Author: iizuka
 */

#include "lang_sql/sql_expression/SQLDistinctArgument.h"

#include "engine/sc_analyze/AnalyzedType.h"

#include "scan_select/scan_columns/ScanColumnHolder.h"
#include "scan_select/scan_planner/base/SelectScanPlanner.h"

#include "vm/VirtualMachine.h"

#include "scan_select/scan_planner/base/ConditionsHolderStackMarker.h"

#include "scan_select/scan_condition/params/ColumnIdentifierScanParam.h"

#include "engine/sc_analyze/AnalyzeContext.h"
#include "engine/sc_analyze/ValidationError.h"

#include "lang_sql/sql_expression/SQLColumnIdentifier.h"

#include "scan_select/scan_columns/ScanColumn.h"
using namespace codablecash;

namespace alinous {

SQLDistinctArgument::SQLDistinctArgument() : AbstractSQLExpression(CodeElement::SQL_EXP_DISTINCT_ARGUMENT) {
	this->exp = nullptr;
}

SQLDistinctArgument::~SQLDistinctArgument() {
	delete this->exp;
}

int SQLDistinctArgument::binarySize() const {
	checkNotNull(this->exp);

	int total = sizeof(uint16_t);
	total += this->exp->binarySize();

	total += positionBinarySize();

	return total;
}

void SQLDistinctArgument::toBinary(ByteBuffer* out) const {
	checkNotNull(this->exp);

	out->putShort(CodeElement::SQL_EXP_DISTINCT_ARGUMENT);
	this->exp->toBinary(out);

	positionToBinary(out);
}

void SQLDistinctArgument::fromBinary(ByteBuffer* in) {
	CodeElement* el = CodeElement::createFromBinary(in);

	checkIsSQLExp(el);
	this->exp = dynamic_cast<AbstractSQLExpression*>(el);

	positionFromBinary(in);
}

void SQLDistinctArgument::preAnalyze(AnalyzeContext* actx) {
	this->exp->setParent(this);
	this->exp->preAnalyze(actx);
}

void SQLDistinctArgument::analyzeTypeRef(AnalyzeContext* actx) {
	this->exp->analyzeTypeRef(actx);
}

void SQLDistinctArgument::analyze(AnalyzeContext* actx) {
	short kind = this->exp->getKind();
	if(kind != CodeElement::SQL_EXP_COLUMN_ID){
		actx->addValidationError(ValidationError::SQL_CONDITION_WRONG_DISTINCT_USE, this, L"Wrong use of DISTINCT", {});

		return;
	}

	this->exp->analyze(actx);
}

AnalyzedType SQLDistinctArgument::getType(AnalyzeContext* actx) {
	return AnalyzedType();
}

void SQLDistinctArgument::init(VirtualMachine* vm) {
	this->exp->init(vm);
}

AbstractVmInstance* SQLDistinctArgument::interpret(VirtualMachine* vm) {
	SelectScanPlanner* planner = vm->getSelectPlanner();
	ConditionsHolderStackMarker marker(planner->getConditionsStack());

	SQLColumnIdentifier* colId = dynamic_cast<SQLColumnIdentifier*>(this->exp);

	ColumnIdentifierScanParam* param = new ColumnIdentifierScanParam(colId);
	param->setDistinct(true);

	planner->push(param);

	return nullptr;
}

void SQLDistinctArgument::onSelectTarget(VirtualMachine* vm) {
	SelectScanPlanner* planner = vm->getSelectPlanner();
	ScanColumnHolder* colHolder = planner->getColumnHolder();

	SQLColumnIdentifier* colId = dynamic_cast<SQLColumnIdentifier*>(this->exp);

	ScanColumn* col = new ScanColumn(colId);
	col->setDistinct(true);
	colHolder->push(col);
}

} /* namespace alinous */
