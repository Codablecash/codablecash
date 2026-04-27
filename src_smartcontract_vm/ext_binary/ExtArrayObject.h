/*
 * ExtArrayObject.h
 *
 *  Created on: 2020/02/18
 *      Author: iizuka
 */

#ifndef EXT_BINARY_EXTARRAYOBJECT_H_
#define EXT_BINARY_EXTARRAYOBJECT_H_

#include "base/ArrayList.h"
#include "ext_binary/AbstractExtObject.h"

namespace alinous {

class AbstractExtObject;
class AnalyzedType;

class ExtArrayObject : public AbstractExtObject {
public:
	ExtArrayObject(const UnicodeString* name, int length, uint8_t atype, int dim, const UnicodeString* fqn);
	virtual ~ExtArrayObject();

	void addInnerElement(AbstractExtObject* obj) noexcept;
	int getLength() const noexcept;
	AbstractExtObject* get(int i) const noexcept;

	virtual AbstractExtObject* copy() const noexcept;

	virtual const UnicodeString* toString() const noexcept;
	virtual AbstractVmInstance* toVmInstance(VirtualMachine* vm);

private:
	ArrayList<AbstractExtObject> array;
	int length;

	uint8_t atype;
	int dim;
	UnicodeString* fqn;

};

} /* namespace alinous */

#endif /* EXT_BINARY_EXTARRAYOBJECT_H_ */
