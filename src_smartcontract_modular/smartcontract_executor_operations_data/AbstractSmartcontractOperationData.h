/*
 * AbstractSmartcontractOperationData.h
 *
 *  Created on: Jan 28, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_OPERATIONS_DATA_ABSTRACTSMARTCONTRACTOPERATIONDATA_H_
#define SMARTCONTRACT_EXECUTOR_OPERATIONS_DATA_ABSTRACTSMARTCONTRACTOPERATIONDATA_H_

#include <cstdint>

#include "filestore_block/IBlockObject.h"

namespace alinous {
class ByteBuffer;
}
using namespace alinous;

namespace codablecash {

class AbstractSmartcontractOperationData : public IBlockObject {
public:
	static const constexpr uint8_t KIND_CREATE_SMARTCONTRACT{1};
	static const constexpr uint8_t KIND_PROCESS_SMARTCONTRACT{2};

	AbstractSmartcontractOperationData(const AbstractSmartcontractOperationData& inst);
	explicit AbstractSmartcontractOperationData(uint8_t kind);
	virtual ~AbstractSmartcontractOperationData();

	static AbstractSmartcontractOperationData* createFromBinary(ByteBuffer* in);

	virtual int binarySize() const = 0;
	virtual void toBinary(ByteBuffer* out) const = 0;
	virtual void fromBinary(ByteBuffer* in) = 0;

protected:
	uint8_t kind;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_OPERATIONS_DATA_ABSTRACTSMARTCONTRACTOPERATIONDATA_H_ */
