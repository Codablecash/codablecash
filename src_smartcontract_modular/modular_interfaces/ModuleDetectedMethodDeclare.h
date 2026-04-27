/*
 * ModuleDetectedMethodDeclare.h
 *
 *  Created on: Dec 7, 2025
 *      Author: iizuka
 */

#ifndef MODULAR_INTERFACES_MODULEDETECTEDMETHODDECLARE_H_
#define MODULAR_INTERFACES_MODULEDETECTEDMETHODDECLARE_H_

#include "instance/reserved_classes/AbstractReservedMethodDeclare.h"

using namespace alinous;

namespace codablecash {

class ModuleDetectedMethodDeclare : public AbstractReservedMethodDeclare {
public:
	static const UnicodeString METHOD_NAME;

	ModuleDetectedMethodDeclare();
	virtual ~ModuleDetectedMethodDeclare();
};

} /* namespace codablecash */

#endif /* MODULAR_INTERFACES_MODULEDETECTEDMETHODDECLARE_H_ */
