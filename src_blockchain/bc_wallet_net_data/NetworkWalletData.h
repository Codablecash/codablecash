/*
 * NetworkCommandData.h
 *
 *  Created on: Mar 23, 2025
 *      Author: iizuka
 */

#ifndef BC_WALLET_NET_NETWORKWALLETDATA_H_
#define BC_WALLET_NET_NETWORKWALLETDATA_H_

#include <cstdint>

#include "base/ArrayList.h"

#include "bc_status_cache/IBlockchainStoreProvider.h"


namespace alinous {
class File;
class UnicodeString;
class SysMutex;
class ConcurrentGate;
}
using namespace alinous;

namespace codablecash {

class ISystemLogger;
class HdWallet;
class HdWalletSeed;
class IWalletDataEncoder;
class HeadBlockDetector;
class BlockHeaderStoreManager;
class TransactionTransferData;
class TransactionGroupDataStore;
class WalletMemoryPool;
class BlockHeader;
class AbstractBlockchainTransaction;
class StatusStore;
class CodablecashSystemParam;
class ManagementAccountsCollection;
class ManagementAccount;
class TransactionId;
class BlockHeaderId;
class HeaderTransactionGroup;
class WalletAccount;


class NetworkWalletData : public IBlockchainStoreProvider {
public:
	static const constexpr wchar_t* KEY_ZONE{L"zone"};
	static const constexpr wchar_t* KEY_FINALIZED_HEIGHT{L"finalizedHeight"};
	static const constexpr wchar_t* STATUS_STORE_FILE_NAME{L"status.bin"};

	NetworkWalletData(const File* baseDir, ISystemLogger* logger, const CodablecashSystemParam *config);
	virtual ~NetworkWalletData();

	void close() noexcept;

	void openHdWallet(const IWalletDataEncoder *encoder);
	void createHdWallet(const HdWalletSeed* seed, uint16_t defaultZone, const IWalletDataEncoder* encoder, int defaultMaxAddress);

	void openData();
	void createBlankData();

	HdWallet* getHdWallet() const noexcept {
		return this->hdWallet;
	}

	uint16_t getDefaultZone() const noexcept;

	void addTransactionDataToMempool(const TransactionTransferData *data);
	void __addTransactionDataToMempool(const AbstractBlockchainTransaction* trx);
	uint8_t getTransactionStoreStatus(const TransactionId* trxId) const noexcept;

	void addHeader(const BlockHeader* header, const ArrayList<AbstractBlockchainTransaction>* trxlist);
	void updateHeadDetection();
	bool checkAndFinalizing();

	const File* getBaseDir() const noexcept {
		return this->baseDir;
	}

	void __saveStatus();
	void __loadStatus();

	virtual BlockHeaderStoreManager* getHeaderManager(uint16_t zone) const noexcept;
	virtual BlockBodyStoreManager* getBlockBodyStoreManager(uint16_t zone) const noexcept;

	void resetManagementAccounts() noexcept;
	void buildManagementAccount(bool buildFinalized, uint64_t startHeight);

	void __buildMempoolAccount();
	void __buildUnfinalizedAccount();

	ConcurrentGate* getLock() const noexcept {
		return this->gateLock;
	}

	ManagementAccountsCollection* getManagementAccountsCollection() const noexcept {
		return this->managementAccounts;
	}

	uint64_t getFinalizedHeight() const;


private:
	void __buildFinalizedManagementAccount(uint64_t startHeight);

	void __buildFinalizedManagementAccount(ManagementAccount* ma, uint64_t startHeight, uint64_t maxBlockHeight);
	void __buildManagementAccount4Header(ManagementAccount* ma, const BlockHeader* header);


	void __initStatusStore();
	void __doFinalize(const BlockHeader *header);
	void __updateFinalizedData(uint64_t finalizingHeight, const BlockHeaderId *finalizingHeaderId);
	void __importIntoHdWallet(const ArrayList<BlockHeader>* list);
	void __importImportHeaderTransactionGroup(const HeaderTransactionGroup* trxGourp, WalletAccount* waccount);

	void __finalizeHeaderStore(uint64_t height,	const BlockHeaderId *headerId);

private:
	File* baseDir;
	ISystemLogger* logger;
	const CodablecashSystemParam *config;

	// wallet (Finalized Data)
	HdWallet* hdWallet;
	uint64_t finalizedHeight;
	uint16_t zone;

	// unfinalized chain
	TransactionGroupDataStore* transactionGroupData;

	BlockHeaderStoreManager* headerManager;
	HeadBlockDetector* detector;

	// memory pool
	WalletMemoryPool* mempool;

	// store
	StatusStore* statusStore;

	// management account
	ManagementAccountsCollection* managementAccounts;

	//
	ConcurrentGate* gateLock;
};

} /* namespace codablecash */

#endif /* BC_WALLET_NET_NETWORKWALLETDATA_H_ */
