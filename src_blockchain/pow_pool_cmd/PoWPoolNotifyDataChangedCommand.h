/*
 * PoWPoolNotifyDataChangedCommand.h
 *
 *  Created on: Apr 19, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_CMD_POWPOOLNOTIFYDATACHANGEDCOMMAND_H_
#define POW_POOL_CMD_POWPOOLNOTIFYDATACHANGEDCOMMAND_H_

#include "pow_pool_cmd/AbstractPoWClientNotifyCommand.h"

namespace codablecash {

class PoWPoolNotifyDataChangedCommand : public AbstractPoWClientNotifyCommand {
public:
	PoWPoolNotifyDataChangedCommand(const PoWPoolNotifyDataChangedCommand& inst);
	PoWPoolNotifyDataChangedCommand();
	virtual ~PoWPoolNotifyDataChangedCommand();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* buff) const;
	virtual void fromBinary(ByteBuffer* buff);

	virtual IBlockObject* copyData() const noexcept;

	virtual AbstractCommandResponse* execute(const PubSubId* pubsubId, IPubsubCommandListner* listner) const;

	void setHeight(uint64_t height);

private:
	uint64_t height; // height is last block's height
};

} /* namespace codablecash */

#endif /* POW_POOL_CMD_POWPOOLNOTIFYDATACHANGEDCOMMAND_H_ */
