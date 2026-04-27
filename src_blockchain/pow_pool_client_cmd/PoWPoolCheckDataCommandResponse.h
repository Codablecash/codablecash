/*
 * PoWPoolCheckDataCommandResponse.h
 *
 *  Created on: Apr 21, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_CLIENT_CMD_POWPOOLCHECKDATACOMMANDRESPONSE_H_
#define POW_POOL_CLIENT_CMD_POWPOOLCHECKDATACOMMANDRESPONSE_H_

#include "pubsub_cmd/AbstractCommandResponse.h"

namespace codablecash {

class PoWRequest2Client;

class PoWPoolCheckDataCommandResponse : public AbstractCommandResponse {
public:
	PoWPoolCheckDataCommandResponse();
	virtual ~PoWPoolCheckDataCommandResponse();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* buff) const;
	virtual void fromBinary(ByteBuffer* buff);

	virtual UnicodeString* toString() const noexcept;

	void setRequest(const PoWRequest2Client* request);
	PoWRequest2Client* getRequest() const noexcept {
		return this->request;
	}

private:
	PoWRequest2Client* request;
};

} /* namespace codablecash */

#endif /* POW_POOL_CLIENT_CMD_POWPOOLCHECKDATACOMMANDRESPONSE_H_ */
