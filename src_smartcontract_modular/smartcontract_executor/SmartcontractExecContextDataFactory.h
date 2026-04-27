/*
 * SmartcontractExecContextDataFactory.h
 *
 *  Created on: Nov 28, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_SMARTCONTRACTEXECCONTEXTDATAFACTORY_H_
#define SMARTCONTRACT_EXECUTOR_SMARTCONTRACTEXECCONTEXTDATAFACTORY_H_

#include "btree/AbstractBtreeDataFactory.h"

using namespace alinous;

namespace codablecash {

class SmartcontractExecContextDataFactory : public AbstractBtreeDataFactory {
public:
	SmartcontractExecContextDataFactory();
	virtual ~SmartcontractExecContextDataFactory();

	virtual IBlockObject* makeDataFromBinary(ByteBuffer* in);
	virtual AbstractBtreeDataFactory* copy() const noexcept;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_SMARTCONTRACTEXECCONTEXTDATAFACTORY_H_ */
