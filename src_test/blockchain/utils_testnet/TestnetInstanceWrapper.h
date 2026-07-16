/*
 * TestnetInstanceWrapper.h
 *
 *  Created on: Nov 20, 2024
 *      Author: iizuka
 */

#ifndef BLOCKCHAIN_UTILS_TESTNET_TESTNETINSTANCEWRAPPER_H_
#define BLOCKCHAIN_UTILS_TESTNET_TESTNETINSTANCEWRAPPER_H_
#include <cstdint>

namespace alinous {
class File;
class UnicodeString;
}
using namespace alinous;

namespace codablecash {

class CodablecashNetworkNodeConfig;
class ISystemLogger;
class CodablecashNetworkNode;
class GenesisBalanceConfig;
class MiningConfig;
class FinalizerConfig;
class IDebugSeeder;
class NodeIdentifierSource;
class TransactionId;
class IBlockGenerationListner;
class AbstractShardExtentionValidator;


class TestnetInstanceWrapper {
public:
	TestnetInstanceWrapper(uint16_t zone, int index);
	virtual ~TestnetInstanceWrapper();

	void init(const wchar_t* name, int port, const File* zoneBaseDir, const CodablecashNetworkNodeConfig* baseNwconfig, ISystemLogger* logger);

	void setGenesisBlockConfig(const GenesisBalanceConfig* gconfig);

	void setMinerConfig(const MiningConfig* minerConfig);
	void setFinalizerConfig(const FinalizerConfig* config);
	const FinalizerConfig* getFinalizerConfig() const noexcept;

	void initGenesis();
	void initBlank();

	void start(IDebugSeeder* seeder, bool pending);
	void syncNetwork();

	uint64_t getHeight();

	void suspendMining();
	void resumeMining();

	int getListeningPort() const noexcept;

	const UnicodeString* getNodeName() const noexcept;
	const NodeIdentifierSource* getNetworkKey() const noexcept;
	const NodeIdentifierSource* getVoterIdentifierSource() const noexcept;

	bool hasTransactionInMempool(const TransactionId* trxId) const;
	bool __hasTransactionInMempool(const TransactionId* trxId) const;
	bool hasTransactionInchain(const TransactionId* trxId) const;
	bool hasTransaction(const TransactionId* trxId);

	int getMempoolTransctionCount() const;

	void addIBlockGenerationListner(const IBlockGenerationListner* listner);
	void setShardExtendValidator(const AbstractShardExtentionValidator* validator);

private:
	uint16_t zone;
	int index;
	File* baseDir;
	CodablecashNetworkNodeConfig* nwconfig;
	ISystemLogger* logger;

	CodablecashNetworkNode* node;

};

} /* namespace codablecash */

#endif /* BLOCKCHAIN_UTILS_TESTNET_TESTNETINSTANCEWRAPPER_H_ */
