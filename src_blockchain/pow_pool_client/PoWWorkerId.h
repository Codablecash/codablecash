/*
 * PoWClientID.h
 *
 *  Created on: Apr 13, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_CLIENT_POWWORKERID_H_
#define POW_POOL_CLIENT_POWWORKERID_H_

#include "bc_base/Abstract32BytesId.h"

namespace codablecash {

class PoWWorkerId : public Abstract32BytesId {
public:
	PoWWorkerId(const PoWWorkerId& inst);
	PoWWorkerId();
	PoWWorkerId(const char *binary, int length);
	virtual ~PoWWorkerId();

	static PoWWorkerId* fromBinary(ByteBuffer* in);

	static PoWWorkerId* createRandomId();

	virtual IBlockObject* copyData() const noexcept;
};

} /* namespace codablecash */

#endif /* POW_POOL_CLIENT_POWWORKERID_H_ */
