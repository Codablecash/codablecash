/*
 * SQLHaving.cpp
 *
 *  Created on: 2019/02/20
 *      Author: iizuka
 */

#include "lang_sql/sql_dml_parts/SQLHaving.h"
#include "lang_sql/sql/AbstractSQLExpression.h"

#include "scan_select/scan_planner/base/ConditionsHolder.h"
#include "scan_select/scan_planner/base/SelectScanPlanner.h"

#include "scan_select/scan_condition/base/ScanConditionCast.h"
#include "scan_select/scan_condition/base/RootScanCondition.h"

#include "vm/VirtualMachine.h"


using namespace alinous;

namespace alinous {

SQLHaving::SQLHaving() : AbstractSQLPart(CodeElement::SQL_PART_HAVING) {
	this->exp = nullptr;
}

SQLHaving::~SQLHaving() {
	delete this->exp;
}

void SQLHaving::setExpression(AbstractSQLExpression* exp) noexcept {
	this->exp = exp;
}

void SQLHaving::preAnalyze(AnalyzeContext* actx) {
	if(this->exp != nullptr){
		this->exp->setParent(this);
		this->exp->preAnalyze(actx);
	}
}

void SQLHaving::analyzeTypeRef(AnalyzeContext* actx) {
	if(this->exp != nullptr){
		this->exp->analyzeTypeRef(actx);
	}
}

void SQLHaving::analyze(AnalyzeContext* actx) {
	if(this->exp != nullptr){
		this->exp->analyze(actx);
	}
}


void SQLHaving::init(VirtualMachine* vm) {
	if(this->exp != nullptr){
		this->exp->init(vm);
	}
}

AbstractVmInstance* SQLHaving::interpret(VirtualMachine* vm){
	if(this->exp != nullptr){
		this->exp->interpret(vm);

		SelectScanPlanner* planner = vm->getSelectPlanner();
		ConditionsHolder* cholder = planner->getConditions();

		RootScanCondition* root = cholder->getHavingRoot();

		AbstractScanConditionElement* element = cholder->pop();
		AbstractScanCondition* cond = ScanConditionCast::toAbstractScanCondition(element, vm, this);

		root->addCondition(cond);
	}

	return nullptr;
}

int SQLHaving::binarySize() const {
	checkNotNull(this->exp);

	int total = sizeof(uint16_t);
	total += this->exp->binarySize();

	total += positionBinarySize();

	return total;
}

void SQLHaving::toBinary(ByteBuffer* out) const {
	checkNotNull(this->exp);

	out->putShort(CodeElement::SQL_PART_HAVING);
	this->exp->toBinary(out);

	positionToBinary(out);
}

void SQLHaving::fromBinary(ByteBuffer* in) {
	CodeElement* element = createFromBinary(in);
	checkIsSQLExp(element);
	this->exp = dynamic_cast<AbstractSQLExpression*>(element);

	positionFromBinary(in);
}

} /* namespace alinous */
