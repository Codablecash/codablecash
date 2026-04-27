/*
 * SQLPlaceHolder.cpp
 *
 *  Created on: 2019/02/18
 *      Author: iizuka
 */

#include "lang_sql/sql_expression/SQLPlaceHolder.h"
#include "lang/sc_expression/AbstractExpression.h"

#include "engine/sc_analyze/AnalyzedType.h"

#include "scan_select/scan_planner/base/SelectScanPlanner.h"
#include "scan_select/scan_planner/base/ConditionsHolderStackMarker.h"

#include "vm/VirtualMachine.h"

#include "scan_select/scan_condition/params/PlaceHolderScanCondition.h"

#include "scan_select/scan_columns/ScanColumnHolder.h"

#include "scan_select/scan_columns/scan_columns_exp/PlaceHolderScanColumnTarget.h"

#include "cstdint"

using codablecash::SelectScanPlanner;
namespace alinous {

SQLPlaceHolder::SQLPlaceHolder() : AbstractSQLExpression(CodeElement::SQL_EXP_PLACE_HOLDER) {
	this->exp = nullptr;
	this->available = nullptr;
}

SQLPlaceHolder::~SQLPlaceHolder() {
	delete this->exp;
	delete this->available;
}

void SQLPlaceHolder::setExpression(AbstractExpression* exp) noexcept {
	this->exp = exp;
}

void SQLPlaceHolder::setAvailable(AbstractExpression *exp) noexcept {
	this->available = exp;
}

int SQLPlaceHolder::binarySize() const {
	checkNotNull(this->exp);

	int total = sizeof(uint16_t);
	total += this->exp->binarySize();

	total += sizeof(uint8_t);
	if(this->available != nullptr){
		total += this->available->binarySize();
	}

	total += positionBinarySize();

	return total;
}

void SQLPlaceHolder::toBinary(ByteBuffer* out) const {
	checkNotNull(this->exp);

	out->putShort(CodeElement::SQL_EXP_PLACE_HOLDER);
	this->exp->toBinary(out);

	uint8_t bl = (this->available != nullptr) ? 1 : 0;
	out->put(bl);

	if(bl > 0){
		this->available->toBinary(out);
	}

	positionToBinary(out);
}

void SQLPlaceHolder::fromBinary(ByteBuffer* in) {
	CodeElement* element = createFromBinary(in);
	checkIsExp(element);
	this->exp = dynamic_cast<AbstractExpression*>(element);

	uint8_t bl = in->get();
	if(bl > 0){
		element = createFromBinary(in);
		checkIsExp(element);
		this->available = dynamic_cast<AbstractExpression*>(element);
	}

	positionFromBinary(in);
}

void SQLPlaceHolder::preAnalyze(AnalyzeContext* actx) {
	this->exp->setParent(this);
	this->exp->preAnalyze(actx);

	if(this->available != nullptr){
		this->available->setParent(this);
		this->available->preAnalyze(actx);
	}
}

void SQLPlaceHolder::analyzeTypeRef(AnalyzeContext* actx) {
	this->exp->analyzeTypeRef(actx);

	if(this->available != nullptr){
		this->available->analyzeTypeRef(actx);
	}
}

void SQLPlaceHolder::analyze(AnalyzeContext* actx) {
	this->exp->analyze(actx);

	if(this->available != nullptr){
		this->available->analyze(actx);
	}
}

AnalyzedType SQLPlaceHolder::getType(AnalyzeContext* actx) {
	//return this->exp->getType(actx);
	return AnalyzedType();
}

void SQLPlaceHolder::init(VirtualMachine* vm) {
	this->exp->init(vm);

	if(this->available != nullptr){
		this->available->init(vm);
	}
}

AbstractVmInstance* SQLPlaceHolder::interpret(VirtualMachine* vm) {
	SelectScanPlanner* planner = vm->getSelectPlanner();
	ConditionsHolderStackMarker marker(planner->getConditionsStack());

	PlaceHolderScanCondition* cond = new PlaceHolderScanCondition(this->exp);
	cond->setAvailable(this->available);

	planner->push(cond);

	return nullptr;
}

void SQLPlaceHolder::onSelectTarget(VirtualMachine* vm) {
	SelectScanPlanner* planner = vm->getSelectPlanner();
	ScanColumnHolder* colHolder = planner->getColumnHolder();

	PlaceHolderScanColumnTarget* cond = new PlaceHolderScanColumnTarget(this->exp);
	colHolder->push(cond);
}



} /* namespace alinous */
