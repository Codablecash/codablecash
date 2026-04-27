/*
 * TicketVotedUtxo.h
 *
 *  Created on: 2023/05/15
 *      Author: iizuka
 */

#ifndef BC_FINALIZER_TRX_TICKETVOTEDUTXO_H_
#define BC_FINALIZER_TRX_TICKETVOTEDUTXO_H_

#include "bc_trx/AbstractUtxo.h"

#include "bc_base/BalanceUnit.h"

namespace codablecash {

class TicketVotedUtxo: public AbstractUtxo {
public:
	TicketVotedUtxo(const TicketVotedUtxo& inst);
	TicketVotedUtxo();
	virtual ~TicketVotedUtxo();

	virtual uint8_t getType() const noexcept;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;

	virtual void build();
	virtual const AddressDescriptor* getAddress() const noexcept;

	void setAddress(const AddressDescriptor* desc);

	void setVotedUtxoId(const UtxoId* votedUtxoId) noexcept;
	void setAmount(BalanceUnit amount) noexcept;
	virtual BalanceUnit getAmount() const noexcept;

	const UtxoId* getVotedUtxoId() const noexcept {
		return this->votedUtxoId;
	}
private:
	AddressDescriptor* address;
	UtxoId* votedUtxoId;
	BalanceUnit amount;
};

} /* namespace codablecash */

#endif /* BC_FINALIZER_TRX_TICKETVOTEDUTXO_H_ */
