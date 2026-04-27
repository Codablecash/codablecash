/*
 * SoftwareVersionKey.h
 *
 *  Created on: Jan 31, 2026
 *      Author: iizuka
 */

#ifndef BC_SOFTWAREVERSIONKEY_H_
#define BC_SOFTWAREVERSIONKEY_H_

#include "btree/AbstractBtreeKey.h"

using namespace alinous;

namespace codablecash {

class SoftwareVersion;

class SmartcontractVersionKey : public AbstractBtreeKey {
public:
	explicit SmartcontractVersionKey(const SoftwareVersion* value);
	virtual ~SmartcontractVersionKey();

	virtual bool isInfinity() const { return false; }
	virtual bool isNull() const { return false; }

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static SmartcontractVersionKey* fromBinary(ByteBuffer* in);

	virtual int compareTo(const AbstractBtreeKey* key) const noexcept;
	virtual AbstractBtreeKey* clone() const noexcept;

protected:
	SoftwareVersion* value;
};

} /* namespace codablecash */

#endif /* BC_SOFTWAREVERSIONKEY_H_ */
