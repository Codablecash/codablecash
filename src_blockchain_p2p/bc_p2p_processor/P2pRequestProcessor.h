/*
 * P2pRequestProcessor.h
 *
 *  Created on: 2023/09/02
 *      Author: iizuka
 */

#ifndef BC_P2P_PROCESSOR_P2PREQUESTPROCESSOR_H_
#define BC_P2P_PROCESSOR_P2PREQUESTPROCESSOR_H_

#include "procesor/ICommandParameter.h"

namespace alinous {
class UnicodeString;
class File;
class SynchronizedLock;
}
using namespace alinous;

namespace codablecash {

class NodeIdentifierSource;
class NetworkInfoProcessor;
class NetworkTransferProcessor;
class CodablecashNodeInstance;
class P2pRequestQueueProcessor;
class DataHistory;
class BlochchainP2pManager;
class CodablecashSystemParam;
class ISystemLogger;
class AbstractTransferedData;
class AbstructNodeQueueCommand;
class PubSubId;
class PendingProcessorCommandsQueue;
class AbstractNodeCommand;
class PendingCommandData;
class P2pDnsManager;

class P2pRequestProcessor : public ICommandParameter {
public:
	static const constexpr wchar_t* BASE_DIR{L"p2pprocessor"};

	P2pRequestProcessor(const File* baseDir, BlochchainP2pManager* p2pManager, P2pDnsManager* p2pDnsManager, CodablecashSystemParam* config, ISystemLogger* logger);
	virtual ~P2pRequestProcessor();

	void createBlank(CodablecashNodeInstance* inst);

	void open(const NodeIdentifierSource* networkKey, CodablecashNodeInstance* inst, bool suspend);

	void close();

	NodeIdentifierSource* getNetworkKey() const noexcept {
		return this->networkKey;
	}

	bool hasHistory(const AbstractTransferedData* data);
	void addHistory(const AbstractTransferedData* data);

	void putQueue(const PubSubId* pubsubId, const AbstructNodeQueueCommand* command);

	NetworkInfoProcessor* getNetworkInfoProcessor() const noexcept {
		return this->infoProcessor;
	}

	NetworkTransferProcessor* getNodeTransferProcessor() const noexcept {
		return this->transferProcessor;
	}

	BlochchainP2pManager* getBlochchainP2pManager() const noexcept {
		return this->p2pManager;
	}

	P2pDnsManager* getP2pDnsManager() const noexcept {
		return this->p2pDnsManager;
	}

	ISystemLogger* getLogger() const noexcept {
		return this->logger;
	}

	P2pRequestQueueProcessor* getP2pRequestQueueProcessor() const noexcept {
		return this->queueProcessor;
	}

	//void setSuspend(bool suspend);
	bool isSuspended() const noexcept;
	bool __isSuspended() const noexcept;
	SynchronizedLock* getSynchrinizedLock() const noexcept;

	void __putPendingCommand(const AbstractNodeCommand* commnad, const PubSubId *pubsubId);

	void resume(CodablecashNodeInstance* inst);

	void setNodeName(const UnicodeString* name) noexcept;
	const UnicodeString* getNodeName() const noexcept {
		return this->nodeName;
	}

private:
	void processData(CodablecashNodeInstance* inst, PendingCommandData* data);

	bool isPendingQueueEmptyWithResume(SynchronizedLock* lock) const;
	PendingCommandData* fetchFirstPendingData(SynchronizedLock *lock) const;

private:
	File* baseDir;
	CodablecashSystemParam* config;
	ISystemLogger* logger;
	BlochchainP2pManager* p2pManager;
	P2pDnsManager* p2pDnsManager;

	NodeIdentifierSource* networkKey;

	NetworkInfoProcessor* infoProcessor; // handle DNS info
	NetworkTransferProcessor* transferProcessor; // handle node 2 node & node 2 client info

	P2pRequestQueueProcessor* queueProcessor;

	DataHistory* historyStore;

	PendingProcessorCommandsQueue* pendingQueue;

	UnicodeString* nodeName;
};

} /* namespace codablecash */

#endif /* BC_P2P_PROCESSOR_P2PREQUESTPROCESSOR_H_ */
