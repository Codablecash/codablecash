/*
 * SimnetPubsubCommandExecutor.cpp
 *
 *  Created on: Sep 3, 2026
 *      Author: iizuka
 */

#include "sim_net_client/SimnetPubsubCommandExecutor.h"
#include "sim_net_client/SimnetClient.h"

#include "base/Exception.h"

#include "pubsub/PubSubId.h"

#include "pubsub_cmd/AbstractPubSubCommand.h"



namespace codablecash {

SimnetPubsubCommandExecutor::SimnetPubsubCommandExecutor(SimnetClient* client) {
	this->client = client;
}

SimnetPubsubCommandExecutor::~SimnetPubsubCommandExecutor() {

}

void SimnetPubsubCommandExecutor::fireOnExeptionOnSubscriber(const PubSubId *pubsubId, const Exception *e) {
}

void SimnetPubsubCommandExecutor::fireExecuteCommand(const PubSubId *pubsubId, const AbstractPubSubCommand *cmd) {
}

void SimnetPubsubCommandExecutor::fireOnSubscribeEnds(const PubSubId *pubsubId) {
}

IPubsubCommandExecutor* SimnetPubsubCommandExecutor::getExecutor() const noexcept {
	return this->client;
}

} /* namespace codablecash */
