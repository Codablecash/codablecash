/*
 * AbstractSmartcontractOperation.h
 *
 *  Created on: Jan 21, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_OPERATIONS_ABSTRACTSMARTCONTRACTOPERATION_H_
#define SMARTCONTRACT_EXECUTOR_OPERATIONS_ABSTRACTSMARTCONTRACTOPERATION_H_
#include <cstdint>

namespace codablecash {

class BlockHeaderId;
class CdbDatabaseSessionId;

class ModularSmartcontractExecutor;

class AbstractSmartcontractOperation {
public:
	AbstractSmartcontractOperation();
	virtual ~AbstractSmartcontractOperation();

	virtual void execute(ModularSmartcontractExecutor* executor) = 0;

	void setSessionId(const CdbDatabaseSessionId* trxId);
	void setHeaderId(uint64_t height, const BlockHeaderId* blockHeaderId);

	void setLastTrxId(const CdbDatabaseSessionId* lastTrxId);

	const CdbDatabaseSessionId* getTrxId() const noexcept {
		return this->trxId;
	}

protected:
	CdbDatabaseSessionId* trxId;
	uint64_t height;
	BlockHeaderId* blockHeaderId;

	CdbDatabaseSessionId* lastTrxId;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_OPERATIONS_ABSTRACTSMARTCONTRACTOPERATION_H_ */
