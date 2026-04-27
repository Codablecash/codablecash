/*
 * ClientBlockMinedCommand.cpp
 *
 *  Created on: Aug 18, 2025
 *      Author: iizuka
 */

#include "bc_wallet_net_cmd/ClientBlockMinedCommand.h"
#include "bc_wallet_net_cmd/FetchHeaderTransactionsWorker.h"

#include "bc_wallet_net_data/NetworkWalletData.h"

#include "bc_wallet_net/WalletNetworkManager.h"
#include "bc_wallet_net/NetworkWallet.h"

#include "bc_network/NodeIdentifier.h"

#include "data_history_data/BlockHeaderTransferData.h"
#include "data_history/AbstractTransferedData.h"

#include "bc_base/BinaryUtils.h"

#include "base/StackRelease.h"
#include "base/ArrayList.h"

#include "bc/ISystemLogger.h"

#include "bc_p2p_cmd_client/ClientBlockHeaderTransferData.h"
#include "bc_p2p_cmd_client/ClientFetchHeaderTransactionsCommandResponse.h"

#include "bc_trx/AbstractBlockchainTransaction.h"

#include "data_history_data/BlockTransactionTransferData.h"

#include "bc/ExceptionThrower.h"

#include "pubsub/PubsubCommandException.h"
#include <cstdint>

#include "bc_blockstore_header/BlockHeaderStoreManager.h"
namespace codablecash {

ClientBlockMinedCommand::ClientBlockMinedCommand(const ClientBlockMinedCommand &inst) : AbstractClientQueueCommand(*this) {
	this->data = inst.data != nullptr ? dynamic_cast<BlockHeaderTransferData*>(inst.data->copyData()) : nullptr;
	this->sourceNodeId = inst.sourceNodeId != nullptr ? dynamic_cast<NodeIdentifier*>(inst.sourceNodeId->copyData()) : nullptr;
}

ClientBlockMinedCommand::ClientBlockMinedCommand() : AbstractClientQueueCommand(CLIENT_BLOCK_MINED) {
	this->data = nullptr;
	this->sourceNodeId = nullptr;
}

ClientBlockMinedCommand::~ClientBlockMinedCommand() {
	delete this->data;
	delete this->sourceNodeId;
}

int ClientBlockMinedCommand::binarySize() const {
	BinaryUtils::checkNotNull(this->data);
	BinaryUtils::checkNotNull(this->sourceNodeId);

	int total = AbstractClientQueueCommand::binarySize();

	total += this->data->binarySize();
	total += this->sourceNodeId->binarySize();

	return total;
}

void ClientBlockMinedCommand::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->data);
	BinaryUtils::checkNotNull(this->sourceNodeId);

	AbstractClientQueueCommand::toBinary(out);

	this->data->toBinary(out);
	this->sourceNodeId->toBinary(out);
}

void ClientBlockMinedCommand::fromBinary(ByteBuffer *in) {
	AbstractTransferedData* obj = AbstractTransferedData::createFromBinary(in); __STP(obj);

	BlockHeaderTransferData* d = dynamic_cast<BlockHeaderTransferData*>(obj);
	BinaryUtils::checkNotNull(d);

	__STP_MV(obj);
	this->data = d;

	this->sourceNodeId = NodeIdentifier::fromBinary(in);
}

IBlockObject* ClientBlockMinedCommand::copyData() const noexcept {
	return new ClientBlockMinedCommand(*this);
}

void ClientBlockMinedCommand::process(NetworkWallet *wallet) const {
	NetworkWalletData* walletData = wallet->getWalletData();
	WalletNetworkManager* networkManager = wallet->getWalletNetworkManager();
	ISystemLogger* logger = wallet->getLogger();

	const BlockHeader* header = this->data->getHeader();
	uint16_t zone = header->getZone();
	const BlockHeaderId* headerId = header->getId();
	uint64_t height = header->getHeight();

	// check orphan
	checkOrphan(header, wallet, networkManager, logger);

	BlockHeader* h = processBlock(zone, headerId, height, this->sourceNodeId, networkManager, logger, walletData); __STP(h);
}

BlockHeader* ClientBlockMinedCommand::processBlock(uint16_t zone, const BlockHeaderId* headerId, uint64_t height, const NodeIdentifier *nodeId, WalletNetworkManager *networkManager, ISystemLogger *logger, NetworkWalletData* walletData) const {
	BlockHeader* ret = nullptr;
	UnicodeString str(L"WalletHeaderTransactionSyncWk");
	bool finalized = false;

	logger->debugLog(ISystemLogger::DEBUG_TMP_INFO, &str, __FILE__, __LINE__);

	// add a worker
	FetchHeaderTransactionsWorker* worker = new FetchHeaderTransactionsWorker(zone, headerId, height, this->sourceNodeId, networkManager, logger, &str); __STP(worker);

	worker->start();
	worker->join();

	const ClientFetchHeaderTransactionsCommandResponse* res = worker->getResponse();

	if(res != nullptr){
		const ClientBlockHeaderTransferData* data = res->getTransferData();
		const BlockHeader* header = data->getHeader();
		ret = dynamic_cast<BlockHeader*>(header->copyData());

		const ArrayList<BlockTransactionTransferData>* list = data->getTransactionsList();
		ArrayList<AbstractBlockchainTransaction>* trxlist = new ArrayList<AbstractBlockchainTransaction>(); __STP(trxlist);
		trxlist->setDeleteOnExit();

		int maxLoop = list->size();
		for(int i = 0; i != maxLoop; ++i){
			BlockTransactionTransferData* data = list->get(i);
			const AbstractBlockchainTransaction* trx = data->getTransaction();

			AbstractBlockchainTransaction* t = dynamic_cast<AbstractBlockchainTransaction*>(trx->copyData());
			trxlist->addElement(t);
		}

		uint64_t lastFinalizedHeight = walletData->getFinalizedHeight();
		uint64_t startHeight = lastFinalizedHeight > 0 ? lastFinalizedHeight : 1;

		walletData->addHeader(header, trxlist);
		walletData->updateHeadDetection();
		finalized = walletData->checkAndFinalizing();

		// update
		if(finalized){
			walletData->updateHeadDetection();
		}

		walletData->buildManagementAccount(finalized, startHeight);
	}

	return ret;
}

void ClientBlockMinedCommand::checkOrphan(const BlockHeader *header, NetworkWallet *wallet, WalletNetworkManager *networkManager, ISystemLogger *logger) const {
	NetworkWalletData* walletData = wallet->getWalletData();

	uint16_t zone = header->getZone();
	BlockHeaderStoreManager* manager = walletData->getHeaderManager(zone);

	const BlockHeaderId* headerId = header->getLastHeaderId();
	uint64_t height = header->getHeight() - 1;

	BlockHeader* lastHeader = manager->getHeader(headerId, height);
	while(lastHeader == nullptr){
		delete lastHeader;

		lastHeader = processBlock(zone, headerId, height, this->sourceNodeId, networkManager, logger, walletData);

		headerId = lastHeader->getLastHeaderId();
		height = lastHeader->getHeight() - 1;
		lastHeader = manager->getHeader(headerId, height);
	}
	__STP(lastHeader);
}


void ClientBlockMinedCommand::setData(const BlockHeaderTransferData *data) {
	delete this->data;
	this->data = dynamic_cast<BlockHeaderTransferData*>(data->copyData());
}

void ClientBlockMinedCommand::setSourceNodeId(const NodeIdentifier *nodeId) {
	delete this->sourceNodeId;
	this->sourceNodeId = dynamic_cast<NodeIdentifier*>(nodeId->copyData());
}



} /* namespace codablecash */
