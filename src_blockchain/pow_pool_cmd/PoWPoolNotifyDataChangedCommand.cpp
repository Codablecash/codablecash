/*
 * PoWPoolNotifyDataChangedCommand.cpp
 *
 *  Created on: Apr 19, 2026
 *      Author: iizuka
 */

#include "pow_pool_cmd/PoWPoolNotifyDataChangedCommand.h"

#include "pubsub_cmd/OkPubsubResponse.h"

#include "pow_pool_client/PoWPubsubCommandExecutor.h"
#include "pow_pool_client/PoWWorkerClient.h"


namespace codablecash {

PoWPoolNotifyDataChangedCommand::PoWPoolNotifyDataChangedCommand(const PoWPoolNotifyDataChangedCommand &inst)
		: AbstractPoWClientNotifyCommand(inst) {
	this->height = inst.height;
}

PoWPoolNotifyDataChangedCommand::PoWPoolNotifyDataChangedCommand() : AbstractPoWClientNotifyCommand(TYPE_POW_POOL_NOTIFY_DATA_CHANGED) {
	this->height = 0;
}

PoWPoolNotifyDataChangedCommand::~PoWPoolNotifyDataChangedCommand() {

}

int PoWPoolNotifyDataChangedCommand::binarySize() const {
	int total = sizeof(uint32_t);
	total += sizeof(this->height);

	return total;
}

void PoWPoolNotifyDataChangedCommand::toBinary(ByteBuffer *buff) const {
	buff->putInt(this->type);
	buff->putLong(this->height);
}

void PoWPoolNotifyDataChangedCommand::fromBinary(ByteBuffer *buff) {
	this->height = buff->getLong();
}

IBlockObject* PoWPoolNotifyDataChangedCommand::copyData() const noexcept {
	return new PoWPoolNotifyDataChangedCommand(*this);
}

AbstractCommandResponse* PoWPoolNotifyDataChangedCommand::execute(const PubSubId *pubsubId, IPubsubCommandListner *listner) const {
	PoWPubsubCommandExecutor* executor = dynamic_cast<PoWPubsubCommandExecutor*>(listner);
	PoWWorkerClient* client = dynamic_cast<PoWWorkerClient*>(executor->getExecutor());

	client->stop(this->height);

	return new OkPubsubResponse();
}

void PoWPoolNotifyDataChangedCommand::setHeight(uint64_t height) {
	this->height = height;
}

} /* namespace codablecash */
