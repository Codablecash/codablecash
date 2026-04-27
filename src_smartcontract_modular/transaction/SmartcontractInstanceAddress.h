/*
 * SmartcontractInstanceAddress.h
 *
 *  Created on: Oct 2, 2025
 *      Author: iizuka
 */

#ifndef TRANSACTION_SMARTCONTRACTINSTANCEADDRESS_H_
#define TRANSACTION_SMARTCONTRACTINSTANCEADDRESS_H_

#include "transaction/AbstractSmartcontractAddress.h"

namespace codablecash {

class ScPublicKey;

class SmartcontractInstanceAddress : public AbstractSmartcontractAddress {
public:
	static const char* PREFIX;

	SmartcontractInstanceAddress();
	SmartcontractInstanceAddress(uint16_t zone, Secp256k1CompressedPoint* pubkey);
	virtual ~SmartcontractInstanceAddress();

	virtual uint8_t getType() const noexcept {
		return AbstractAddress::ADDRESS_TYPE_SMARTCONTRACT_INSTANCE;
	}

	virtual const char* getPrefix() const noexcept;

	virtual IBlockObject* copyData() const noexcept;

	static SmartcontractInstanceAddress* createAddress(uint16_t zone);
	static SmartcontractInstanceAddress* createAddress(uint16_t zone, const ScPublicKey* publicKey);
};

} /* namespace codablecash */

#endif /* TRANSACTION_SMARTCONTRACTINSTANCEADDRESS_H_ */
