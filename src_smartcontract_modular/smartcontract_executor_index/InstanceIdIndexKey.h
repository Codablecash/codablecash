/*
 * InstanceIdIndexKey.h
 *
 *  Created on: Nov 28, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_INDEX_INSTANCEIDINDEXKEY_H_
#define SMARTCONTRACT_EXECUTOR_INDEX_INSTANCEIDINDEXKEY_H_

#include "btree/AbstractBtreeKey.h"

namespace alinous {
class AbstractBtreeKey;
}
using namespace alinous;

namespace codablecash {

class SmartcontractInstanceAddress;
class CdbDatabaseSessionId;

class InstanceIdIndexKey : public AbstractBtreeKey {
public:
	InstanceIdIndexKey();
	InstanceIdIndexKey(const SmartcontractInstanceAddress* instanceAddress, uint64_t height);
	virtual ~InstanceIdIndexKey();

	virtual bool isInfinity() const { return false; }
	virtual bool isNull() const { return false; }

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static InstanceIdIndexKey* fromBinary(ByteBuffer* in);

	virtual int compareTo(const AbstractBtreeKey* key) const noexcept;
	virtual AbstractBtreeKey* clone() const noexcept;

	void setInstanceAddress(const SmartcontractInstanceAddress* instanceAddress);
	SmartcontractInstanceAddress* getInstanceAddress() const noexcept {
		return this->instanceAddress;
	}
	void setHeight(uint64_t height);

	void setTrxId(const CdbDatabaseSessionId* trxId);
	CdbDatabaseSessionId* getTrxId() const noexcept {
		return this->trxId;
	}

private:
	SmartcontractInstanceAddress* instanceAddress;
	uint64_t height;

	// for removal
	CdbDatabaseSessionId* trxId;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_INDEX_INSTANCEIDINDEXKEY_H_ */
