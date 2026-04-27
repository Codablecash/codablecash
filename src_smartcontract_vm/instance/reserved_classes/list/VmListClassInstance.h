/*
 * VmListClassInstance.h
 *
 *  Created on: Nov 6, 2025
 *      Author: iizuka
 */

#ifndef INSTANCE_RESERVED_CLASSES_LIST_VMLISTCLASSINSTANCE_H_
#define INSTANCE_RESERVED_CLASSES_LIST_VMLISTCLASSINSTANCE_H_

#include "instance/reserved_classes/AbstractVmReservedInstance.h"

namespace alinous {

class VmListClassInstance : public AbstractVmReservedInstance {
public:
	VmListClassInstance(AnalyzedClass* clazz, VirtualMachine* v);
	virtual ~VmListClassInstance();
};

} /* namespace alinous */

#endif /* INSTANCE_RESERVED_CLASSES_LIST_VMLISTCLASSINSTANCE_H_ */
