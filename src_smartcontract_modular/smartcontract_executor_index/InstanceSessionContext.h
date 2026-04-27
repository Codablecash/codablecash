/*
 * InstanceSessionContext.h
 *
 *  Created on: Feb 18, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_INDEX_INSTANCESESSIONCONTEXT_H_
#define SMARTCONTRACT_EXECUTOR_INDEX_INSTANCESESSIONCONTEXT_H_

#include <cstdint>

#include "filestore_block/IBlockObject.h"

using namespace alinous;

namespace codablecash {

class CdbDatabaseSessionId;


class InstanceSessionContext : public IBlockObject {
public:
	InstanceSessionContext(const InstanceSessionContext& inst);
	InstanceSessionContext();
	virtual ~InstanceSessionContext();

	void setCallSerial(uint64_t callSerial);
	void setTrxId(const CdbDatabaseSessionId* trxId);

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static InstanceSessionContext* createFromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;

	uint64_t getCallSerial() const noexcept {
		return this->callSerial;
	}
	const CdbDatabaseSessionId* getSessionId() const noexcept {
		return this->trxId;
	}

private:
	uint64_t callSerial;
	CdbDatabaseSessionId* trxId;
};

class SessionContextReverseCompare {
public:
	int operator() (const InstanceSessionContext* const a, const InstanceSessionContext* const b) const noexcept;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_INDEX_INSTANCESESSIONCONTEXT_H_ */
