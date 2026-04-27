/*
 * PoWPoolStatusCommandResponse.h
 *
 *  Created on: Apr 20, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_CLIENT_CMD_POWPOOLSTATUSCOMMANDRESPONSE_H_
#define POW_POOL_CLIENT_CMD_POWPOOLSTATUSCOMMANDRESPONSE_H_

#include "pubsub_cmd/AbstractCommandResponse.h"

namespace codablecash {

class PoWRequestStatusData;

class PoWPoolStatusCommandResponse : public AbstractCommandResponse {
public:
	PoWPoolStatusCommandResponse();
	virtual ~PoWPoolStatusCommandResponse();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* buff) const;
	virtual void fromBinary(ByteBuffer* buff);

	virtual UnicodeString* toString() const noexcept;

	void setData(const PoWRequestStatusData* data);
	PoWRequestStatusData* getPoWRequestStatusData() const noexcept {
		return this->data;
	}

private:
	PoWRequestStatusData* data;
};

} /* namespace codablecash */

#endif /* POW_POOL_CLIENT_CMD_POWPOOLSTATUSCOMMANDRESPONSE_H_ */
