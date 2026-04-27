/*
 * ExecutableModule.cpp
 *
 *  Created on: Nov 11, 2025
 *      Author: iizuka
 */
#include "smartcontract_instance/ExecutableModuleInstance.h"

#include "base_io/File.h"

#include "base_io_stream/exceptions.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "bc/ExceptionThrower.h"

#include "vm/VirtualMachine.h"

namespace codablecash {

ExecutableModuleInstance::ExecutableModuleInstance() : AbstractExecutableModuleInstance(TYPE_EXEC) {


}

ExecutableModuleInstance::~ExecutableModuleInstance() {

}

void ExecutableModuleInstance::loadCompilantUnits(const File *projectBaseDir) {
	resetContract();
	parseSourceFolders(projectBaseDir);
}

} /* namespace codablecash */
