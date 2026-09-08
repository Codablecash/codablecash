/*
 * AddressDescriptor.h
 *
 *  Created on: 2023/01/24
 *      Author: iizuka
 */

#ifndef BC_BASE_ADDRESSDESCRIPTOR_H_
#define BC_BASE_ADDRESSDESCRIPTOR_H_

#include "filestore_block/IBlockObject.h"

namespace alinous {
class ByteBuffer;
class UnicodeString;
}
using namespace alinous;

namespace codablecash {

class AddressDescriptor : public alinous::IBlockObject {
public:
	static const constexpr int PREFIX_LENGTH{2};
	static const constexpr int ZONE_LENGTH{3};
	static const constexpr int CHECKDIGIT_LENGTH{2};

	AddressDescriptor(const AddressDescriptor& inst);
	AddressDescriptor(const char* prefix, const char* zone, const char* body, int bodylength);
	explicit AddressDescriptor(const char* cstr);
	explicit AddressDescriptor(const UnicodeString* str);
	virtual ~AddressDescriptor();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static AddressDescriptor* createFromBinary(ByteBuffer *in);

	virtual IBlockObject* copyData() const noexcept;

	const char* toCString() const noexcept;

	int compareTo(const AddressDescriptor* other) const noexcept;

	uint16_t getZone() const noexcept;

private:
	void makeCheckDigit();
	void importCstring(const char* cstr);
private:
	char prefix[2]; // utf base
	char zone[3]; // utf base radix 16
	ByteBuffer* body;

	char checkDigit[2];// utf base radix 10
};

} /* namespace codablecash */

#endif /* BC_BASE_ADDRESSDESCRIPTOR_H_ */
