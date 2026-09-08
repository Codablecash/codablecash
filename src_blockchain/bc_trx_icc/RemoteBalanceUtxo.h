/*
 * RemoteUtxo.h
 *
 *  Created on: Aug 7, 2026
 *      Author: iizuka
 */

#ifndef BC_TRX_ICC_REMOTEBALANCEUTXO_H_
#define BC_TRX_ICC_REMOTEBALANCEUTXO_H_

#include "bc_trx_balance/BalanceUtxo.h"


namespace codablecash {

class RemoteBalanceUtxo: public BalanceUtxo {
public:
	RemoteBalanceUtxo(const RemoteBalanceUtxo& inst);
	RemoteBalanceUtxo();
	virtual ~RemoteBalanceUtxo();

	virtual uint8_t getType() const noexcept;
	virtual bool isRemote() const noexcept;

	virtual IBlockObject* copyData() const noexcept;
};

} /* namespace codablecash */

#endif /* BC_TRX_ICC_REMOTEBALANCEUTXO_H_ */
