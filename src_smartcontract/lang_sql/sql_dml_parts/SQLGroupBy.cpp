/*
 * SQLGroupBy.cpp
 *
 *  Created on: 2019/02/16
 *      Author: iizuka
 */

#include "lang_sql/sql_dml_parts/SQLGroupBy.h"
#include "lang_sql/sql_dml_parts/SQLColumnsList.h"
#include "lang_sql/sql_dml_parts/SQLHaving.h"

#include "vm/VirtualMachine.h"

#include "scan_select/scan_planner/base/SelectScanPlanner.h"
#include "scan_select/scan_planner/base/GroupByPlanner.h"
#include "scan_select/scan_planner/base/ConditionsHolder.h"

#include "scan_select/scan_columns/AbstractScanColumnsTarget.h"
#include "scan_select/scan_columns/ScanColumnHolder.h"

#include "lang_sql/sql_expression/SQLColumnIdentifier.h"

#include "scan_select/scan_condition/base/AbstractScanConditionElement.h"

#include "scan_select/scan_columns/ScanColumn.h"


using namespace codablecash;

namespace alinous {

SQLGroupBy::SQLGroupBy() : AbstractSQLPart(CodeElement::SQL_PART_GROUP_BY) {
	this->list = nullptr;
	this->having = nullptr;
}

SQLGroupBy::~SQLGroupBy() {
	delete this->list;
	delete this->having;
}

void SQLGroupBy::setList(SQLColumnsList* list) noexcept {
	this->list = list;
}

void SQLGroupBy::setHaving(SQLHaving* having) noexcept {
	this->having = having;
}

void SQLGroupBy::preAnalyze(AnalyzeContext* actx) {
	this->list->setParent(this);
	this->list->preAnalyze(actx);

	if(this->having != nullptr){
		this->having->setParent(this);
		this->having->preAnalyze(actx);
	}
}

void SQLGroupBy::analyzeTypeRef(AnalyzeContext* actx) {
	this->list->analyzeTypeRef(actx);

	if(this->having != nullptr){
		this->having->analyzeTypeRef(actx);
	}
}

void SQLGroupBy::analyze(AnalyzeContext* actx) {
	this->list->analyze(actx);

	if(this->having != nullptr){
		this->having->analyze(actx);
	}
}

void SQLGroupBy::init(VirtualMachine* vm) {
	this->list->init(vm);
	if(this->having != nullptr){
		this->having->init(vm);
	}
}

AbstractVmInstance* SQLGroupBy::interpret(VirtualMachine* vm) {
	SelectScanPlanner* planner = vm->getSelectPlanner();

	GroupByPlanner* groupPlan = planner->getGroupPlan();

	const ArrayList<SQLColumnIdentifier>* cols = this->list->getList();

	int maxLoop = cols->size();
	for(int i = 0; i != maxLoop; ++i){
		SQLColumnIdentifier* colId = cols->get(i);

		ScanColumn* param = new ScanColumn(colId);

		groupPlan->addColumn(param);
	}

	if(this->having != nullptr){
		this->having->interpret(vm);

		ConditionsHolder* conditions = planner->getConditions();
		conditions->analyzeHavingConditions(vm, planner);
	}

	return nullptr;
}

int SQLGroupBy::binarySize() const {
	checkNotNull(this->list);

	int total = sizeof(uint16_t);
	total += this->list->binarySize();

	total += sizeof(uint8_t);
	if(this->having != nullptr){
		total += this->having->binarySize();
	}

	total += positionBinarySize();

	return total;
}

void SQLGroupBy::toBinary(ByteBuffer* out) const {
	checkNotNull(this->list);

	out->putShort(CodeElement::SQL_PART_GROUP_BY);
	this->list->toBinary(out);

	out->put(this->having != nullptr ? 1 : 0);
	if(this->having != nullptr){
		this->having->toBinary(out);
	}

	positionToBinary(out);
}

void SQLGroupBy::fromBinary(ByteBuffer* in) {
	CodeElement* element = createFromBinary(in);
	checkKind(element, CodeElement::SQL_PART_COLUMN_LIST);
	this->list = dynamic_cast<SQLColumnsList*>(element);

	int8_t bl = in->get();
	if(bl == 1){
		element = createFromBinary(in);
		checkKind(element, CodeElement::SQL_PART_HAVING);
		this->having = dynamic_cast<SQLHaving*>(element);
	}

	positionFromBinary(in);
}

} /* namespace alinous */
