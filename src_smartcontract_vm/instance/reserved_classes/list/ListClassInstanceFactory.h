/*
 * ListClassInstanceFactory.h
 *
 *  Created on: Nov 6, 2025
 *      Author: iizuka
 */

#ifndef INSTANCE_RESERVED_CLASSES_LIST_LISTCLASSINSTANCEFACTORY_H_
#define INSTANCE_RESERVED_CLASSES_LIST_LISTCLASSINSTANCEFACTORY_H_

#include "engine/sc_analyze/IVmInstanceFactory.h"

namespace alinous {

class ListClassInstanceFactory : public IVmInstanceFactory {
public:
	ListClassInstanceFactory();
	virtual ~ListClassInstanceFactory();

	virtual VmClassInstance* createInstance(AnalyzedClass* clazz, VirtualMachine* vm);

	static ListClassInstanceFactory* getInstance() noexcept;
};

} /* namespace alinous */

#endif /* INSTANCE_RESERVED_CLASSES_LIST_LISTCLASSINSTANCEFACTORY_H_ */
