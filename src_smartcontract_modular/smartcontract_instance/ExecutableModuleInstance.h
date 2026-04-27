/*
 * ExecutableModule.h
 *
 *  Created on: Nov 11, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_INSTANCE_EXECUTABLEMODULEINSTANCE_H_
#define SMARTCONTRACT_INSTANCE_EXECUTABLEMODULEINSTANCE_H_

#include "base/ArrayList.h"
#include "smartcontract_instance/AbstractExecutableModuleInstance.h"

namespace codablecash {


class ExecutableModuleInstance : public AbstractExecutableModuleInstance {
public:
	ExecutableModuleInstance();
	virtual ~ExecutableModuleInstance();

	virtual void loadCompilantUnits(const File* projectBaseDir);

};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_INSTANCE_EXECUTABLEMODULEINSTANCE_H_ */
