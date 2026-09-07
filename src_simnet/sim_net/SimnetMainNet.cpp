/*
 * SimnetMainNet.cpp
 *
 *  Created on: Sep 3, 2026
 *      Author: iizuka
 */

#include "SimnetMainNet.h"

#include "bc/ISystemLogger.h"

#include "bc_p2p_info/P2pNodeRecord.h"

#include "p2pserver/P2pServer.h"

#include "base/UnicodeString.h"

namespace codablecash {

SimnetMainNet::SimnetMainNet(ISystemLogger *logger) {
	this->logger = logger;
	this->p2pServer = nullptr;

	this->protocol = P2pNodeRecord::TCP_IP_V6;
	this->srvhost = nullptr;
	this->port = 18081;
}

SimnetMainNet::~SimnetMainNet() {
	shutdown();

	delete this->srvhost;

	this->logger = nullptr;
}

void SimnetMainNet::shutdown() {
	if(this->p2pServer != nullptr){
		this->p2pServer->stopListning();
		this->p2pServer->dispose();
		delete this->p2pServer, this->p2pServer = nullptr;
	}
}

void SimnetMainNet::setServerNetInfo(int protocol, const UnicodeString *srvhost, int port) {
	this->protocol = protocol;
	this->srvhost = new UnicodeString(*srvhost);
	this->port = port;
}

void SimnetMainNet::startNetwork() {
	this->p2pServer = new P2pServer(this->logger, this);

	UnicodeString node(L"simnetIf");
	this->p2pServer->startListening(this->protocol, this->srvhost, this->port, &node);
}

} /* namespace codablecash */
