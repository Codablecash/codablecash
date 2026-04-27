/*
 * PoWPoolStatusCommand.cpp
 *
 *  Created on: Apr 19, 2026
 *      Author: iizuka
 */

#include "pow_pool_client_cmd/PoWPoolStatusCommand.h"
#include "pow_pool_client_cmd/PoWPoolStatusCommandResponse.h"

#include "pow_pool/NetworkPoWPool.h"
#include "pow_pool/PoWRequestStatus.h"
#include "pow_pool/PoWRequestStatusData.h"

#include "pubsub_cmd/OkPubsubResponse.h"

#include "pubsub/IPubsubCommandListner.h"

#include "base/StackRelease.h"

#include "bc/ExceptionThrower.h"

#include "pubsub/PubsubNetworkException.h"
namespace codablecash {

PoWPoolStatusCommand::PoWPoolStatusCommand(const PoWPoolStatusCommand &inst)
		: AbstractPowClientRequestCommand(inst) {
}

PoWPoolStatusCommand::PoWPoolStatusCommand() : AbstractPowClientRequestCommand(TYPE_POW_POOL_CLIENT_STATUS) {


}

PoWPoolStatusCommand::~PoWPoolStatusCommand() {

}

int PoWPoolStatusCommand::binarySize() const {
	int total = AbstractPowClientRequestCommand::binarySize();

	return total;
}

void PoWPoolStatusCommand::toBinary(ByteBuffer *buff) const {
	AbstractPowClientRequestCommand::toBinary(buff);
}

void PoWPoolStatusCommand::fromBinary(ByteBuffer *buff) {
	AbstractPowClientRequestCommand::fromBinary(buff);
}

IBlockObject* PoWPoolStatusCommand::copyData() const noexcept {
	return new PoWPoolStatusCommand(*this);
}

AbstractCommandResponse* PoWPoolStatusCommand::execute(const PubSubId *pubsubId, IPubsubCommandListner *listner) const {
	IPubsubCommandExecutor* exe = listner->getExecutor();
	ExceptionThrower<PubsubNetworkException>::throwExceptionIfCondition(exe == nullptr, L"The pool was finished", __FILE__, __LINE__);

	NetworkPoWPool* pool = dynamic_cast<NetworkPoWPool*>(exe);

	PoWRequestStatus* reqStatus = pool->getRequestStatus();
	PoWRequestStatusData* data = reqStatus->getStatusData(); __STP(data);

	PoWPoolStatusCommandResponse* response = new PoWPoolStatusCommandResponse();
	response->setData(data);

	return response;
}

} /* namespace codablecash */
