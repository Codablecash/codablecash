/*
 * AbstractPoWClientNotifyCommand.h
 *
 *  Created on: Apr 19, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_CMD_ABSTRACTPOWCLIENTNOTIFYCOMMAND_H_
#define POW_POOL_CMD_ABSTRACTPOWCLIENTNOTIFYCOMMAND_H_

#include "pubsub_cmd/AbstractPubSubCommand.h"

namespace codablecash {

class AbstractPoWClientNotifyCommand : public AbstractPubSubCommand {
public:
	AbstractPoWClientNotifyCommand(const AbstractPoWClientNotifyCommand& inst);
	explicit AbstractPoWClientNotifyCommand(uint32_t type);
	virtual ~AbstractPoWClientNotifyCommand();
};

} /* namespace codablecash */

#endif /* POW_POOL_CMD_ABSTRACTPOWCLIENTNOTIFYCOMMAND_H_ */
