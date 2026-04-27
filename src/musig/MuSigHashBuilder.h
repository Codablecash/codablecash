/*
 * MuSigHashBuilder.h
 *
 *  Created on: 2023/02/02
 *      Author: iizuka
 */

#ifndef MUSIG_MUSIGHASHBUILDER_H_
#define MUSIG_MUSIGHASHBUILDER_H_

#include "base/ArrayList.h"

#include <cstdint>

namespace alinous {
class ByteBuffer;
class BigInteger;
}
using namespace alinous;

namespace codablecash {

class Secp256k1Point;

class MuSigHashBuilder {
public:
	MuSigHashBuilder();
	virtual ~MuSigHashBuilder();

	void addBigInteger(const BigInteger* bi);
	void add(const Secp256k1Point* pt);
	void addArray(const char *data, int length);

	void buildHash();

	BigInteger getResultAsBigInteger() const noexcept;

private:
	int binarySize() const noexcept;

private:
	ArrayList<ByteBuffer>* list;
	uint8_t sha256[32];
};

} /* namespace codablecash */

#endif /* MUSIG_MUSIGHASHBUILDER_H_ */
