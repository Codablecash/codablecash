/*
 * PoWPoolCheckData.h
 *
 *  Created on: Apr 19, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_CLIENT_CMD_POWPOOLCHECKDATACOMMAND_H_
#define POW_POOL_CLIENT_CMD_POWPOOLCHECKDATACOMMAND_H_

#include "pow_pool_client_cmd/AbstractPowClientRequestCommand.h"

namespace codablecash {

class PoWPoolCheckDataCommand : public AbstractPowClientRequestCommand {
public:
	PoWPoolCheckDataCommand(const PoWPoolCheckDataCommand& inst);
	PoWPoolCheckDataCommand();
	virtual ~PoWPoolCheckDataCommand();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* buff) const;
	virtual void fromBinary(ByteBuffer* buff);

	virtual IBlockObject* copyData() const noexcept;

	virtual AbstractCommandResponse* execute(const PubSubId* pubsubId, IPubsubCommandListner* listner) const;
};

} /* namespace codablecash */

#endif /* POW_POOL_CLIENT_CMD_POWPOOLCHECKDATACOMMAND_H_ */
