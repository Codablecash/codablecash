/*
 * CrossJoinScanTarget.cpp
 *
 *  Created on: 2020/08/08
 *      Author: iizuka
 */

#include "scan_select/scan_table/CrossJoinScanTarget.h"

#include "scan_select/scan_planner/base/ConditionsHolder.h"
#include "scan_select/scan_planner/base/SelectScanPlanner.h"

#include "scan_select/scan_planner/scanner/ctx/ScanJoinContextHolder.h"
#include "scan_select/scan_planner/scanner/ctx/FilterConditionDitector.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "scan_select/scan_planner/scanner/join/JoinCandidateHolder.h"
#include "scan_select/scan_planner/scanner/join/AbstractJoinCandidate.h"

#include "scan_select/scan_condition/base/AbstractScanCondition.h"
#include "scan_select/scan_condition/base/RootScanCondition.h"

#include "scan_select/scan_planner/scanner/factory/AbstractJoinScannerFactory.h"
#include "scan_select/scan_planner/scanner/factory/InnerJoinScannerFactory.h"
#include "scan_select/scan_planner/scanner/factory/LeftJoinScannerFactory.h"
#include "scan_select/scan_planner/scanner/factory/CrossJoinScanerFactory.h"

namespace codablecash {

CrossJoinScanTarget::CrossJoinScanTarget() {

}

CrossJoinScanTarget::~CrossJoinScanTarget() {

}

const UnicodeString* CrossJoinScanTarget::toString() noexcept {
	if(this->str == nullptr){
		resetStr();
		this->str = new UnicodeString(L"");

		this->str->append(this->left->toString());

		this->str->append(L" CROSS JOIN ");

		this->str->append(this->right->toString());

		addConditionString();
	}

	return this->str;
}

AbstractScannerFactory* CrossJoinScanTarget::getScanFactory(VirtualMachine* vm, SelectScanPlanner* planner) {
	ConditionsHolder* holder = planner->getConditions();

	ScanJoinContextHolder* contextHolder = new ScanJoinContextHolder(); __STP(contextHolder);

	JoinCandidateHolder joinCandidates(this->left, this->right);

	// ON clause
	if(this->cond != nullptr){
		this->cond->collectJoinCandidate(vm, planner, AbstractJoinCandidate::INNER, &joinCandidates);
		while(!joinCandidates.isEmpty()){
			AbstractJoinCandidate* candidate = joinCandidates.pop();
			joinCandidates.addJoinCandidate(candidate);
		}
	}

	RootScanCondition* root = holder->getRoot(); // Where clause
	if(root != nullptr){
		root->collectJoinCandidate(vm, planner, AbstractJoinCandidate::INNER, &joinCandidates);
		while(!joinCandidates.isEmpty()){
			AbstractJoinCandidate* candidate = joinCandidates.pop();
			joinCandidates.addJoinCandidate(candidate);
		}
	}

	// set up this->cond
	FilterConditionDitector filterDetector(vm, planner);
	filterDetector.detect(this);

	AbstractJoinScannerFactory* joinFactory = nullptr;
	if(joinCandidates.isInnerJoin()){
		joinFactory = new InnerJoinScannerFactory(this->metadata, this->cond);
	}
	else{
		joinFactory = new CrossJoinScanerFactory(this->metadata, this->cond);
	}
	AbstractScanCondition* filterCondition = filterDetector.getCondition();
	joinFactory->setFilterCondition(filterCondition);

	const AbstractJoinCandidate* bestCandidate = joinCandidates.getBestCandidate(this->left, this->right);
	joinFactory->setJoinCandidate(bestCandidate);

	AbstractScannerFactory* leftFactory = this->left->getScanFactory(vm, planner);
	joinFactory->setLeft(leftFactory);

	AbstractScannerFactory* rightFactory = this->right->getScanFactory(vm, planner);
	joinFactory->setRight(rightFactory);

	return joinFactory;
}


} /* namespace codablecash */
