/*
 * IInitializeCompilantUnitProvidor.h
 *
 *  Created on: Dec 8, 2025
 *      Author: iizuka
 */

#ifndef VM_IINITIALIZECOMPILANTUNITPROVIDOR_H_
#define VM_IINITIALIZECOMPILANTUNITPROVIDOR_H_

namespace alinous {

class VirtualMachine;

class IInitializeCompilantUnitProvidor {
public:
	IInitializeCompilantUnitProvidor();
	virtual ~IInitializeCompilantUnitProvidor();

	virtual void initCompilantUnits(VirtualMachine *vm) = 0;
};

} /* namespace alinous */

#endif /* VM_IINITIALIZECOMPILANTUNITPROVIDOR_H_ */
