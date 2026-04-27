/*
 * MemPoolSync.cpp
 *
 *  Created on: 2023/11/03
 *      Author: iizuka
 */

#include "bc_network_instance_sync/MemPoolSync.h"
#include "bc_network_instance_sync/NetworkNodeSync.h"
#include "bc_network_instance_sync/RamdomNodesSelector.h"
#include "bc_network_instance_sync/MemPoolSyncWorker.h"

#include "bc_network_instance/CodablecashNetworkNode.h"

#include "bc_network/NodeIdentifier.h"

#include "bc/CodablecashNodeInstance.h"
#include "bc_p2p/BlochchainP2pManager.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"
#include "bc/CodablecashSystemParam.h"


namespace codablecash {

MemPoolSync::MemPoolSync(NetworkNodeSync* parent, ISystemLogger* logger, const UnicodeString* name) : AbstractSyncWorkerThread(logger, name) {
	this->parent = parent;
	this->workers = new ArrayList<MemPoolSyncWorker>();
}

MemPoolSync::~MemPoolSync() {
	this->parent = nullptr;

	this->workers->deleteElements();
	delete this->workers;
}

void MemPoolSync::doProcess() {
	CodablecashNetworkNode* node = this->parent->getCodablecashNetworkNode();
	CodablecashNodeInstance* inst = node->getInstance();
	CodablecashSystemParam* param = inst->getCodablecashSystemParam();

	uint16_t zoneSelf = inst->getZoneSelf();

	// extra zone
	uint16_t numZones = inst->getNumZones();
	if(numZones <= zoneSelf){
		return;
	}

	BlochchainP2pManager* p2pManager = inst->getBlochchainP2pManager();
	{
		ArrayList<NodeIdentifier>* nodelist = p2pManager->getNodeIds(zoneSelf); __STP(nodelist);
		nodelist->setDeleteOnExit();

		int num = param->getNumInitialSyncNodes();
		RamdomNodesSelector sel(nodelist, num);

		int threadindex = 0;
		while(sel.hasNext()){
			const NodeIdentifier* nodeId = sel.next();

			UnicodeString str(L"MemPoolSyncWk");
			str.append((int64_t)threadindex);
			threadindex++;

			MemPoolSyncWorker* worker = new MemPoolSyncWorker(nodeId, this->parent, this->logger, &str);
			this->workers->addElement(worker);

			worker->start();
		}
	}

	int maxLoop = this->workers->size();
	for(int i = 0; i != maxLoop; ++i){
		MemPoolSyncWorker* worker = this->workers->get(i);
		worker->join();
	}
}

} /* namespace codablecash */
