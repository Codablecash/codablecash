/*
 * Pool2ClientTransferCommandMessage.cpp
 *
 *  Created on: Apr 23, 2026
 *      Author: iizuka
 */

#include "pow_pool/Pool2ClientTransferCommandMessage.h"
#include "pow_pool/NetworkPoWPool.h"

#include "pow_pool_cmd/AbstractPoWClientNotifyCommand.h"

#include "p2pserver/P2pServer.h"
#include "p2pserver/P2pServerConnectionManager.h"

#include "pubsub/CommandPublisher.h"
#include "pubsub/PubSubId.h"
#include "pubsub/P2pHandshake.h"

#include "pubsub_cmd/AbstractCommandResponse.h"

#include "bc_p2p/StackRawHandshakeReleaser.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "bc/ISystemLogger.h"

namespace codablecash {

Pool2ClientTransferCommandMessage::Pool2ClientTransferCommandMessage(const PubSubId *pusubId, const AbstractPoWClientNotifyCommand* command) {
	this->pusubId = new PubSubId(*pusubId);
	this->command = dynamic_cast<AbstractPoWClientNotifyCommand*>(command->copyData());
}

Pool2ClientTransferCommandMessage::~Pool2ClientTransferCommandMessage() {
	delete this->pusubId;
	delete this->command;
}

void Pool2ClientTransferCommandMessage::execute(ICommandParameter *param) {
	NetworkPoWPool* pool = dynamic_cast<NetworkPoWPool*>(param);
	P2pServer* p2pServer = pool->getP2pServer();
	ISystemLogger* logger = pool->getlogger();

	P2pServerConnectionManager* conManager = p2pServer->getConnectionManager();

	P2pHandshake* handshake = conManager->getP2pHandshake(this->pusubId);
	StackRawHandshakeReleaser __releaser(handshake);

	AbstractCommandResponse* response = handshake->publishCommand(this->command); __STP(response);
	if(response != nullptr){
		UnicodeString* message = response->toString(); __STP(message);

		logger->debugLog(ISystemLogger::DEBUG_NODE_TRANSFER_RESPONSE, message, __FILE__, __LINE__);
	}
}

} /* namespace codablecash */
