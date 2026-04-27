/*
 * PoWPoolCheckData.cpp
 *
 *  Created on: Apr 19, 2026
 *      Author: iizuka
 */

#include "pow_pool/NetworkPoWPool.h"
#include "pow_pool_client_cmd/PoWPoolCheckDataCommand.h"

#include "pow_pool/PoWRequestStatus.h"
#include "pow_pool/PoWRequest2Client.h"

#include "pubsub/IPubsubCommandListner.h"

#include "base/StackRelease.h"

#include "pow_pool_client_cmd/PoWPoolCheckDataCommandResponse.h"

namespace codablecash {

PoWPoolCheckDataCommand::PoWPoolCheckDataCommand(const PoWPoolCheckDataCommand &inst) : AbstractPowClientRequestCommand(inst) {

}

PoWPoolCheckDataCommand::PoWPoolCheckDataCommand() : AbstractPowClientRequestCommand(TYPE_POW_POOL_CLIENT_CHECK_DATA) {

}

PoWPoolCheckDataCommand::~PoWPoolCheckDataCommand() {

}

int PoWPoolCheckDataCommand::binarySize() const {
	int total = AbstractPowClientRequestCommand::binarySize();

	return total;
}

void PoWPoolCheckDataCommand::toBinary(ByteBuffer *buff) const {
	AbstractPowClientRequestCommand::toBinary(buff);
}

void PoWPoolCheckDataCommand::fromBinary(ByteBuffer *buff) {
	AbstractPowClientRequestCommand::fromBinary(buff);
}

IBlockObject* PoWPoolCheckDataCommand::copyData() const noexcept {
	return new PoWPoolCheckDataCommand(*this);
}

AbstractCommandResponse* PoWPoolCheckDataCommand::execute(const PubSubId *pubsubId, IPubsubCommandListner *listner) const {
	IPubsubCommandExecutor* exe = listner->getExecutor();
	NetworkPoWPool* pool = dynamic_cast<NetworkPoWPool*>(exe);

	PoWRequestStatus* reqStatus = pool->getRequestStatus();

	PoWRequest2Client* request = reqStatus->getClientPoWRequest(); __STP(request);

	PoWPoolCheckDataCommandResponse* res = new PoWPoolCheckDataCommandResponse();
	res->setRequest(request);

	return res;
}

} /* namespace codablecash */
