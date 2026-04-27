/*
 * AbstractZoneSync.cpp
 *
 *  Created on: 2023/11/01
 *      Author: iizuka
 */

#include "bc_network_instance_sync/AbstractZoneSync.h"
#include "bc_network_instance_sync/ZoneBlockchainSync.h"
#include "bc_network_instance_sync/ZoneHeaderSync.h"
#include "bc_network_instance_sync/NetworkNodeSync.h"
#include "bc_network_instance_sync/RamdomNodesSelector.h"

#include "bc_network_instance_sync_repo/SyncHeaderRepo.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"
#include "base/ArrayList.h"

#include "bc_network/NodeIdentifier.h"

#include "bc_network_instance/CodablecashNetworkNode.h"

#include "bc/CodablecashNodeInstance.h"
#include "bc_p2p/BlochchainP2pManager.h"

#include "bc_network_instance_sync/HeaderSyncWorker.h"

#include "base_io/File.h"

#include "data_history_data/BlockHeaderTransferData.h"

#include "base_thread/StackUnlocker.h"
#include "base_thread/SysMutex.h"
#include "bc/CodablecashSystemParam.h"

#include "bc/ISystemLogger.h"

namespace codablecash {

AbstractZoneSync::AbstractZoneSync(uint16_t zone, NetworkNodeSync* parent, ISystemLogger* logger, const UnicodeString* name)
		: AbstractSyncWorkerThread(logger, name){
	this->parent = parent;
	this->zone = zone;

	this->workers = new ArrayList<HeaderSyncWorker>();
	this->headerDataStore = nullptr;
	this->headerDataMutex = new SysMutex();
}

AbstractZoneSync::~AbstractZoneSync() {
	this->parent = nullptr;

	this->workers->deleteElements();
	delete this->workers;

	delete this->headerDataStore;
	delete this->headerDataMutex;
}

AbstractZoneSync* AbstractZoneSync::createZone(uint16_t zone,	uint16_t zoneSelf, NetworkNodeSync *parent, ISystemLogger* logger) {
	UnicodeString str(L"SYNC_ZONE_");
	str.append((int64_t)zone);

	if(zone == zoneSelf){
		return new ZoneBlockchainSync(zone, parent, logger, &str);
	}

	return new ZoneHeaderSync(zone, parent, logger, &str);
}

void AbstractZoneSync::doProcess() {
	initHeadersRepo();
	importHeaders();

	importResult();
}

CodablecashNetworkNode* AbstractZoneSync::getCodablecashNetworkNode() const noexcept {
	return this->parent->getCodablecashNetworkNode();
}

void AbstractZoneSync::importHeaders() noexcept {
	CodablecashNetworkNode* node = this->parent->getCodablecashNetworkNode();
	CodablecashNodeInstance* inst = node->getInstance();
	CodablecashSystemParam* param = inst->getCodablecashSystemParam();

	BlochchainP2pManager* p2pManager = inst->getBlochchainP2pManager();
	{
		ArrayList<NodeIdentifier>* nodelist = p2pManager->getNodeIds(this->zone); __STP(nodelist);
		nodelist->setDeleteOnExit();

		int num = param->getNumInitialSyncNodes();
		RamdomNodesSelector sel(nodelist, num);

		int threadindex = 0;
		while(sel.hasNext()){
			const NodeIdentifier* nodeId = sel.next();

			UnicodeString str(L"AbstractZoneSyncWk");
			str.append((int64_t)threadindex);
			threadindex++;

			this->logger->debugLog(ISystemLogger::DEBUG_TMP_INFO, &str, __FILE__, __LINE__);

			// add a worker
			HeaderSyncWorker* worker = new HeaderSyncWorker(this->zone, nodeId, this, this->logger, &str);
			this->workers->addElement(worker);

			worker->start();
		}

		int maxLoop = this->workers->size();
		for(int i = 0; i != maxLoop; ++i){
			HeaderSyncWorker* worker = this->workers->get(i);
			worker->join();
		}
	}

	// AbstractZoneSync::importHeaders()
	UnicodeString __str(L"AbstractZoneSync::importHeaders() was DONE.");
	this->logger->debugLog(ISystemLogger::DEBUG_TMP_INFO, &__str, __FILE__, __LINE__);
}

void AbstractZoneSync::initHeadersRepo() {
	const File* syncBaseDir = this->parent->getTmpSyncDir();

	UnicodeString zoneStr(L"zonedata_");
	zoneStr.append((int64_t)this->zone);

	File* zoneDir = syncBaseDir->get(&zoneStr); __STP(zoneDir);
	zoneDir->mkdirs();

	this->headerDataStore = new SyncHeaderRepo(zoneDir);
	this->headerDataStore->createBlankDatabase();

	this->headerDataStore->open();
}

void AbstractZoneSync::addHeaderData(const SyncHeaderHeightData *headerData) {
	StackUnlocker __lock(this->headerDataMutex, __FILE__, __LINE__);

	this->headerDataStore->add(headerData);
}

} /* namespace codablecash */
