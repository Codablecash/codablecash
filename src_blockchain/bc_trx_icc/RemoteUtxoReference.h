/*
 * RemoteUtxoReference.h
 *
 *  Created on: Aug 8, 2026
 *      Author: iizuka
 */

#ifndef BC_TRX_ICC_REMOTEUTXOREFERENCE_H_
#define BC_TRX_ICC_REMOTEUTXOREFERENCE_H_

#include "bc_trx/AbstractUtxoReference.h"

namespace codablecash {

class RemoteUtxoReference: public AbstractUtxoReference {
public:
	RemoteUtxoReference();
	virtual ~RemoteUtxoReference();

	virtual uint8_t getType() const noexcept {
		return AbstractUtxoReference::UTXO_REF_TYPE_REMOTE;
	}

private:

};

} /* namespace codablecash */

#endif /* BC_TRX_ICC_REMOTEUTXOREFERENCE_H_ */
