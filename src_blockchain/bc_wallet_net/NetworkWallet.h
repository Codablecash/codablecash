/*
 * NetworkWallet.h
 *
 *  Created on: Feb 17, 2025
 *      Author: iizuka
 */

#ifndef BC_WALLET_NET_NETWORKWALLET_H_
#define BC_WALLET_NET_NETWORKWALLET_H_
#include <cstdint>

namespace alinous {
class File;
class UnicodeString;
}
using namespace alinous;


namespace codablecash {

class HdWallet;
class WalletNetworkManager;
class IWalletDataEncoder;
class HdWalletSeed;
class INetworkSeeder;
class AddressDescriptor;
class ISystemLogger;
class StakingSeedManager;
class NetworkClientCommandProcessor;
class NetworkWalletData;
class CodablecashSystemParam;
class WalletConfig;
class NodeIdentifierSource;
class NetworkTransactionHandler;


class NetworkWallet {
public:
	NetworkWallet(const File* baseDir, ISystemLogger* logger, const CodablecashSystemParam *config, const WalletConfig* walletConfig);
	virtual ~NetworkWallet();

	void init();
	void resetComamndQueue();

	void closeData() noexcept;

	static NetworkWallet* createNewWallet(const File* dir, const UnicodeString* pass, uint16_t zone, int defaultMaxAddress, ISystemLogger* logger
			, const CodablecashSystemParam *config, const WalletConfig* walletConfig);
	static NetworkWallet* resotreWallet(const File* dir, const UnicodeString* pass, uint16_t zone, const HdWalletSeed* rootSeed, int defaultMaxAddress, ISystemLogger* logger
			, const CodablecashSystemParam *config, const WalletConfig* walletConfig);
	static NetworkWallet* openWallet(const File* dir, const UnicodeString* pass, ISystemLogger* logger
			, const CodablecashSystemParam *config, const WalletConfig* walletConfig);

	void createData();
	void openData();

	HdWalletSeed* getRootSeed(const IWalletDataEncoder* encoder) const;

	void setStakingSourceId(const NodeIdentifierSource* source, const IWalletDataEncoder* encoder);
	NodeIdentifierSource* getStakingSourceId(const IWalletDataEncoder* encoder) const noexcept;

	void initNetwork(INetworkSeeder *seeder, const IWalletDataEncoder* encoder);
	void syncBlockchain(bool resume = false);
	void resumeNetwork();

	void shutdownNetwork();

	AddressDescriptor* getAddressDescriptor(int accountIndex, int addressIndex) const;
	const File* getBaseDir() const noexcept {
		return this->baseDir;
	}
	NetworkWalletData* getWalletData() const noexcept {
		return this->walletData;
	}

	// transaction handling
	NetworkTransactionHandler* getNetworkTransactionHandler(int accountIndex) noexcept;

	WalletNetworkManager* getWalletNetworkManager() const noexcept {
		return this->networkManager;
	}

	const WalletConfig* getWalletConfig() const noexcept {
		return this->walletConfig;
	}
	ISystemLogger* getLogger() const noexcept {
		return this->logger;
	}

	NetworkClientCommandProcessor* getNetworkClientCommandProcessor() const noexcept {
		return this->clientCommandProcessor;
	}

private:
	void setNetworkSeeder(INetworkSeeder *seeder) noexcept;
	void doCreateWallet(const IWalletDataEncoder* encoder, const HdWalletSeed* seed, uint16_t zone, int defaultMaxAddress);
	void doOpeneWallet(const IWalletDataEncoder *encoder);

private:
	File* baseDir;
	ISystemLogger* logger;
	CodablecashSystemParam *config;
	WalletConfig* walletConfig;

	// wallet
	NetworkWalletData* walletData;

	// command processor
	NetworkClientCommandProcessor* clientCommandProcessor;

	// net
	WalletNetworkManager* networkManager;

	// Staking Seed Manager
	StakingSeedManager* stakingSeedManager;
};

} /* namespace codablecash */

#endif /* BC_WALLET_NET_NETWORKWALLET_H_ */
