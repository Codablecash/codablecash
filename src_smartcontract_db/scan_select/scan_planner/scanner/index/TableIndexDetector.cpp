/*
 * TableIndexDetector.cpp
 *
 *  Created on: 2020/09/07
 *      Author: iizuka
 */

#include "scan_select/scan_planner/scanner/index/TableIndexDetector.h"
#include "scan_select/scan_planner/scanner/index/TableIndexDetectorStack.h"

#include "scan_select/scan_condition/base/AbstractScanCondition.h"


namespace codablecash {

TableIndexDetector::TableIndexDetector(VirtualMachine* vm, SelectScanPlanner* planner) {
	this->vm = vm;
	this->planner = planner;
	this->stack = new TableIndexDetectorStack();
}

TableIndexDetector::~TableIndexDetector() {
	this->vm = nullptr;
	this->planner = nullptr;
	delete this->stack;
}

void TableIndexDetector::detect(AbstractScanCondition* cond) {
	if(cond != nullptr){

	}
}

void TableIndexDetector::push(AbstractIndexCandidate* candidate) noexcept {
	this->stack->push(candidate);
}

AbstractIndexCandidate* TableIndexDetector::pop() noexcept {
	return this->stack->pop();
}

} /* namespace codablecash */
