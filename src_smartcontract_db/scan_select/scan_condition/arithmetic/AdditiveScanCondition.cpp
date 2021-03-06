/*
 * AdditiveScanCondition.cpp
 *
 *  Created on: 2020/07/31
 *      Author: iizuka
 */

#include "scan_select/scan_condition/arithmetic/AdditiveScanCondition.h"

#include "engine/sc/CodeElement.h"

#include "base/UnicodeString.h"

#include "lang_sql/sql_expression/SQLAdditiveExpression.h"

#include "scan_select/scan_planner/scanner/ctx/FilterConditionDitector.h"
#include "scan_select/scan_planner/scanner/ctx/FilterConditionStackMarker.h"

namespace codablecash {

AdditiveScanCondition::AdditiveScanCondition() : AbstractScanCondition(CodeElement::SQL_EXP_ADDITIVE), operations(2) {
	this->str = nullptr;
}

AdditiveScanCondition::~AdditiveScanCondition() {
	this->list.deleteElements();
	resetStr();
}

void AdditiveScanCondition::addOperand(IValueProvider* vp) noexcept {
	this->list.addElement(vp);
	resetStr();
}

void AdditiveScanCondition::addOperator(uint8_t op) noexcept {
	this->operations.addElement(op);
	resetStr();
}

const UnicodeString* AdditiveScanCondition::toStringCode() noexcept {
	if(this->str == nullptr){
		resetStr();
		this->str = new UnicodeString(L"");

		int maxLoop = this->list.size();
		for(int i = 0; i != maxLoop; ++i){
			if(i > 0){
				int pos = i - 1;
				uint8_t op = this->operations.get(pos);

				if(op == SQLAdditiveExpression::ADD){
					this->str->append(L" + ");
				}
				else if(op == SQLAdditiveExpression::SUB){
					this->str->append(L" - ");
				}
			}

			IValueProvider* vp = this->list.get(i);
			AbstractScanConditionElement* el = dynamic_cast<AbstractScanConditionElement*>(vp);

			this->str->append(el->toStringCode());
		}

	}

	return this->str;
}

void AdditiveScanCondition::analyzeConditions(VirtualMachine* vm, SelectScanPlanner* planner) {
	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		IValueProvider* vp = this->list.get(i);

		vp->analyzeConditions(vm, planner);
	}
}

AbstractScanCondition* AdditiveScanCondition::cloneCondition() const noexcept {
	return dynamic_cast<AdditiveScanCondition*>(clone());
}

bool AdditiveScanCondition::isFilterable(VirtualMachine* vm,
		SelectScanPlanner* planner, FilterConditionDitector* detector) const noexcept {
	bool result = true;

	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		IValueProvider* vp = this->list.get(i);

		if(!vp->isFilterable(vm, planner, detector)){
			result = false;
			break;
		}
	}

	return result;
}

void AdditiveScanCondition::detectFilterConditions(VirtualMachine* vm,
		SelectScanPlanner* planner, FilterConditionDitector* detector) {
	FilterConditionStackMarker marker(detector->getStack());

	if(isFilterable(vm, planner, detector)){
		detector->push(cloneCondition());
	}
}

void AdditiveScanCondition::detectIndexCondition(VirtualMachine* vm, SelectScanPlanner* planner,
		TableIndexDetector* detector) {
}

void AdditiveScanCondition::resetStr() noexcept {
	if(this->str != nullptr){
		delete this->str;
		this->str = nullptr;
	}
}

IValueProvider* AdditiveScanCondition::clone() const noexcept {
	AdditiveScanCondition* cond = new AdditiveScanCondition();

	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		IValueProvider* vp = this->list.get(i);

		cond->addOperand(vp->clone());
	}

	maxLoop = this->operations.size();
	for(int i = 0; i != maxLoop; ++i){
		uint8_t op = this->operations.get(i);

		cond->addOperator(op);
	}

	return cond;
}


} /* namespace codablecash */
