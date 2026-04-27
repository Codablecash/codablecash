/*
 * SmartcontractAddress.h
 *
 *  Created on: Oct 2, 2025
 *      Author: iizuka
 */

#ifndef TRANSACTION_ABSTRACTSMARTCONTRACTADDRESS_H_
#define TRANSACTION_ABSTRACTSMARTCONTRACTADDRESS_H_

#include "bc_base/AbstractAddress.h"

namespace codablecash {

class Secp256k1CompressedPoint;

class AbstractSmartcontractAddress : public AbstractAddress {
public:
	AbstractSmartcontractAddress();
	AbstractSmartcontractAddress(uint16_t zone, Secp256k1CompressedPoint* pubkey);
	virtual ~AbstractSmartcontractAddress();

	virtual ByteBuffer* getBodyPart() const noexcept;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	int compareTo(const AbstractSmartcontractAddress* other) const noexcept;

protected:
	Secp256k1CompressedPoint* pubkey;
};

} /* namespace codablecash */

#endif /* TRANSACTION_ABSTRACTSMARTCONTRACTADDRESS_H_ */

