/*
 * BetweenScanCondition.cpp
 *
 *  Created on: 2020/08/05
 *      Author: iizuka
 */

#include "scan_select/scan_condition/exp/BetweenScanCondition.h"

#include "scan_select/scan_condition/base/IValueProvider.h"

#include "engine/sc/CodeElement.h"

#include "base/UnicodeString.h"

#include "scan_select/scan_planner/scanner/ctx/FilterConditionDitector.h"
#include "scan_select/scan_planner/scanner/ctx/FilterConditionStackMarker.h"

using namespace alinous;

namespace codablecash {

BetweenScanCondition::BetweenScanCondition() : AbstractScanCondition(CodeElement::SQL_EXP_BETWEEN) {
	this->left = nullptr;
	this->start = nullptr;
	this->end = nullptr;

	this->str = nullptr;
}

BetweenScanCondition::~BetweenScanCondition() {
	delete this->left;
	delete this->start;
	delete this->end;

	resetStr();
}

void BetweenScanCondition::setLeft(IValueProvider* left) noexcept {
	this->left = left;
	resetStr();
}

void BetweenScanCondition::setStart(IValueProvider* start) noexcept {
	this->start = start;
	resetStr();
}

void BetweenScanCondition::setEnd(IValueProvider* end) noexcept {
	this->end = end;
	resetStr();
}

const UnicodeString* BetweenScanCondition::toStringCode() noexcept {
	if(this->str == nullptr){
		resetStr();
		this->str = new UnicodeString(L"");

		AbstractScanConditionElement* el = dynamic_cast<AbstractScanConditionElement*>(this->left);
		this->str->append(el->toStringCode());

		this->str->append(L" BETWEEN ");

		el = dynamic_cast<AbstractScanConditionElement*>(this->start);
		this->str->append(el->toStringCode());

		this->str->append(L" AND ");

		el = dynamic_cast<AbstractScanConditionElement*>(this->end);
		this->str->append(el->toStringCode());
	}

	return this->str;
}

AbstractScanCondition* BetweenScanCondition::cloneCondition() const noexcept {
	BetweenScanCondition* cond = new BetweenScanCondition();

	cond->setLeft(this->left->clone());
	cond->setStart(this->start->clone());
	cond->setEnd(this->end->clone());

	return cond;
}

void BetweenScanCondition::detectFilterConditions(VirtualMachine* vm,
		SelectScanPlanner* planner, FilterConditionDitector* detector) {
	FilterConditionStackMarker marker(detector->getStack());

	if(this->left->isFilterable(vm, planner, detector) &&
			this->start->isFilterable(vm, planner, detector) &&
			this->end->isFilterable(vm, planner, detector)){
		detector->push(cloneCondition());
	}
}

void BetweenScanCondition::detectIndexCondition(VirtualMachine* vm,
		SelectScanPlanner* planner, TableIndexDetector* detector) {
	// FIXME detectIndexCondition
}


void BetweenScanCondition::resetStr() noexcept {
	if(this->str != nullptr){
		delete this->str;
		this->str = nullptr;
	}
}

void BetweenScanCondition::analyzeConditions(VirtualMachine* vm, SelectScanPlanner* planner) {
	this->left->analyzeConditions(vm, planner);
	this->start->analyzeConditions(vm, planner);
	this->end->analyzeConditions(vm, planner);

}


} /* namespace codablecash */
