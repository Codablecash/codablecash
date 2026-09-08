/*
 * AbstractUtxo.h
 *
 *  Created on: 2023/01/25
 *      Author: iizuka
 */

#ifndef BC_TRX_ABSTRACTUTXO_H_
#define BC_TRX_ABSTRACTUTXO_H_

#include "filestore_block/IBlockObject.h"

using namespace alinous;

namespace codablecash {

class UtxoId;
class BalanceUnit;
class AddressDescriptor;

class AbstractUtxo : public alinous::IBlockObject {
public:
	static const constexpr uint8_t TRX_UTXO_BALANCE{1};
	static const constexpr uint8_t TRX_UTXO_TICKET{2};
	static const constexpr uint8_t TRX_UTXO_VOTED_TICKET{3};

	static const constexpr uint8_t TRX_UTXO_REMOTE_BALANCE{4};

	static const constexpr uint8_t TRX_UTXO_REMOVED_DUMMY{100};


	static AbstractUtxo* createFromBinary(ByteBuffer* in);

	AbstractUtxo(const AbstractUtxo& inst);
	AbstractUtxo();
	virtual ~AbstractUtxo();

	virtual uint8_t getType() const noexcept = 0;
	virtual void fromBinary(ByteBuffer* in) = 0;
	virtual void build() = 0;

	virtual const AddressDescriptor* getAddress() const noexcept = 0;
	virtual BalanceUnit getAmount() const noexcept = 0;

	virtual bool isRemote() const noexcept {return false;}

	bool equals(const AbstractUtxo* other) const noexcept;

	const UtxoId* getId() const noexcept {
		return this->utxoId;
	}

	void setNonce(const uint8_t* data32bytes, int index);

protected:
	UtxoId* utxoId;

	uint8_t nonce[32];
};

} /* namespace codablecash */

#endif /* BC_TRX_ABSTRACTUTXO_H_ */
