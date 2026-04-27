/*
 * AbstractVmReservedInstance.h
 *
 *  Created on: Nov 3, 2025
 *      Author: iizuka
 */

#ifndef INSTANCE_RESERVED_CLASSES_ABSTRACTVMRESERVEDINSTANCE_H_
#define INSTANCE_RESERVED_CLASSES_ABSTRACTVMRESERVEDINSTANCE_H_

#include "instance/VmClassInstance.h"


namespace alinous {

class AbstractVmReservedInstance : public VmClassInstance {
public:
	AbstractVmReservedInstance(AnalyzedClass* clazz, VirtualMachine* vm);
	virtual ~AbstractVmReservedInstance();
};

} /* namespace alinous */

#endif /* INSTANCE_RESERVED_CLASSES_ABSTRACTVMRESERVEDINSTANCE_H_ */
