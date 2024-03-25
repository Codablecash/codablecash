/*
 * MultiplicativeExpressionScanColumnTarget.h
 *
 *  Created on: 2020/08/21
 *      Author: iizuka
 */

#ifndef SCAN_COLUMNS_ARITHMETIC_MULTIPLICATIVEEXPRESSIONSCANCOLUMNTARGET_H_
#define SCAN_COLUMNS_ARITHMETIC_MULTIPLICATIVEEXPRESSIONSCANCOLUMNTARGET_H_

#include "scan_select/scan_columns/scan_columns_exp/AbstractExpressionScanColumnTarget.h"
#include <cstdint>

#include "base/ArrayList.h"
#include "base/RawArrayPrimitive.h"

namespace codablecash {

class MultiplicativeScanCondition;

class MultiplicativeExpressionScanColumnTarget: public AbstractExpressionScanColumnTarget {
public:
	MultiplicativeExpressionScanColumnTarget();
	virtual ~MultiplicativeExpressionScanColumnTarget();

	void addOperand(AbstractScanColumnsTarget* vp) noexcept;
	void addOperator(uint8_t op) noexcept;

	virtual const UnicodeString* toStringCode() noexcept;

	virtual void resolveColumns(VirtualMachine* vm, SelectScanPlanner* planner);

	virtual void scanColumns(VirtualMachine* vm, const CdbRecord* record, const ScanResultMetadata* metadata, CdbRecord* newRecord);
	virtual IValueProvider* toValueProvider();
private:
	ArrayList<AbstractScanColumnsTarget> list;
	RawArrayPrimitive<uint8_t> operations;

	MultiplicativeScanCondition* cond;
};

} /* namespace codablecash */

#endif /* SCAN_COLUMNS_ARITHMETIC_MULTIPLICATIVEEXPRESSIONSCANCOLUMNTARGET_H_ */
