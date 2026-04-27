/*
 * Secp256k1CompressedPoint.h
 *
 *  Created on: 2023/01/31
 *      Author: iizuka
 */

#ifndef ECDA_SECP256K1COMPRESSEDPOINT_H_
#define ECDA_SECP256K1COMPRESSEDPOINT_H_

#include "numeric/BigInteger.h"

#include "filestore_block/IBlockObject.h"
using namespace alinous;

namespace codablecash {

class Secp256k1Point;

class Secp256k1CompressedPoint : public IBlockObject {
public:
	static const constexpr uint8_t COMPRESS_Y_EVEN{02};
	static const constexpr uint8_t COMPRESS_Y_ODD{03};

	Secp256k1CompressedPoint &operator=(const Secp256k1CompressedPoint &inst);

	Secp256k1CompressedPoint(const Secp256k1CompressedPoint& inst);
	Secp256k1CompressedPoint(const BigInteger x, uint8_t prefix);
	virtual ~Secp256k1CompressedPoint();

	static Secp256k1CompressedPoint compress(const Secp256k1Point* pt);
	Secp256k1Point decompress() const;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static Secp256k1CompressedPoint* fromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;

	int compareTo(const Secp256k1CompressedPoint* other);

private:
	uint8_t prefix;
	BigInteger x;
};

} /* namespace codablecash */

#endif /* ECDA_SECP256K1COMPRESSEDPOINT_H_ */
