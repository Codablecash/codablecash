/*
 * SimnetClient.cpp
 *
 *  Created on: Sep 3, 2026
 *      Author: iizuka
 */

#include "SimnetClient.h"
#include "sim_net_client/SimnetPubsubCommandExecutor.h"

#include "bc/ISystemLogger.h"

#include "pubsub/P2pHandshake.h"
#include "pubsub/PubSubId.h"

#include "base/StackRelease.h"


namespace codablecash {

SimnetClient::SimnetClient(ISystemLogger* logger) {
	this->logger = logger;
	this->handshake = nullptr;
	this->exec = new SimnetPubsubCommandExecutor(this);
}

SimnetClient::~SimnetClient() {
	shutdown();

	delete this->exec;
}

void SimnetClient::shutdown() {
	if(this->handshake != nullptr){
		this->handshake->dispose();
		delete this->handshake;
		this->handshake = nullptr;
	}
}

void SimnetClient::start(int protocol, const UnicodeString *host, int port) {
	PubSubId* psId = PubSubId::createNewId(); __STP(psId);

	// establish handshake
	this->handshake = new P2pHandshake(psId, logger);
	this->handshake->connect(protocol, host, port, this->exec, true);
}

} /* namespace codablecash */
