/*
 * ExecutableSmartcontractModule.h
 *
 *  Created on: Sep 19, 2025
 *      Author: iizuka
 */

#ifndef MODULAR_PROJECT_EXECUTABLESMARTCONTRACTMODULE_H_
#define MODULAR_PROJECT_EXECUTABLESMARTCONTRACTMODULE_H_

#include "modular_project/AbstractSmartcontractModule.h"

namespace codablecash {

class ExecutableSmartcontractModule : public AbstractSmartcontractModule {
public:
	explicit ExecutableSmartcontractModule(const UnicodeString* projectRelativePath);
	virtual ~ExecutableSmartcontractModule();

	virtual void load(const File* modulePath);

	virtual AbstractExecutableModuleInstance* toInstance() const;
};

} /* namespace codablecash */

#endif /* MODULAR_PROJECT_EXECUTABLESMARTCONTRACTMODULE_H_ */
