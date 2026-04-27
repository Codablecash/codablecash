/*
 * IInstanceSessionContextListner.h
 *
 *  Created on: Mar 18, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_INDEX_IINSTANCESESSIONCONTEXTLISTNER_H_
#define SMARTCONTRACT_EXECUTOR_INDEX_IINSTANCESESSIONCONTEXTLISTNER_H_

namespace codablecash {

class InstanceSessionContext;

class IInstanceSessionContextListner {
public:
	IInstanceSessionContextListner();
	virtual ~IInstanceSessionContextListner();

	virtual void visit(const InstanceSessionContext* context) = 0;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_INDEX_IINSTANCESESSIONCONTEXTLISTNER_H_ */
