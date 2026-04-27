/*
 * PoWPoolNonceCalculatedCommand.h
 *
 *  Created on: Apr 19, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_CLIENT_CMD_POWPOOLNONCECALCULATEDCOMMAND_H_
#define POW_POOL_CLIENT_CMD_POWPOOLNONCECALCULATEDCOMMAND_H_

#include "pow_pool_client_cmd/AbstractPowClientRequestCommand.h"

namespace codablecash {

class PoWNonce;

class PoWPoolNonceCalculatedCommand : public AbstractPowClientRequestCommand {
public:
	PoWPoolNonceCalculatedCommand(const PoWPoolNonceCalculatedCommand& inst);
	PoWPoolNonceCalculatedCommand();
	virtual ~PoWPoolNonceCalculatedCommand();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* buff) const;
	virtual void fromBinary(ByteBuffer* buff);

	virtual IBlockObject* copyData() const noexcept;

	virtual AbstractCommandResponse* execute(const PubSubId* pubsubId, IPubsubCommandListner* listner) const;

	void setPoWNonce(const PoWNonce* result);

private:
	PoWNonce* nonce;
};

} /* namespace codablecash */

#endif /* POW_POOL_CLIENT_CMD_POWPOOLNONCECALCULATEDCOMMAND_H_ */
