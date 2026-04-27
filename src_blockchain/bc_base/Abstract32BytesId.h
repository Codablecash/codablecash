/*
 * Abstract32BytesId.h
 *
 *  Created on: 2023/01/23
 *      Author: iizuka
 */

#ifndef BC_BASE_ABSTRACT32BYTESID_H_
#define BC_BASE_ABSTRACT32BYTESID_H_

#include <cstdint>

#include "filestore_block/IBlockObject.h"

namespace alinous {
class ByteBuffer;
class BigInteger;
}
using namespace alinous;

namespace codablecash {

class ISystemLogger;

class Abstract32BytesId : public IBlockObject {
protected:
	Abstract32BytesId();
public:
	static const BigInteger Q;
	static const BigInteger G;

	explicit Abstract32BytesId(const Abstract32BytesId& inst);
	Abstract32BytesId(const char* binary, int length);
	virtual ~Abstract32BytesId();

	int size() const noexcept;
	const char* toArray() const noexcept;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;

	bool equals(const Abstract32BytesId* other) const noexcept;
	int compareTo(const Abstract32BytesId* other) const noexcept;
	virtual IBlockObject* copyData() const noexcept = 0;

	BigInteger* toBigInteger() const noexcept;

	ByteBuffer* getByteBuffer() const noexcept {
		return this->id;
	}
	bool bufferIsNull() const noexcept {
		return this->id == nullptr;
	}

	UnicodeString* toString() const;

	int hashCode() const;
	class ValueCompare {
	public:
		int operator () (const Abstract32BytesId* const a, const Abstract32BytesId* const b) const;
	};

	void debugLog(ISystemLogger* logger) const;

protected:
	static ByteBuffer* makeRandom16Bytes();

protected:
	ByteBuffer* id; // 32 bytes
};

} /* namespace codablecash */

#endif /* BC_BASE_ABSTRACT32BYTESID_H_ */
