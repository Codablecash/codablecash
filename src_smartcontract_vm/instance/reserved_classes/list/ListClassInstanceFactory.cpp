/*
 * ListClassInstanceFactory.cpp
 *
 *  Created on: Nov 6, 2025
 *      Author: iizuka
 */

#include "instance/reserved_classes/list/ListClassInstanceFactory.h"
#include "instance/reserved_classes/list/VmListClassInstance.h"

namespace alinous {

ListClassInstanceFactory::ListClassInstanceFactory() {

}

ListClassInstanceFactory::~ListClassInstanceFactory() {

}

VmClassInstance* ListClassInstanceFactory::createInstance(AnalyzedClass *clazz, VirtualMachine *vm) {
	VmListClassInstance* inst = new(vm) VmListClassInstance(clazz, vm);

	return inst;
}

ListClassInstanceFactory* ListClassInstanceFactory::getInstance() noexcept {
	static ListClassInstanceFactory inst;
	return &inst;
}

} /* namespace alinous */
