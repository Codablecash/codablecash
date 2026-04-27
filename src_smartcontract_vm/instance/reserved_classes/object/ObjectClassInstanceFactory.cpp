/*
 * ObjectClassInstanceFactory.cpp
 *
 *  Created on: Nov 2, 2025
 *      Author: iizuka
 */

#include "instance/reserved_classes/object/ObjectClassInstanceFactory.h"
#include "instance/reserved_classes/object/VmObjectClassInstance.h"


namespace alinous {

ObjectClassInstanceFactory::ObjectClassInstanceFactory() {


}

ObjectClassInstanceFactory::~ObjectClassInstanceFactory() {

}

VmClassInstance* ObjectClassInstanceFactory::createInstance(AnalyzedClass *clazz, VirtualMachine *vm) {
	VmObjectClassInstance* inst = new(vm) VmObjectClassInstance(clazz, vm);

	return inst;
}

ObjectClassInstanceFactory* ObjectClassInstanceFactory::getInstance() noexcept {
	static ObjectClassInstanceFactory factory;
	return &factory;
}

} /* namespace alinous */
