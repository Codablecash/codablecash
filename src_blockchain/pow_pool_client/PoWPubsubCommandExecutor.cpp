/*
 * PoWPubsubCommandExecutor.cpp
 *
 *  Created on: Apr 18, 2026
 *      Author: iizuka
 */

#include "pow_pool_client/PoWPubsubCommandExecutor.h"
#include "pow_pool_client/PoWWorkerClient.h"


namespace codablecash {

PoWPubsubCommandExecutor::PoWPubsubCommandExecutor(PoWWorkerClient* client) {
	this->client = client;
}

PoWPubsubCommandExecutor::~PoWPubsubCommandExecutor() {

}

void PoWPubsubCommandExecutor::fireExecuteCommand(const PubSubId *pubsubId, const AbstractPubSubCommand *cmd) {
}

void PoWPubsubCommandExecutor::fireOnExeptionOnSubscriber(const PubSubId *pubsubId, const Exception *e) {
}

void PoWPubsubCommandExecutor::fireOnSubscribeEnds(const PubSubId *pubsubId) {
}

IPubsubCommandExecutor* PoWPubsubCommandExecutor::getExecutor() const noexcept {
	return this->client;
}

} /* namespace codablecash */
