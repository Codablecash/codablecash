/*
 * ModuleProxyInstance.h
 *
 *  Created on: Dec 3, 2025
 *      Author: iizuka
 */

#ifndef INTER_MODULAR_ACCESS_MODULEPROXYINSTANCE_H_
#define INTER_MODULAR_ACCESS_MODULEPROXYINSTANCE_H_

#include "instance/reserved_classes/AbstractVmReservedInstance.h"

using namespace alinous;

namespace codablecash {

class ModuleProxyInstance : public AbstractVmReservedInstance {
public:
	ModuleProxyInstance(AnalyzedClass* clazz, VirtualMachine* v);
	virtual ~ModuleProxyInstance();
};

} /* namespace codablecash */

#endif /* INTER_MODULAR_ACCESS_MODULEPROXYINSTANCE_H_ */
