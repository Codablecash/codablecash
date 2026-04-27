/*
 * AbstractPoWClientNotifyCommand.cpp
 *
 *  Created on: Apr 19, 2026
 *      Author: iizuka
 */

#include "pow_pool_cmd/AbstractPoWClientNotifyCommand.h"

namespace codablecash {

AbstractPoWClientNotifyCommand::AbstractPoWClientNotifyCommand(const AbstractPoWClientNotifyCommand &inst)
		: AbstractPubSubCommand(inst) {

}

AbstractPoWClientNotifyCommand::AbstractPoWClientNotifyCommand(uint32_t type) : AbstractPubSubCommand(type) {

}

AbstractPoWClientNotifyCommand::~AbstractPoWClientNotifyCommand() {

}

} /* namespace codablecash */
