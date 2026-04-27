/*
 * VmObjectClassInstance.h
 *
 *  Created on: Nov 2, 2025
 *      Author: iizuka
 */

#ifndef INSTANCE_RESERVED_CLASSES_OBJECT_VMOBJECTCLASSINSTANCE_H_
#define INSTANCE_RESERVED_CLASSES_OBJECT_VMOBJECTCLASSINSTANCE_H_

#include "instance/reserved_classes/AbstractVmReservedInstance.h"

namespace alinous {

class AnalyzedType;
class IAbstractVmInstanceSubstance;

class VmObjectClassInstance : public AbstractVmReservedInstance {
public:
	VmObjectClassInstance(AnalyzedClass* clazz, VirtualMachine* v);
	virtual ~VmObjectClassInstance();


};

} /* namespace alinous */

#endif /* INSTANCE_RESERVED_CLASSES_OBJECT_VMOBJECTCLASSINSTANCE_H_ */
