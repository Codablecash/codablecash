/*
 * WalletNetworkManager.h
 *
 *  Created on: Feb 17, 2025
 *      Author: iizuka
 */

#ifndef BC_WALLET_NET_WALLETNETWORKMANAGER_H_
#define BC_WALLET_NET_WALLETNETWORKMANAGER_H_
#include <cstdint>

#include "base/ArrayList.h"

namespace alinous {
class UnicodeString;
}
using namespace alinous;


namespace codablecash {

class INetworkSeeder;
class ISystemLogger;
class P2pNodeRecord;
class WalletNetworkNodeCandidates;
class WalletConnectionManager;
class NetworkClientCommandProcessor;
class BloomFilter512;

class WalletNetworkManager {
public:
	static const constexpr int MAX_CONNECTIONS{8};

	explicit WalletNetworkManager(uint16_t defaultZone, ISystemLogger* logger, NetworkClientCommandProcessor* clientCommandProcessor);
	virtual ~WalletNetworkManager();

	void setSeeder(INetworkSeeder* seeder) noexcept;

	void maintainNetwork(const ArrayList<BloomFilter512>* filters);

private:
	void importSeeds(int numZones, const P2pNodeRecord* seedRec);
	void doInitNodeCandidates(uint16_t zone, const UnicodeString *host, int port, int protoco);
	int selectRandom(int curSize) const noexcept;

private:
	INetworkSeeder* seeder;
	ISystemLogger* logger;

	WalletNetworkNodeCandidates* nodesCandidates;

	int maxNumConnection;
	WalletConnectionManager* connectionManager;
};

} /* namespace codablecash */

#endif /* BC_WALLET_NET_WALLETNETWORKMANAGER_H_ */
