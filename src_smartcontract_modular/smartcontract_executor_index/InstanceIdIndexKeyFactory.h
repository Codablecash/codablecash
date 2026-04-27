/*
 * InstanceIdIndexKeyFactory.h
 *
 *  Created on: Nov 28, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_INDEX_INSTANCEIDINDEXKEYFACTORY_H_
#define SMARTCONTRACT_EXECUTOR_INDEX_INSTANCEIDINDEXKEYFACTORY_H_

#include "btreekey/BtreeKeyFactory.h"

using namespace alinous;

namespace codablecash {

class InstanceIdIndexKeyFactory : public BtreeKeyFactory {
public:
	static const constexpr uint32_t INSTANCE_INDEX_KEY{0x24};

	InstanceIdIndexKeyFactory();
	virtual ~InstanceIdIndexKeyFactory();

	virtual AbstractBtreeKey* fromBinary(uint32_t keyType, ByteBuffer* in) const;
	virtual BtreeKeyFactory* copy() const noexcept;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_INDEX_INSTANCEIDINDEXKEYFACTORY_H_ */
