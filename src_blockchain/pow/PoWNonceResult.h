/*
 * PoWNonceResult.h
 *
 *  Created on: 2023/03/21
 *      Author: iizuka
 */

#ifndef POW_POWNONCERESULT_H_
#define POW_POWNONCERESULT_H_

#include "pow/PoWNonce.h"

namespace codablecash {

class PoWNonceResult : public PoWNonce {
public:
	PoWNonceResult(const PoWNonceResult& inst);
	explicit PoWNonceResult(const BigInteger* nonce);
	virtual ~PoWNonceResult();

	static PoWNonceResult* nonce2Result(const PoWNonce* nonce);
};

} /* namespace codablecash */

#endif /* POW_POWNONCERESULT_H_ */
