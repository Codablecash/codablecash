/*
 * PoWPoolNonceCalculatedCommand.cpp
 *
 *  Created on: Apr 19, 2026
 *      Author: iizuka
 */

#include "pow_pool_client_cmd/PoWPoolNonceCalculatedCommand.h"

#include "pow_pool/NetworkPoWPool.h"
#include "pow_pool/PoWRequestStatus.h"
#include "pow_pool/PoWRequest2Client.h"

#include "pubsub/IPubsubCommandListner.h"

#include "pubsub_cmd/OkPubsubResponse.h"

#include "pow/PoWNonce.h"

#include "bc_base/BinaryUtils.h"

#include "base/StackRelease.h"


namespace codablecash {

PoWPoolNonceCalculatedCommand::PoWPoolNonceCalculatedCommand(const PoWPoolNonceCalculatedCommand &inst)
		: AbstractPowClientRequestCommand(inst) {
	this->nonce = inst.nonce != nullptr ? dynamic_cast<PoWNonce*>(inst.nonce->copyData()) : nullptr;
}

PoWPoolNonceCalculatedCommand::PoWPoolNonceCalculatedCommand() : AbstractPowClientRequestCommand(TYPE_POW_POOL_CLIENT_NONCE_CALCULATED) {
	this->nonce = nullptr;
}

PoWPoolNonceCalculatedCommand::~PoWPoolNonceCalculatedCommand() {
	delete this->nonce;
}

int PoWPoolNonceCalculatedCommand::binarySize() const {
	BinaryUtils::checkNotNull(this->nonce);

	int total = AbstractPowClientRequestCommand::binarySize();

	total += this->nonce->binarySize();

	return total;
}

void PoWPoolNonceCalculatedCommand::toBinary(ByteBuffer *buff) const {
	BinaryUtils::checkNotNull(this->nonce);

	AbstractPowClientRequestCommand::toBinary(buff);

	this->nonce->toBinary(buff);
}

void PoWPoolNonceCalculatedCommand::fromBinary(ByteBuffer *buff) {
	AbstractPowClientRequestCommand::fromBinary(buff);

	this->nonce = PoWNonce::createFromBinary(buff);
}

IBlockObject* PoWPoolNonceCalculatedCommand::copyData() const noexcept {
	return new PoWPoolNonceCalculatedCommand(*this);
}

AbstractCommandResponse* PoWPoolNonceCalculatedCommand::execute(const PubSubId *pubsubId, IPubsubCommandListner *listner) const {
	IPubsubCommandExecutor* exe = listner->getExecutor();
	NetworkPoWPool* pool = dynamic_cast<NetworkPoWPool*>(exe);

	PoWRequestStatus* reqStatus = pool->getRequestStatus();

	PoWRequest2Client* request  = reqStatus->getClientPoWRequest(); __STP(request);

	pool->onClientCalculatedNonce(request, this->nonce);

	return new OkPubsubResponse();
}

void PoWPoolNonceCalculatedCommand::setPoWNonce(const PoWNonce *nonce) {
	delete this->nonce, this->nonce = nullptr;
	this->nonce = dynamic_cast<PoWNonce*>(nonce->copyData());
}

} /* namespace codablecash */
