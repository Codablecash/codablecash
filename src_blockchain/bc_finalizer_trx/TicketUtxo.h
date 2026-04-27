/*
 * TicketUtxo.h
 *
 *  Created on: 2023/04/27
 *      Author: iizuka
 */

#ifndef BC_FINALIZER_TRX_TICKETUTXO_H_
#define BC_FINALIZER_TRX_TICKETUTXO_H_

#include "bc_trx/AbstractUtxo.h"

#include "bc_base/BalanceUnit.h"

namespace codablecash {

class NodeIdentifier;

class TicketUtxo: public AbstractUtxo {
public:
	TicketUtxo(const TicketUtxo& inst);
	TicketUtxo();
	virtual ~TicketUtxo();

	virtual uint8_t getType() const noexcept;

	virtual void build();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;

	virtual const AddressDescriptor* getAddress() const noexcept;
	virtual BalanceUnit getAmount() const noexcept;

	void setNodeIndentifier(const NodeIdentifier* nodeId) noexcept;
	void setAddressDescriptor(const AddressDescriptor* addressDesc) noexcept;
	void setAmounst(BalanceUnit amount) noexcept;

	const NodeIdentifier* getNodeIdentifier() const noexcept {
		return this->nodeId;
	}

private:
	NodeIdentifier* nodeId; // Stake pool
	AddressDescriptor* addressDesc; // registered ticket address
	BalanceUnit amount;
};

} /* namespace codablecash */

#endif /* BC_FINALIZER_TRX_TICKETUTXO_H_ */
