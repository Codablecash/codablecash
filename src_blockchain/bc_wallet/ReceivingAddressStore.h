/*
 * ReceivingAddress.h
 *
 *  Created on: 2023/01/23
 *      Author: iizuka
 */

#ifndef BC_WALLET_RECEIVINGADDRESSSTORE_H_
#define BC_WALLET_RECEIVINGADDRESSSTORE_H_

#include "bc_wallet/AbstractAddressStore.h"

namespace codablecash {

class BloomFilter1024;

class ReceivingAddressStore : public AbstractAddressStore {
public:
	static const constexpr wchar_t* STORE_NAME{L"ReceivingAddressStore"};
	static const constexpr wchar_t* KEY_MAX_ADDRESS{L"maxAddress"};

	explicit ReceivingAddressStore(uint16_t zone, int maxAddress, const File* baseDir);
	virtual ~ReceivingAddressStore();

	void init(const IWalletDataEncoder *encoder);

	virtual void save();
	virtual void load(const IWalletDataEncoder *encoder);

	void exportAddress2Filter(BloomFilter1024* filter);

private:
	int maxAddress;
};

} /* namespace codablecash */

#endif /* BC_WALLET_RECEIVINGADDRESSSTORE_H_ */
