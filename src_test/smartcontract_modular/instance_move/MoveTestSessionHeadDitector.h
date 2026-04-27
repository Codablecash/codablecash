/*
 * MoveTestSessionHeadDitector.h
 *
 *  Created on: Mar 19, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_MODULAR_INSTANCE_MOVE_MOVETESTSESSIONHEADDITECTOR_H_
#define SMARTCONTRACT_MODULAR_INSTANCE_MOVE_MOVETESTSESSIONHEADDITECTOR_H_

#include "smartcontract_executor_index/AbstractSessionDetector.h"

namespace codablecash {

class InstanceSessionContext;

class MoveTestSessionHeadDitector : public AbstractSessionDetector {
public:
	explicit MoveTestSessionHeadDitector(SmartcontractExecContextRegistory* contextRegistory);
	virtual ~MoveTestSessionHeadDitector();


	ArrayList<InstanceSessionContext>* getHeads() const noexcept {
		return this->heads;
	}

protected:
	virtual void registerHead(const InstanceSessionContext* context, const SmartcontractExecContextData* data);

private:
	ArrayList<InstanceSessionContext>* heads;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_MODULAR_INSTANCE_MOVE_MOVETESTSESSIONHEADDITECTOR_H_ */
