/*
 * SmartcontractProjectDataFactory.h
 *
 *  Created on: Nov 27, 2025
 *      Author: iizuka
 */

#ifndef MODULAR_PROJECT_REGISTORY_SMARTCONTRACTPROJECTDATAFACTORY_H_
#define MODULAR_PROJECT_REGISTORY_SMARTCONTRACTPROJECTDATAFACTORY_H_

#include "btree/AbstractBtreeDataFactory.h"

using namespace alinous;

namespace codablecash {

class SmartcontractProjectDataFactory : public AbstractBtreeDataFactory {
public:
	SmartcontractProjectDataFactory();
	virtual ~SmartcontractProjectDataFactory();

	virtual IBlockObject* makeDataFromBinary(ByteBuffer* in);
	virtual AbstractBtreeDataFactory* copy() const noexcept;
};

} /* namespace codablecash */

#endif /* MODULAR_PROJECT_REGISTORY_SMARTCONTRACTPROJECTDATAFACTORY_H_ */
