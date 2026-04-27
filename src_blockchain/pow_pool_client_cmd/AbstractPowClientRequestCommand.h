/*
 * AbstractPowClientRequestCommand.h
 *
 *  Created on: Apr 19, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_CLIENT_CMD_ABSTRACTPOWCLIENTREQUESTCOMMAND_H_
#define POW_POOL_CLIENT_CMD_ABSTRACTPOWCLIENTREQUESTCOMMAND_H_

#include "pubsub_cmd/AbstractPubSubCommand.h"

namespace codablecash {

class PoWWorkerId;

class AbstractPowClientRequestCommand : public AbstractPubSubCommand {
public:
	AbstractPowClientRequestCommand(const AbstractPowClientRequestCommand& inst);
	explicit AbstractPowClientRequestCommand(uint32_t type);
	virtual ~AbstractPowClientRequestCommand();

	virtual void fromBinary(ByteBuffer* buff);
	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* buff) const;

	void setClientId(const PoWWorkerId* id);

protected:
	PoWWorkerId* clientId;
};

} /* namespace codablecash */

#endif /* POW_POOL_CLIENT_CMD_ABSTRACTPOWCLIENTREQUESTCOMMAND_H_ */
