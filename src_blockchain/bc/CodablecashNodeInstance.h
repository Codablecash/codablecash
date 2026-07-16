/*
 * CodablecashNodeInstance.h
 *
 *  Created on: 2023/03/04
 *      Author: iizuka
 */

#ifndef BC_CODABLECASHNODEINSTANCE_H_
#define BC_CODABLECASHNODEINSTANCE_H_

#include <cstdint>

#include "pubsub/IPubsubCommandExecutor.h"
namespace alinous {
class File;
class UnicodeString;
}
using namespace alinous;

namespace codablecash {

class CodablecashBlockchain;
class MemoryPool;
class P2pServer;
class ISystemLogger;
class BlockchainStatusCache;
class BlockchainController;
class PoWManager;
class BlockGenerator;
class MiningConfig;
class CodablecashSystemParam;
class CentralProcessor;
class BlochchainP2pManager;
class FinalizerPool;
class NodeIdentifierSource;
class IInstanceMemberAllocator;
class P2pRequestProcessor;
class P2pDnsManager;
class P2pHandshake;
class P2pNodeRecord;
class NodeIdentifier;
class AbstractShardExtentionValidator;


class CodablecashNodeInstance : public IPubsubCommandExecutor {
public:
	static const constexpr wchar_t* BLOCKCHAIN_DIR{L"blockchain"};
	static const constexpr wchar_t* MEMORY_POOL_DIR{L"memorypool"};
	static const constexpr wchar_t* STATUS_CACHE_DIR{L"statuscache"};
	static const constexpr wchar_t* TMP_CACHE_DIR{L"tmpcache"};

	static const constexpr int IP_V4{4};
	static const constexpr int IP_V6{6};

	CodablecashNodeInstance(const File* baseDir, ISystemLogger* logger, const CodablecashSystemParam* param, const IInstanceMemberAllocator* allocator);
	CodablecashNodeInstance(const File* baseDir, ISystemLogger* logger, const CodablecashSystemParam* param);
	virtual ~CodablecashNodeInstance();

	bool initBlankInstance(uint16_t zoneSelf, uint16_t numZones);

	void load();
	void initCacheStatus();

	void startBlockGenerator(const MiningConfig* config);
	void suspendBlockGenerator();
	void resumeBlockGenerator();
	bool isBlockGeneratorSuspendStatus() const noexcept;

	void startNetwork(const UnicodeString *host, int port);
	void startProcessors(const NodeIdentifierSource* networkKey, bool suspend);

	void startBlockFinalizer(const NodeIdentifierSource *nodeSource);

	void shutdownNetwork();
	void shutdown();

	void connectIpV6Node(uint16_t zone, const UnicodeString* host, int port, const NodeIdentifier* nodeId, const UnicodeString* canonicalName);
	void connectIpV4Node(uint16_t zone, const UnicodeString* host, int port, const NodeIdentifier* nodeId, const UnicodeString* canonicalName);
	void doConnectIpNode(uint16_t zone, const UnicodeString* host, int port, int protocol, const NodeIdentifier* nodeId, const UnicodeString* canonicalName);
	void loginNode(uint16_t zone, P2pHandshake *handshake, const UnicodeString* canonicalName);

	void maintainNetwork();

	BlockchainController* getController() const noexcept {
		return this->ctrl;
	}
	MemoryPool* getMemoryPool() const noexcept {
		return this->memoryPool;
	}

	BlochchainP2pManager* getBlochchainP2pManager() const noexcept {
		return this->p2pManager;
	}
	P2pRequestProcessor* getP2pRequestProcessor() const noexcept {
		return this->p2pRequestProcessor;
	}
	P2pDnsManager* getP2pDnsManager() const noexcept {
		return this->p2pDnsManager;
	}

	NodeIdentifierSource* getNetworkKey() const noexcept;

	uint16_t getZoneSelf() const noexcept;
	int getNumZones() const noexcept;

	CodablecashSystemParam* getCodablecashSystemParam() const noexcept {
		return this->param;
	}
	ISystemLogger* getLogger() const noexcept {
		return this->logger;
	}
	CentralProcessor* getCentralProcessor() const noexcept {
		return this->centralProcessor;
	}
	FinalizerPool* getFinalizerPool() const noexcept {
		return this->finalizer;
	}

	void resumeRequestProcessor();

	File* getTempCacheDir() const;

	void setNodeName(const UnicodeString* name) noexcept;
	const UnicodeString* getNodeName() const noexcept {
		return this->nodeName;
	}

	void validateZone(uint16_t zone) const;

	void broadCastShutdownCommand(const NodeIdentifierSource *source);

	int getListningPort() const noexcept;
	BlockGenerator* getBlockGenerator() const noexcept {
		return this->blockGenerator;
	}

	void setShardExtendValidator(const AbstractShardExtentionValidator *validator);

private:
	void __init(const File* baseDir, ISystemLogger* logger, const CodablecashSystemParam* config);
	void __maintainNetwork(uint16_t zone);

	bool __connectWithP2pNodeRecord(const P2pNodeRecord* record);

private:
	IInstanceMemberAllocator* allocator;

	File* baseDir;
	CodablecashSystemParam* param;

	CentralProcessor* centralProcessor;

	MemoryPool* memoryPool;

	CodablecashBlockchain* blockchain;
	BlockchainStatusCache* statusCache;
	BlockchainController* ctrl;

	PoWManager* powManager;
	BlockGenerator* blockGenerator;

	FinalizerPool* finalizer;

	P2pServer* p2pServer;
	BlochchainP2pManager* p2pManager;

	// P2P Request processor
	P2pRequestProcessor* p2pRequestProcessor;
	P2pDnsManager* p2pDnsManager;

	ISystemLogger* logger;

	UnicodeString* nodeName;
};

} /* namespace codablecash */

#endif /* BC_CODABLECASHNODEINSTANCE_H_ */
