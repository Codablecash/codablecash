/*
 * WalletAccount.h
 *
 *  Created on: 2023/01/23
 *      Author: iizuka
 */

#ifndef BC_WALLET_WALLETACCOUNT_H_
#define BC_WALLET_WALLETACCOUNT_H_

#include <cstdint>

#include "bc_wallet/AbstractWalletAccount.h"

#include "base/ArrayList.h"

#include "bc_trx/IAddressChecker.h"

namespace alinous {
class File;
}
using namespace alinous;

namespace codablecash {

class ReceivingAddressStore;
class ChangeAddressStore;
class HdWalletSeed;
class IWalletDataEncoder;
class AddressDescriptor;
class IMuSigSigner;
class BalanceAddress;
class WalletAccountTrxRepository;
class BalanceUnit;
class GenesisTransaction;
class BalanceTransferTransaction;
class AbstractBlockchainTransaction;
class AbstractAddressStore;
class StatusStore;
class TransactionId;
class RegisterVotePoolTransaction;
class RegisterTicketTransaction;
class NodeIdentifier;
class NodeIdentifierSource;
class BloomFilter1024;
class ITransactionBuilderContext;

class WalletAccount : public AbstractWalletAccount, public IAddressChecker {
public:
	static const constexpr wchar_t* STORE_NAME{L"WalletAccount"};
	static const constexpr wchar_t* KEY_ENCRYPTED_SEED{L"encryptedSeed"};


	explicit WalletAccount(const File* accountBaseDir);
	virtual ~WalletAccount();

	static WalletAccount* newAccount(const File* baseDir, const HdWalletSeed *rootSeed, int accountIndex, uint16_t zone, const IWalletDataEncoder* encoder, int maxAddress);
	static WalletAccount* loadAccount(const File* baseDir, int accountIndex, const IWalletDataEncoder* encoder);

	void setEncryptedSeed(HdWalletSeed* encrypted) noexcept;

	void initAddressStores(const IWalletDataEncoder* encoder, int maxAddress);
	void initTransactionRepository();

	AddressDescriptor* getReceivingAddressDescriptor(int i) const noexcept;
//	AddressDescriptor* getChangeAddressDescriptor(int i) const noexcept;

//	const BalanceAddress* getReceivingAddress(int i) const noexcept;
//	const BalanceAddress* getChangeAddress(int i) const noexcept;

	const BalanceAddress* getAddress(const AddressDescriptor* desc) const noexcept;

	IMuSigSigner* getSigner(const AddressDescriptor* desc, const IWalletDataEncoder* encoder);

	const File* getAccountBaseDir() const noexcept {
		return this->accountBaseDir;
	}

	GenesisTransaction* createGenesisTransaction(const BalanceUnit amount, int pos);
	BalanceTransferTransaction* createBalanceTransferTransaction(const AddressDescriptor* dest, const BalanceUnit& amount
			, const BalanceUnit& feeRate, bool feeIncluded, const IWalletDataEncoder* encoder);
	BalanceTransferTransaction* createBalanceTransferTransaction(const AddressDescriptor* dest, const BalanceUnit& amount
			, const BalanceUnit& feeRate, bool feeIncluded, const IWalletDataEncoder* encoder, ITransactionBuilderContext* context);

	RegisterVotePoolTransaction* createRegisterVotePoolTransaction(const NodeIdentifierSource *source
			, const BalanceUnit& feeRate, const AddressDescriptor* addressDesc, const IWalletDataEncoder* encoder);
	RegisterVotePoolTransaction* createRegisterVotePoolTransaction(const NodeIdentifierSource *source
			, const BalanceUnit& feeRate, const AddressDescriptor* addressDesc, const IWalletDataEncoder* encoder, ITransactionBuilderContext* context);

	RegisterTicketTransaction* createRegisterTicketTransaction(const NodeIdentifier* nodeId, const BalanceUnit& stakeAmount
			, const BalanceUnit& feeRate, const AddressDescriptor *addressDesc, const IWalletDataEncoder* encoder);
	RegisterTicketTransaction* createRegisterTicketTransaction(const NodeIdentifier* nodeId, const BalanceUnit& stakeAmount
			, const BalanceUnit& feeRate, const AddressDescriptor *addressDesc, const IWalletDataEncoder* encoder, ITransactionBuilderContext* context);

	void importTransaction(const AbstractBlockchainTransaction* trx);
	AbstractBlockchainTransaction* findTransaction(const TransactionId* trxId);
	void removeTransaction(const TransactionId* trxId);

	BalanceUnit getTotalAmount() const noexcept;

	void save();
	void load(const IWalletDataEncoder* encoder);

	ReceivingAddressStore* getReceivingAddresses() const noexcept {
		return this->receivingAddresses;
	}
	ChangeAddressStore* getChangeAddresses() const noexcept {
		return this->changeAddresses;
	}
	WalletAccountTrxRepository* getWalletAccountTrxRepository() const noexcept {
		return this->trxRepo;
	}

	bool hasAddress(const AddressDescriptor* addressDesc);

	ArrayList<AbstractBlockchainTransaction>* getTransactions() const;

	const BloomFilter1024* getBloomFilter(const IWalletDataEncoder* encoder);

	virtual bool checkAddress(const AddressDescriptor* desc) const;

private:
	void createBloomFilter(const IWalletDataEncoder* encoder);

	int findAddressDiscriptor(const AbstractAddressStore* store, const AddressDescriptor* addressDesc);

	void initReceivingAddressStore(const HdWalletSeed* rootAccountSeed, int maxAddressCount, const IWalletDataEncoder* encoder);
	void initChangeAddressStore(const HdWalletSeed* rootAccountSeed, const IWalletDataEncoder* encoder, int numAddressInThisGroup);

private:
	File* accountBaseDir;

	ReceivingAddressStore* receivingAddresses;
	ChangeAddressStore* changeAddresses;

	HdWalletSeed* encryptedSeed;

	WalletAccountTrxRepository* trxRepo;

	StatusStore* store;

	BloomFilter1024* bloomFilter;
};

} /* namespace codablecash */

#endif /* BC_WALLET_WALLETACCOUNT_H_ */
