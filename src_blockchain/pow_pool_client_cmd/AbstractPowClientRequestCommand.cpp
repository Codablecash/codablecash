/*
 * AbstractPowClientRequestCommand.cpp
 *
 *  Created on: Apr 19, 2026
 *      Author: iizuka
 */

#include "pow_pool_client_cmd/AbstractPowClientRequestCommand.h"

#include "pow_pool_client/PoWWorkerId.h"

#include "bc_base/BinaryUtils.h"

namespace codablecash {

AbstractPowClientRequestCommand::AbstractPowClientRequestCommand(const AbstractPowClientRequestCommand &inst)
		: AbstractPubSubCommand(inst) {
	this->clientId = inst.clientId != nullptr ? dynamic_cast<PoWWorkerId*>(inst.clientId->copyData()) : nullptr;
}

AbstractPowClientRequestCommand::AbstractPowClientRequestCommand(uint32_t type) : AbstractPubSubCommand(type) {
	this->clientId = nullptr;
}

AbstractPowClientRequestCommand::~AbstractPowClientRequestCommand() {
	delete this->clientId;
}

void AbstractPowClientRequestCommand::setClientId(const PoWWorkerId *id) {
	delete this->clientId, this->clientId = nullptr;
	this->clientId = id != nullptr ? dynamic_cast<PoWWorkerId*>(id->copyData()) : nullptr;
}

int AbstractPowClientRequestCommand::binarySize() const {
	BinaryUtils::checkNotNull(this->clientId);

	int total = sizeof(uint32_t);
	total += this->clientId->binarySize();

	return total;
}

void AbstractPowClientRequestCommand::toBinary(ByteBuffer *buff) const {
	BinaryUtils::checkNotNull(this->clientId);

	buff->putInt(this->type);
	this->clientId->toBinary(buff);
}

void AbstractPowClientRequestCommand::fromBinary(ByteBuffer *buff) {
	this->clientId = PoWWorkerId::fromBinary(buff);
}

} /* namespace codablecash */
