/*
 * test_inter_module_access.cpp
 *
 *  Created on: Dec 3, 2025
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "../utils/ModularProjectUtils.h"

#include "modular_project/ModularSmartcontractProject.h"
#include "modular_project/SmartcontractProjectId.h"

#include "smartcontract_executor/ModularSmartcontractExecutor.h"

#include "modular_project_registory/SmartcontractProjectData.h"

#include "transaction/SmartcontractInstanceAddress.h"

#include "smartcontract_cache/InstanceSpaceReleaser.h"

#include "base/ArrayList.h"

#include "ext_arguments/AbstractFunctionExtArguments.h"

#include "smartcontract_executor/SmartcontractExecResult.h"

#include "smartcontract_cache/InstanceSpace.h"

#include "ext_binary/AbstractExtObject.h"


using namespace codablecash;
using namespace alinous;

TEST_GROUP(TestInterModuleAccessGroup) {
	TEST_SETUP() {
		env->setup();
	}
	TEST_TEARDOWN() {
		env->teardown();
	}
};

TEST(TestInterModuleAccessGroup, case01){
	const File* projectFolder = this->env->getProjectRoot();
	ModularProjectUtils utils(L"src_test/smartcontract_modular/instance/resources/case03/project01/", projectFolder);
	const File* baseDir = utils.getFolder();

	ModularSmartcontractProject project(baseDir);
	project.loadProject();

	// Executor
	File projectTestFolder = this->env->testCaseDir();
	_ST(File, baseTestDir, projectTestFolder.get(L"exec"))

	ModularSmartcontractExecutor executor(baseTestDir);
	executor.createExecutor();
	{
		executor.open();
		executor.registerModularSmartcontractProject(&project);

		SmartcontractProjectId* projectId = project.getProjectId(); __STP(projectId);
		SmartcontractProjectData* data = executor.getProject(projectId); __STP(data);
		CHECK(data != nullptr);

		SmartcontractInstanceAddress* address = SmartcontractInstanceAddress::createAddress(0); __STP(address);
		executor.createInstance(address, projectId);

		{
			InstanceSpace* space = executor.loadFromCache(address);
			InstanceSpaceReleaser __space(space);
			CHECK(space != nullptr);

			ArrayList<AbstractFunctionExtArguments> args;
			args.setDeleteOnExit();

			UnicodeString module(L"exec");
			UnicodeString methodName(L"callConnector");

			SmartcontractExecResult* result = space->invokeMainObjectMethod(&module, &methodName, &args); __STP(result);
			AbstractExtObject* obj = result->getReturnedValue();
		}

		executor.close();
	}
}

