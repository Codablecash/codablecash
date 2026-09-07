/*
 * AbstractSimnetClientCommand.cpp
 *
 *  Created on: Sep 3, 2026
 *      Author: iizuka
 */

#include "sim_net_client/AbstractSimnetClientCommand.h"

namespace codablecash {

AbstractSimnetClientCommand::AbstractSimnetClientCommand(const AbstractSimnetClientCommand &inst)
		: AbstractPubSubCommand(inst) {
}

AbstractSimnetClientCommand::AbstractSimnetClientCommand(uint32_t type) : AbstractPubSubCommand(type) {

}

AbstractSimnetClientCommand::~AbstractSimnetClientCommand() {

}

} /* namespace codablecash */
