/*
 * AbstractAddressStore.h
 *
 *  Created on: 2023/01/23
 *      Author: iizuka
 */

#ifndef BC_WALLET_ABSTRACTADDRESSSTORE_H_
#define BC_WALLET_ABSTRACTADDRESSSTORE_H_
#include <cstdint>

#include "base/ArrayList.h"

namespace alinous {
class File;
}
using namespace alinous;

namespace codablecash {

class HdWalletSeed;
class AddressAndPrivateKey;
class IWalletDataEncoder;
class AddressDescriptor;
class IMuSigSigner;
class BalanceAddress;
class StatusStore;
class WalletAccountTrxRepository;
class BalanceUtxo;
class BalanceUnit;

class AbstractAddressStore {
public:
	static const constexpr wchar_t* KEY_ZONE{L"zone"};
	static const constexpr wchar_t* KEY_ENCRYPTED_SEED{L"encryptedSeed"};
	static const constexpr wchar_t* KEY_ADDRESS_SERIAL{L"adressSerial"};

	explicit AbstractAddressStore(uint16_t zone, const File* baseDir, const wchar_t* storeName);
	virtual ~AbstractAddressStore();

	void setEncryptedSeed(HdWalletSeed* encrypted) noexcept;

	AddressDescriptor* getAddressDescriptor(int i) const noexcept;
	const BalanceAddress* getAddress(int i) const noexcept;
	int size() const noexcept {
		return this->list->size();
	}

	IMuSigSigner* getSigner(const AddressDescriptor* desc, const IWalletDataEncoder* encoder);

	virtual void save() = 0;
	virtual void load(const IWalletDataEncoder *encoder) = 0;

	const BalanceAddress* getAddress(const AddressDescriptor *dest) const noexcept;
	bool hasAddress(const AddressDescriptor *dest) const noexcept;

protected:
	void __save();
	void __load();

	AddressAndPrivateKey* createNewAddressAndPrivateKey(const IWalletDataEncoder* encoder, uint64_t serial);

protected:
	uint16_t zone;
	HdWalletSeed* encryptedSeed;
	uint64_t addressSerial;

	ArrayList<AddressAndPrivateKey>* list;
	StatusStore* store;
};

} /* namespace codablecash */

#endif /* BC_WALLET_ABSTRACTADDRESSSTORE_H_ */
