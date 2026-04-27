/*
 * AbstractAddress.h
 *
 *  Created on: 2023/01/23
 *      Author: iizuka
 */

#ifndef BC_BASE_ABSTRACTADDRESS_H_
#define BC_BASE_ABSTRACTADDRESS_H_

#include "filestore_block/IBlockObject.h"

namespace alinous {
class UnicodeString;
class ByteBuffer;
}
using namespace alinous;

#include <cstdint>

namespace codablecash {

class AddressDescriptor;

class AbstractAddress : public alinous::IBlockObject {
public:
	static const constexpr uint8_t ADDRESS_TYPE_BALANCE{1};
	static const constexpr uint8_t ADDRESS_TYPE_SMARTCONTRACT_MODULE{2};
	static const constexpr uint8_t ADDRESS_TYPE_SMARTCONTRACT_INSTANCE{3};

	static AbstractAddress* createFromBinary(ByteBuffer* in);

	explicit AbstractAddress(uint16_t zone);
	virtual ~AbstractAddress();

	virtual uint8_t getType() const noexcept = 0;

	virtual void fromBinary(ByteBuffer* in) = 0;

	virtual const char* getPrefix() const noexcept = 0;
	virtual ByteBuffer* getBodyPart() const noexcept = 0;
	AddressDescriptor* toAddressDescriptor() const noexcept;

	UnicodeString* toString() const noexcept;

protected:
	uint16_t zone;
};

} /* namespace codablecash */

#endif /* BC_BASE_ABSTRACTADDRESS_H_ */
