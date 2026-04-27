/*
 * SmartcontractExecContextData.h
 *
 *  Created on: Nov 28, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_SMARTCONTRACTEXECCONTEXTDATA_H_
#define SMARTCONTRACT_EXECUTOR_SMARTCONTRACTEXECCONTEXTDATA_H_
#include <cstdint>

#include "filestore_block/IBlockObject.h"

using namespace alinous;

namespace codablecash {

class CdbDatabaseSessionId;
class BlockHeaderId;
class AbstractSmartcontractOperationData;

class SmartcontractExecContextData : public IBlockObject {
public:
	SmartcontractExecContextData(const SmartcontractExecContextData& inst);
	SmartcontractExecContextData();
	virtual ~SmartcontractExecContextData();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static SmartcontractExecContextData* createFromBinary(ByteBuffer *in);

	virtual IBlockObject* copyData() const noexcept;

	void setOperationData(const AbstractSmartcontractOperationData* opData);

	void setCallSerial(uint64_t callSerial);
	void setHeight(uint64_t height);
	void setBlockHead(const BlockHeaderId* blockHeaderId);
	void setLastSessionId(const CdbDatabaseSessionId* lastTrxId);
	void setFinalized(bool finalized);

	uint64_t getCallSerial() const noexcept {
		return this->callSerial;
	}
	uint64_t getHeight() const noexcept {
		return this->height;
	}
	const BlockHeaderId* getBlockHeaderId() const noexcept {
		return this->blockHeaderId;
	}
	const AbstractSmartcontractOperationData* getOpData() const noexcept {
		return this->opData;
	}
	const CdbDatabaseSessionId* getLastTrxId() const noexcept {
		return this->lastTrxId;
	}
	bool isFinalized() const noexcept {
		return this->finalized;
	}

protected:
	uint64_t callSerial;
	uint64_t height;
	BlockHeaderId* blockHeaderId;
	AbstractSmartcontractOperationData* opData;

	CdbDatabaseSessionId* lastTrxId; // = SessionId = trxId

	// snapshot
	bool finalized;

};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_SMARTCONTRACTEXECCONTEXTDATA_H_ */
