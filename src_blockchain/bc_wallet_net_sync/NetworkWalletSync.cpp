/*
 * NetworkWalletSync.cpp
 *
 *  Created on: Apr 18, 2025
 *      Author: iizuka
 */

#include "bc_wallet_net_sync/NetworkWalletSync.h"
#include "bc_wallet_net_sync/WalletHeaderSyncWorker.h"

#include "bc_wallet_net/WalletNetworkManager.h"
#include "bc_wallet_net/WalletConfig.h"

#include "bc_wallet_net_data/NetworkWalletData.h"

#include "base/StackRelease.h"

#include "base_io/File.h"

#include "bc_network/NodeIdentifier.h"

#include "bc_network_instance_sync/RamdomNodesSelector.h"

#include "bc_network_instance_sync_repo/SyncHeaderRepo.h"
#include "bc_network_instance_sync_repo/HeightHeaderSyncDataList.h"

#include "bc/ISystemLogger.h"

#include "base_thread/SysMutex.h"
#include "base_thread/StackUnlocker.h"

#include "bc_block/BlockHeader.h"

#include "data_history_data/BlockHeaderTransferData.h"
#include "data_history_data/BlockTransactionTransferData.h"

#include "bc_trx/AbstractBlockchainTransaction.h"


namespace codablecash {

NetworkWalletSync::NetworkWalletSync(uint64_t startHeight, NetworkWalletData* walletData, WalletNetworkManager* networkManager, const WalletConfig* walletConfig, ISystemLogger* logger) {
	this->startHeight = startHeight;

	this->walletData = walletData;
	this->networkManager = networkManager;
	this->walletConfig = walletConfig;
	this->logger = logger;
	this->tmpSyncDir = nullptr;

	this->headerDataMutex = new SysMutex();
	this->headerDataStore = nullptr;
}

NetworkWalletSync::~NetworkWalletSync() {
	this->walletData = nullptr;
	this->networkManager = nullptr;

	if(this->tmpSyncDir != nullptr){
		this->tmpSyncDir->deleteDir();
	}
	delete this->tmpSyncDir;

	delete this->headerDataStore;
	delete this->headerDataMutex;
}

void NetworkWalletSync::sync() {
	// base dir
	{
		const File* tmpDir = walletData->getBaseDir();

		// reset
		delete this->tmpSyncDir;
		this->tmpSyncDir = tmpDir->get(SYNC_DIR);
		this->tmpSyncDir->deleteDir();
		this->tmpSyncDir->mkdirs();
	}
	initHeadersRepo();

	uint16_t zone = this->walletData->getDefaultZone();
	ArrayList<NodeIdentifier>* nodelist = this->networkManager->getNodeIdList(); __STP(nodelist);
	nodelist->setDeleteOnExit();

	syncHeader(zone, nodelist);
	processHeaders();
	__buildManagementAccounts();
}

void NetworkWalletSync::__buildManagementAccounts() {
	this->walletData->resetManagementAccounts();

	// buildManagementAccounts
	uint64_t finalizedHeight = this->walletData->getFinalizedHeight();
	uint64_t startHeight = finalizedHeight > 0 ? finalizedHeight : 1;

	this->walletData->buildManagementAccount(true, startHeight);
}

void NetworkWalletSync::processHeaders() {
	StackUnlocker __lock(this->headerDataMutex, __FILE__, __LINE__);

	// import headers
	while(!this->headerDataStore->isEmpty()){
		HeightHeaderSyncDataList* datalist = this->headerDataStore->fetchFirst(); __STP(datalist);

		const ArrayList<BlockHeaderTransferData>* list = datalist->getList();
		int maxLoop = list->size();
		for(int i = 0; i != maxLoop; ++i){
			const BlockHeaderTransferData* data = list->get(i);

			// add header
			{
				const BlockHeader* header = data->getHeader();
				const ArrayList<BlockTransactionTransferData>* trxdatalist = data->getTransactionList();

				ArrayList<AbstractBlockchainTransaction> trxlist;
				trxlist.setDeleteOnExit();

				int numTrx = trxdatalist->size();
				for(int j = 0; j != numTrx; ++j){
					const BlockTransactionTransferData* trxData = trxdatalist->get(j);
					const AbstractBlockchainTransaction* trx = trxData->getTransaction();

					trxlist.addElement(dynamic_cast<AbstractBlockchainTransaction*>(trx->copyData()));
				}

				this->walletData->addHeader(header, &trxlist);
				this->walletData->updateHeadDetection();
			}
		}
	}
}

void NetworkWalletSync::syncHeader(uint16_t zone, ArrayList<NodeIdentifier>* nodelist) {
	int num = this->walletConfig->getNumInitialSyncNodes();

	RamdomNodesSelector sel(nodelist, num);

	ArrayList<WalletHeaderSyncWorker>* workers = new ArrayList<WalletHeaderSyncWorker>(); __STP(workers);
	workers->setDeleteOnExit();

	int threadindex = 0;
	while(sel.hasNext()){
		const NodeIdentifier* nodeId = sel.next();

		UnicodeString str(L"WalletHeaderSyncWk");
		str.append((int64_t)threadindex);
		threadindex++;

		this->logger->debugLog(ISystemLogger::DEBUG_TMP_INFO, &str, __FILE__, __LINE__);

		// add a worker
		WalletHeaderSyncWorker* worker = new WalletHeaderSyncWorker(zone, nodeId, this->startHeight, this, this->logger, &str);
		workers->addElement(worker);

		worker->start();
	}

	int maxLoop = workers->size();
	for(int i = 0; i != maxLoop; ++i){
		WalletHeaderSyncWorker* worler = workers->get(i);
		worler->join();
	}

	// NetworkWalletSync::sync()
	UnicodeString __str(L"NetworkWalletSync::syncHeader() was DONE.");
	this->logger->debugLog(ISystemLogger::DEBUG_TMP_INFO, &__str, __FILE__, __LINE__);
}

void NetworkWalletSync::addHeaderData(const ClientSyncHeaderHeightData *headerData) {
	StackUnlocker __lock(this->headerDataMutex, __FILE__, __LINE__);

	this->headerDataStore->add(headerData);
}

void NetworkWalletSync::initHeadersRepo() {
	uint16_t zone = this->walletData->getDefaultZone();

	UnicodeString zoneStr(L"zonedata_");
	zoneStr.append((int64_t)zone);

	File* zoneDir = this->tmpSyncDir->get(&zoneStr); __STP(zoneDir);
	zoneDir->mkdirs();

	this->headerDataStore = new SyncHeaderRepo(zoneDir);
	this->headerDataStore->createBlankDatabase();

	this->headerDataStore->open();
}

} /* namespace codablecash */
