/*
 * ProjectIdIndexKey.h
 *
 *  Created on: Jan 29, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_INDEX_PROJECTID_PROJECTIDINDEXKEY_H_
#define SMARTCONTRACT_EXECUTOR_INDEX_PROJECTID_PROJECTIDINDEXKEY_H_

#include "btree/AbstractBtreeKey.h"

using namespace alinous;

namespace codablecash {

class SmartcontractModuleAddress;

class ProjectIdIndexKey : public AbstractBtreeKey {
public:
	ProjectIdIndexKey();
	ProjectIdIndexKey(const SmartcontractModuleAddress* moduleAddress, const AbstractBtreeKey* version);
	virtual ~ProjectIdIndexKey();

	virtual bool isInfinity() const { return false; }
	virtual bool isNull() const { return false; }

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static ProjectIdIndexKey* fromBinary(ByteBuffer* in);

	virtual int compareTo(const AbstractBtreeKey* key) const noexcept;
	virtual AbstractBtreeKey* clone() const noexcept;

	void setModuleAddress(const SmartcontractModuleAddress* moduleAddress);
	void setVersion(const AbstractBtreeKey* v) noexcept;

	SmartcontractModuleAddress* getSmartcontractModuleAddress() const noexcept {
		return this->moduleAddress;
	}

private:
	SmartcontractModuleAddress* moduleAddress;
	AbstractBtreeKey* version; // SoftwareVersionKey or NullKey of InfinityKey
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_INDEX_PROJECTID_PROJECTIDINDEXKEY_H_ */
