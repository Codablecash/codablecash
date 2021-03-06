/*
 * TableIndexDetector.h
 *
 *  Created on: 2020/09/07
 *      Author: iizuka
 */

#ifndef SCAN_PLANNER_SCANNER_CTX_INDEX_TABLEINDEXDETECTOR_H_
#define SCAN_PLANNER_SCANNER_CTX_INDEX_TABLEINDEXDETECTOR_H_

namespace alinous {
class VirtualMachine;
}
using namespace alinous;

namespace codablecash {

class SelectScanPlanner;
class AbstractScanCondition;
class TableIndexDetectorStack;
class AbstractIndexCandidate;

class TableIndexDetector {
public:
	TableIndexDetector(VirtualMachine* vm, SelectScanPlanner* planner);
	virtual ~TableIndexDetector();

	void detect(AbstractScanCondition* cond);

	void push(AbstractIndexCandidate* candidate) noexcept;
	AbstractIndexCandidate* pop() noexcept;

private:
	VirtualMachine* vm;
	SelectScanPlanner* planner;

	TableIndexDetectorStack* stack;

};

} /* namespace codablecash */

#endif /* SCAN_PLANNER_SCANNER_CTX_INDEX_TABLEINDEXDETECTOR_H_ */
