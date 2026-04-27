/*
 * ExtNullPtrObject.h
 *
 *  Created on: 2020/03/28
 *      Author: iizuka
 */

#ifndef EXT_BINARY_EXTNULLPTROBJECT_H_
#define EXT_BINARY_EXTNULLPTROBJECT_H_

#include "ext_binary/AbstractExtObject.h"

namespace alinous {

class ExtNullPtrObject : public AbstractExtObject {
public:
	ExtNullPtrObject(const UnicodeString* name, uint8_t atype);
	virtual ~ExtNullPtrObject();

	virtual bool isNull() const noexcept;
	virtual AbstractExtObject* copy() const noexcept;

	virtual const UnicodeString* toString() const noexcept;

	virtual AbstractVmInstance* toVmInstance(VirtualMachine* vm);

private:
	uint8_t atype;
};

} /* namespace alinous */

#endif /* EXT_BINARY_EXTNULLPTROBJECT_H_ */
