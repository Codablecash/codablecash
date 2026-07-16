/*
 * CodablecashNetworkInstance.h
 *
 *  Created on: 2023/08/21
 *      Author: iizuka
 */

#ifndef BC_NETWORK_INSTANCE_CODABLECASHNETWORKNODE_H_
#define BC_NETWORK_INSTANCE_CODABLECASHNETWORKNODE_H_

#include <cstdint>

namespace alinous {
class File;
}
using namespace alinous;

namespace codablecash {

class CodablecashNodeInstance;
class CodablecashNetworkNodeConfig;
class ISystemLogger;
class INetworkSeeder;
class P2pNodeRecord;
class NodeIdentifierSource;
class BlockchainNodeHandshake;
class NodeIdentifier;

class CodablecashNetworkNode {
public:
	CodablecashNetworkNode(const File* baseDir, const CodablecashNetworkNodeConfig* nwconfig, ISystemLogger* logger);
	virtual ~CodablecashNetworkNode();

	void shutdown() noexcept;

	void setNetworkConfig(const CodablecashNetworkNodeConfig* nwconfig);

	bool initBlank(uint16_t zoneSelf, int numZones);
	bool generateNetwork(uint16_t zoneSelf, int numZones);
	void startNetwork(INetworkSeeder* seeder, bool pending);
	void syncNetwork();

	void startMiner();
	void requestMining();
	void suspendMining();
	void resumeMining();

	void startStakePool();
	const NodeIdentifierSource* getVoterSource() const noexcept;

	CodablecashNodeInstance* getInstance() const noexcept {
		return this->inst;
	}

	void setNodeName(const wchar_t* name) noexcept;

	uint64_t getMaxHeight(uint16_t zone) const;

	BlockchainNodeHandshake* getNodeHandshakeByNodeId(const NodeIdentifier* nodeId) const noexcept;

	void broadCastShutdownCommand();



private:
	void initDatabase(ArrayList<P2pNodeRecord>* seedList);
	void maintainNetwork();

private:
	File* baseDir;
	CodablecashNetworkNodeConfig* nwconfig;
	CodablecashNodeInstance* inst;
	ISystemLogger* logger;

	UnicodeString* nodeName;
};

} /* namespace codablecash */

#endif /* BC_NETWORK_INSTANCE_CODABLECASHNETWORKNODE_H_ */
