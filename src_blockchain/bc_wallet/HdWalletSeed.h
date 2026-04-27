/*
 * HdWalletSeed.h
 *
 *  Created on: 2023/01/23
 *      Author: iizuka
 */

#ifndef BC_WALLET_HDWALLETSEED_H_
#define BC_WALLET_HDWALLETSEED_H_

#include "bc_base/Abstract32BytesId.h"

namespace alinous {
class BigInteger;
}
using namespace alinous;

namespace codablecash {

class IWalletDataEncoder;
class AddressDescriptor;
class IMuSigSigner;

class HdWalletSeed : public Abstract32BytesId {
protected:
	HdWalletSeed();
public:
	HdWalletSeed(const char* binary, int length);

	virtual ~HdWalletSeed();

	static HdWalletSeed* newSeed() noexcept ;

	virtual IBlockObject* copyData() const noexcept;
	HdWalletSeed* indexedSeed(int accountIndex) const noexcept;

	HdWalletSeed* encodedSeed(const IWalletDataEncoder* encoder) const noexcept;
};

} /* namespace codablecash */

#endif /* BC_WALLET_HDWALLETSEED_H_ */
