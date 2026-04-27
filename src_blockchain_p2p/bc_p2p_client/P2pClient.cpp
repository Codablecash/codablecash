/*
 * P2pClient.cpp
 *
 *  Created on: 2023/09/09
 *      Author: iizuka
 */

#include "bc_p2p_client/P2pClient.h"

#include "bc_p2p_cmd/LoginClientCommand.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "pubsub/PubSubId.h"
#include "pubsub/P2pHandshake.h"

#include "pubsub_cmd/AbstractCommandResponse.h"

#include "bc_network/NodeIdentifierSource.h"

#include "bc_p2p_client/P2pClientConnectionException.h"

#include "bc_p2p_cmd_client/AbstractClientRequestCommand.h"

#include "bc_p2p_cmd_client/PingNodeCommand.h"

#include "bc/ISystemLogger.h"

#include "bc_p2p_cmd_client_notify/ClientExecutor.h"

#include "bc_p2p_info/P2pNodeRecord.h"
namespace codablecash {

P2pClient::P2pClient(uint16_t zone, ISystemLogger* logger) {
	this->logger = logger;
	this->zone = zone;

	this->hostName = nullptr;
	this->port = -1;
	this->psId = nullptr;
	this->handshake = nullptr;
	this->source = NodeIdentifierSource::create();

	this->commandExecutor = new ClientExecutor();
}

P2pClient::~P2pClient() {
	close(true);

	delete this->source;
	this->source = nullptr;

	delete this->commandExecutor;
}

void P2pClient::close() noexcept {
	close(false);
}

void P2pClient::close(bool force) noexcept {
	if(!force){
		sendEndConnectionCommand();
	}

	delete this->hostName;
	this->hostName = nullptr;

	delete this->psId;
	this->psId = nullptr;

	if(this->handshake != nullptr){
		this->handshake->dispose();
		delete this->handshake;
		this->handshake = nullptr;
	}
}

void P2pClient::sendEndConnectionCommand() {
	if(this->handshake != nullptr){
		this->handshake->sendEndConnectionCommand();
	}
}

void P2pClient::connect(int protocol, const UnicodeString *hostName, int port) {
	if(protocol == P2pNodeRecord::TCP_IP_V4){
		connectIpV4(hostName, port);
	}
	else if(protocol == P2pNodeRecord::TCP_IP_V6){
		connectIpV6(hostName, port);
	}
}

void P2pClient::connectIpV4(const UnicodeString *hostName, int port) {
	close();

	__connectIpV4(hostName, port);
}

void P2pClient::__connectIpV4(const UnicodeString *hostName, int port) {
	this->psId = PubSubId::createNewId();
	this->handshake = new P2pHandshake(this->psId, this->logger);

	this->handshake->connectIpV4(hostName, port, this, true);

	__connectLogin(hostName, port);
}

void P2pClient::connectIpV6(const UnicodeString* hostName, int port) {
	close();

	__connectIpV6(hostName, port);
}

void P2pClient::__connectIpV6(const UnicodeString *hostName, int port) {
	this->psId = PubSubId::createNewId();
	this->handshake = new P2pHandshake(this->psId, this->logger);

	this->handshake->connectIpV6(hostName, port, this, true);

	__connectLogin(hostName, port);
}

void P2pClient::__connectLogin(const UnicodeString *hostName, int port) {
	LoginClientCommand cmd(this->zone);
	cmd.sign(this->source);

	AbstractCommandResponse* response = this->handshake->publishCommand(&cmd); __STP(response);
	uint32_t type = response->getType();

	if(type != AbstractCommandResponse::TYPE_RES_OK){
		close(true);
		throw new P2pClientConnectionException(__FILE__, __LINE__);
	}
}

void P2pClient::sign(AbstractBlockchainPubsubCommand *cmd) const {
	cmd->sign(this->source);
}

AbstractCommandResponse* P2pClient::sendCommnad(const AbstractClientRequestCommand *cmd) {
	AbstractCommandResponse* response = this->handshake->publishCommand(cmd);

	return response;
}

bool P2pClient::ping() {
	PingNodeCommand cmd;
	cmd.sign(this->source);

	AbstractCommandResponse* response = this->handshake->publishCommand(&cmd); __STP(response);
	uint32_t type = response->getType();

	return type == AbstractCommandResponse::TYPE_RES_PING;
}

void P2pClient::fireExecuteCommand(const PubSubId* pubsubId, const AbstractPubSubCommand *cmd) {
	// do nothing
}

void P2pClient::fireOnExeptionOnSubscriber(const PubSubId* pubsubId, const Exception *e) {
	this->logger->logException(e);

	this->handshake->fireOnExeptionOnSubscriber(pubsubId, e);
}

void P2pClient::fireOnSubscribeEnds(const PubSubId* pubsubId) {
	this->handshake->fireOnSubscribeEnds(pubsubId);
}

IPubsubCommandExecutor* P2pClient::getExecutor() const noexcept {
	return this->commandExecutor;
}

void P2pClient::addListner(IClientNotifyListner *listner) noexcept {
	this->commandExecutor->addListner(listner);
}

bool P2pClient::removeListner(IClientNotifyListner *listner) noexcept {
	IClientNotifyListner* ret = this->commandExecutor->removeListner(listner);

	return listner == ret;
}

} /* namespace codablecash */
