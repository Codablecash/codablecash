/*
 * PoWNonceResult.cpp
 *
 *  Created on: 2023/03/21
 *      Author: iizuka
 */

#include "pow/PoWNonceResult.h"

namespace codablecash {

PoWNonceResult::PoWNonceResult(const PoWNonceResult &inst) : PoWNonce(inst) {
}

PoWNonceResult::PoWNonceResult(const BigInteger* nonce) : PoWNonce(nonce) {

}

PoWNonceResult::~PoWNonceResult() {

}

PoWNonceResult* PoWNonceResult::nonce2Result(const PoWNonce *nonce) {
	const BigInteger* value = nonce->getValue();
	return new PoWNonceResult(value);
}

} /* namespace codablecash */
