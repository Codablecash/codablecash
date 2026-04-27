/*
 * VmNullprtInstance.h
 *
 *  Created on: Jan 8, 2026
 *      Author: iizuka
 */

#ifndef INSTANCE_VMNULLPRTINSTANCE_H_
#define INSTANCE_VMNULLPRTINSTANCE_H_

#include "instance/AbstractVmInstance.h"
#include "instance/IAbstractVmInstanceSubstance.h"

namespace alinous {

class VmNullprtInstance : public AbstractVmInstance, public IAbstractVmInstanceSubstance {
public:
	static const UnicodeString NULL_STR;

	VmNullprtInstance(uint64_t serial, uint8_t atype);
	virtual ~VmNullprtInstance();

	virtual AbstractReference* wrap(IAbstractVmInstanceSubstance* owner, VirtualMachine* vm);
	virtual uint8_t getInstType() const noexcept;

	virtual AnalyzedType getRuntimeType() const noexcept;

	virtual const VMemList<AbstractReference>* getInstReferences() const noexcept;
	virtual int instHashCode() const noexcept;
	virtual bool instIsPrimitive() const noexcept;
	virtual bool instIsNull() const noexcept;

	virtual int instValueCompare(const IAbstractVmInstanceSubstance* right) const noexcept;
	virtual AbstractExtObject* instToClassExtObject(const UnicodeString* name, VTableRegistory* table);

	virtual IAbstractVmInstanceSubstance* getInstance() noexcept;
	virtual int valueCompare(const IAbstractVmInstanceSubstance* right) const noexcept;
	virtual const UnicodeString* toString() const noexcept;

private:
	uint8_t atype;
};

} /* namespace alinous */

#endif /* INSTANCE_VMNULLPRTINSTANCE_H_ */
