/*
 * AbstractSimnetClientCommand.h
 *
 *  Created on: Sep 3, 2026
 *      Author: iizuka
 */

#ifndef SIM_NET_CLIENT_ABSTRACTSIMNETCLIENTCOMMAND_H_
#define SIM_NET_CLIENT_ABSTRACTSIMNETCLIENTCOMMAND_H_

#include "pubsub_cmd/AbstractPubSubCommand.h"

namespace codablecash {

class AbstractSimnetClientCommand : public AbstractPubSubCommand {
public:
	AbstractSimnetClientCommand(const AbstractSimnetClientCommand& inst);
	explicit AbstractSimnetClientCommand(uint32_t type);
	virtual ~AbstractSimnetClientCommand();
};

} /* namespace codablecash */

#endif /* SIM_NET_CLIENT_ABSTRACTSIMNETCLIENTCOMMAND_H_ */
