/*
 * PoWPoolStatusCommand.h
 *
 *  Created on: Apr 19, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_CLIENT_CMD_POWPOOLSTATUSCOMMAND_H_
#define POW_POOL_CLIENT_CMD_POWPOOLSTATUSCOMMAND_H_

#include "pow_pool_client_cmd/AbstractPowClientRequestCommand.h"

namespace codablecash {

class PoWPoolStatusCommand : public AbstractPowClientRequestCommand {
public:
	PoWPoolStatusCommand(const PoWPoolStatusCommand& inst);
	PoWPoolStatusCommand();
	virtual ~PoWPoolStatusCommand();

	virtual void fromBinary(ByteBuffer* buff);
	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* buff) const;

	virtual IBlockObject* copyData() const noexcept;

	virtual AbstractCommandResponse* execute(const PubSubId* pubsubId, IPubsubCommandListner* listner) const;
};

} /* namespace codablecash */

#endif /* POW_POOL_CLIENT_CMD_POWPOOLSTATUSCOMMAND_H_ */
