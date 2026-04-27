/*
 * SmartcontractExecContextKey.h
 *
 *  Created on: Nov 28, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_SMARTCONTRACTEXECCONTEXTKEY_H_
#define SMARTCONTRACT_EXECUTOR_SMARTCONTRACTEXECCONTEXTKEY_H_
#include <cstdint>


#include "btree/AbstractBtreeKey.h"

using namespace alinous;

namespace codablecash {

class CdbDatabaseSessionId;

class SmartcontractExecContextKey : public AbstractBtreeKey {
public:
	SmartcontractExecContextKey(const SmartcontractExecContextKey& inst);
	SmartcontractExecContextKey();
	virtual ~SmartcontractExecContextKey();

	virtual bool isInfinity() const { return false; }
	virtual bool isNull() const { return false; }

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static SmartcontractExecContextKey* fromBinary(ByteBuffer* in);

	virtual int compareTo(const AbstractBtreeKey* key) const noexcept;
	virtual AbstractBtreeKey* clone() const noexcept;

	void setTransactionId(const CdbDatabaseSessionId* trxId);

private:
	CdbDatabaseSessionId* trxId;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_SMARTCONTRACTEXECCONTEXTKEY_H_ */
