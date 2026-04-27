/*
 * NetworkPoWPool.cpp
 *
 *  Created on: Apr 13, 2026
 *      Author: iizuka
 */

#include "pow_pool/NetworkPoWPool.h"
#include "pow_pool/PoWRequestStatus.h"
#include "pow_pool/PoWRequestStatusData.h"
#include "pow_pool/PoWRequestData.h"
#include "pow_pool/PoWRequest2Client.h"

#include "bc_block/BlockHeaderId.h"
#include "bc_block/BlockMerkleRoot.h"

#include "ipconnect/IServerSocket.h"

#include "p2pserver/P2pServer.h"
#include "p2pserver/P2pServerConnectionManager.h"

#include "bc_p2p_info/P2pNodeRecord.h"

#include "numeric/BigInteger.h"

#include "pow/PoWNonce.h"
#include "pow/PoWNonceResult.h"
#include "pow/PoWManager.h"

#include "base/StackRelease.h"

#include "procesor_multi/MultipleCommandProcessor.h"

#include "pubsub/PubSubId.h"

#include "pow_pool/Pool2ClientTransferCommandMessage.h"

#include "pow_pool_cmd/PoWPoolNotifyDataChangedCommand.h"


namespace codablecash {

NetworkPoWPool::NetworkPoWPool(PoWManager* powManager) {
	this->status = new PoWRequestStatus();
	this->logger = nullptr;

	this->protocol = P2pNodeRecord::TCP_IP_V6;
	this->srvhost = nullptr;
	this->port = 18080;

	this->p2pServer = nullptr;
	this->powManager = powManager;

	this->suspended = false;
	this->executor = nullptr;
}

NetworkPoWPool::~NetworkPoWPool() {
	shutdown();

	delete this->status;
	delete this->srvhost;
}

/**
 * start mining
 * @param lastBlockHeight
 * @param lastBlockId
 * @param merkleRoot
 * @param tm
 * @param targetDifficulty
 */
void NetworkPoWPool::calculate(uint64_t lastBlockHeight, const BlockHeaderId *lastBlockId, const BlockMerkleRoot *merkleRoot,
		const SystemTimestamp *tm, const BigInteger *targetDifficulty) {
	if(!this->suspended){
		this->status->request(lastBlockHeight, lastBlockId, merkleRoot, tm, targetDifficulty);
	}
}

void NetworkPoWPool::init(ISystemLogger *logger) {
	this->logger = logger;
}

void NetworkPoWPool::shutdown() {
	if(this->p2pServer != nullptr){
		this->p2pServer->stopListning();
		this->p2pServer->dispose();

		delete this->p2pServer;
		this->p2pServer = nullptr;
	}

	if(this->executor != nullptr){
		this->executor->shutdown();
		delete this->executor;
		this->executor = nullptr;
	}
}

void NetworkPoWPool::suspend() {
	this->suspended = true;
}

void NetworkPoWPool::resume() {
	this->suspended = false;
}

bool NetworkPoWPool::isSuspendStatus() const noexcept {
	return this->suspended;
}

void NetworkPoWPool::startNetwork() {
	this->p2pServer = new P2pServer(this->logger, this);

	UnicodeString node(L"mining");
	this->p2pServer->startListening(this->protocol, this->srvhost, this->port, &node);

	UnicodeString name(L"PoWPooLPub");
	this->executor = new MultipleCommandProcessor(this, 4, this->logger, &name);
	this->executor->start();
}

void NetworkPoWPool::setProtocol(int type) {
	this->protocol = type;
}

void NetworkPoWPool::setServerHost(const UnicodeString *host) {
	this->srvhost = new UnicodeString(host);
}

void NetworkPoWPool::setPort(int port) {
	this->port = port;
}

void NetworkPoWPool::onClientCalculatedNonce(const PoWRequest2Client* request, const PoWNonce *nonce) {
	const BigInteger* targetDiff = this->status->getTargetDifficulty();

	PoWRequestData* data = request->getRequestData();
	BlockHeaderId* lastHeaderId = data->getLastBlockId();
	BlockMerkleRoot* merkleRoot = data->getMerkleRoot();
	SystemTimestamp* tm = data->getTimestamp();

	PoWNonceResult* result = nonce->calcResult(lastHeaderId, merkleRoot, tm); __STP(result);
	BigInteger diff = result->getDifficulty();

#ifdef __DEBUG__
	{
		uint64_t target = targetDiff->longValue();
		uint64_t cur = diff.longValue();

		bool bl = (target < cur);
	}
#endif

	if(diff.compareTo(*targetDiff) >= 0){
		this->status->setWaiting(true);

		uint64_t height = data->getLastBlockHeight();

		this->powManager->onNonceCalculated(height, lastHeaderId, nonce);

		// notify other pool users
		notifyClientsMined(height, lastHeaderId, nonce);
	}

	// FIXME call back to wallet
}

void NetworkPoWPool::notifyClientsMined(uint64_t height, const BlockHeaderId *bid, const PoWNonce *nonce) {
	P2pServerConnectionManager* conManager = this->p2pServer->getConnectionManager();

	ArrayList<PubSubId>* list = conManager->getHandshakeIdList(); __STP(list);
	list->setDeleteOnExit();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		const PubSubId *pusubId = list->get(i);
		PoWPoolNotifyDataChangedCommand* notifyCommand = new PoWPoolNotifyDataChangedCommand(); __STP(notifyCommand);

		notifyCommand->setHeight(height); // height is last block's height


		Pool2ClientTransferCommandMessage* message = new Pool2ClientTransferCommandMessage(pusubId, notifyCommand);

		this->executor->addCommandMessage(message, i);
	}
}

} /* namespace codablecash */
