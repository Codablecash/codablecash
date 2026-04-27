/*
 * ObjectClassInstanceFactory.h
 *
 *  Created on: Nov 2, 2025
 *      Author: iizuka
 */

#ifndef INSTANCE_RESERVED_CLASSES_OBJECT_OBJECTCLASSINSTANCEFACTORY_H_
#define INSTANCE_RESERVED_CLASSES_OBJECT_OBJECTCLASSINSTANCEFACTORY_H_

#include "engine/sc_analyze/IVmInstanceFactory.h"

namespace alinous {

class ObjectClassInstanceFactory : public IVmInstanceFactory {
private:
	ObjectClassInstanceFactory();

public:
	static ObjectClassInstanceFactory* getInstance() noexcept;
	virtual ~ObjectClassInstanceFactory();

	virtual VmClassInstance* createInstance(AnalyzedClass* clazz, VirtualMachine* vm);
};

} /* namespace alinous */

#endif /* INSTANCE_RESERVED_CLASSES_OBJECT_OBJECTCLASSINSTANCEFACTORY_H_ */
