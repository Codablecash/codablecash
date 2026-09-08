/*
 * HdWallet.h
 *
 *  Created on: 2023/01/23
 *      Author: iizuka
 */

#ifndef BC_WALLET_HDWALLET_H_
#define BC_WALLET_HDWALLET_H_
#include <cstdint>

#include "base/ArrayList.h"

namespace alinous {
class File;
}
using namespace alinous;

namespace codablecash {

class HdWalletSeed;
class WalletAccount;
class IWalletDataEncoder;
class StatusStore;
class BloomFilter1024;

class HdWallet {
public:
	static const constexpr wchar_t* STORE_NAME{L"HdWallet"};
	static const constexpr wchar_t* KEY_ENCODED_SEED{L"encodedSeed"};
	static const constexpr wchar_t* KEY_NUM_ACCOUNTS{L"numAccounts"};
	static const constexpr wchar_t* KEY_DEFAULT_ZONE{L"defaultZone"};

	HdWallet(const File* baseDir);
	virtual ~HdWallet();

	static HdWallet* create(const File* dir, const HdWalletSeed* seed, uint16_t defaultZone, const IWalletDataEncoder* encoder, int defaultMaxAddress);
	static HdWallet* loadWallet(const File* dir, const IWalletDataEncoder* encoder);

	void setSeed(const HdWalletSeed *seed, const IWalletDataEncoder* encoder);
	HdWalletSeed* getRootSeed(const IWalletDataEncoder* encoder) const;

	void newAccount(const HdWalletSeed *rootSeed, uint16_t zone, const IWalletDataEncoder* encoder, int maxAddress);

	WalletAccount* getAccount(int i) const noexcept {
		return this->accounts->get(i);
	}
	WalletAccount* getZoneAccount(uint16_t zone) const noexcept;


	void setDefaultZone(uint16_t defaultZone) noexcept {
		this->defaultZone = defaultZone;
	}
	uint16_t getDefaultZone() const noexcept {
		return this->defaultZone;
	}

	void save();
	void load(const IWalletDataEncoder* encoder);
	void close();

	ArrayList<BloomFilter1024>* getBloomFilters(const IWalletDataEncoder* encoder) const noexcept;


private:
	File* baseDir;
	ArrayList<WalletAccount> *accounts;

	HdWalletSeed* encodedSeed;

	uint16_t defaultZone;

	StatusStore* store;
};

} /* namespace codablecash */

#endif /* BC_WALLET_HDWALLET_H_ */
