/*
 * ProjectIndexDataFactory.h
 *
 *  Created on: Feb 1, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_INDEX_PROJECTID_PROJECTINDEXDATAFACTORY_H_
#define SMARTCONTRACT_EXECUTOR_INDEX_PROJECTID_PROJECTINDEXDATAFACTORY_H_

#include "btree/AbstractBtreeDataFactory.h"

using namespace alinous;

namespace codablecash {

class ProjectIndexDataFactory : public AbstractBtreeDataFactory {
public:
	ProjectIndexDataFactory();
	virtual ~ProjectIndexDataFactory();

	virtual IBlockObject* makeDataFromBinary(ByteBuffer* in);
	virtual AbstractBtreeDataFactory* copy() const noexcept;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_INDEX_PROJECTID_PROJECTINDEXDATAFACTORY_H_ */
