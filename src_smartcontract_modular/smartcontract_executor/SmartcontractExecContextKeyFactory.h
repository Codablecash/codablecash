/*
 * SmartcontractExecContextKeyFactory.h
 *
 *  Created on: Nov 28, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_SMARTCONTRACTEXECCONTEXTKEYFACTORY_H_
#define SMARTCONTRACT_EXECUTOR_SMARTCONTRACTEXECCONTEXTKEYFACTORY_H_

#include "btreekey/BtreeKeyFactory.h"

using namespace alinous;

namespace codablecash {

class SmartcontractExecContextKeyFactory : public BtreeKeyFactory {
public:
	static const constexpr uint32_t SMARTCONTRACT_EXEC_ID_KEY{0x24};

	SmartcontractExecContextKeyFactory();
	virtual ~SmartcontractExecContextKeyFactory();

	virtual AbstractBtreeKey* fromBinary(uint32_t keyType, ByteBuffer* in) const;
	virtual BtreeKeyFactory* copy() const noexcept;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_SMARTCONTRACTEXECCONTEXTKEYFACTORY_H_ */
