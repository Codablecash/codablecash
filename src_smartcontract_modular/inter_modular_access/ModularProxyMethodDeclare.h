/*
 * ModularProxyMethodDeclare.h
 *
 *  Created on: Dec 4, 2025
 *      Author: iizuka
 */

#ifndef INTER_MODULAR_ACCESS_MODULARPROXYMETHODDECLARE_H_
#define INTER_MODULAR_ACCESS_MODULARPROXYMETHODDECLARE_H_

#include "instance/reserved_classes/AbstractReservedMethodDeclare.h"

namespace alinous {
class MethodDeclare;
}
using namespace alinous;

namespace codablecash {

class ModularProxyMethodDeclare : public AbstractReservedMethodDeclare {
public:
	ModularProxyMethodDeclare();
	virtual ~ModularProxyMethodDeclare();

	static ModularProxyMethodDeclare* fromMethodDeclare(MethodDeclare* method);

	virtual void init(VirtualMachine* vm);
	virtual void interpret(FunctionArguments* args, VirtualMachine* vm);
	virtual const UnicodeString* toString();

private:
	void doInterpret(FunctionArguments* args, VirtualMachine* vm);
};

} /* namespace codablecash */

#endif /* INTER_MODULAR_ACCESS_MODULARPROXYMETHODDECLARE_H_ */
