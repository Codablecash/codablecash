/*
 * BalanceUtxo.h
 *
 *  Created on: 2023/02/20
 *      Author: iizuka
 */

#ifndef BC_TRX_BALANCE_BALANCEUTXO_H_
#define BC_TRX_BALANCE_BALANCEUTXO_H_

#include "bc_trx/AbstractUtxo.h"

#include "bc_base/BalanceUnit.h"

namespace codablecash {

class AddressDescriptor;

class BalanceUtxo : public AbstractUtxo {
public:
	BalanceUtxo(const BalanceUtxo& inst);
	BalanceUtxo();
	explicit BalanceUtxo(BalanceUnit amount);
	virtual ~BalanceUtxo();

	virtual uint8_t getType() const noexcept;

	virtual const AddressDescriptor* getAddress() const noexcept {
		return this->addressDesc;
	}

	void setAddress(const AddressDescriptor* desc) noexcept;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;

	virtual void build();
	virtual BalanceUnit getAmount() const noexcept {
		return this->amount;
	}
	void setAmount(const BalanceUnit amount) noexcept;

	void discountFee(const BalanceUnit& feeRemain);

protected:
	AddressDescriptor* addressDesc;
	BalanceUnit amount;
};

} /* namespace codablecash */

#endif /* BC_TRX_BALANCE_BALANCEUTXO_H_ */
