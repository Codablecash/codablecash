/*
 * test_instance_spaces.cpp
 *
 *  Created on: Nov 30, 2025
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "../utils/ModularProjectUtils.h"

#include "modular_project/ModularSmartcontractProject.h"
#include "modular_project/SmartcontractProjectId.h"

#include "smartcontract_instance/ModularSmartcontractInstance.h"

#include "modular_project_registory/SmartcontractProjectData.h"
#include "modular_project_registory/ModularSmartcontractProjectRegistory.h"

#include "trx/session/base/CdbDatabaseSessionId.h"

#include "smartcontract_executor/ModularSmartcontractExecutor.h"
#include "smartcontract_executor/SmartcontractExecResult.h"

#include "transaction/SmartcontractInstanceAddress.h"

#include "smartcontract_cache/InstanceSpaceReleaser.h"
#include "smartcontract_cache/InstanceSpace.h"

#include "base/ArrayList.h"

#include "ext_arguments/AbstractFunctionExtArguments.h"
#include "ext_arguments/NumericArgument.h"

#include "engine/sc_analyze/AnalyzedType.h"

#include "ext_binary/ExtPrimitiveObject.h"

using namespace codablecash;
using namespace alinous;

TEST_GROUP(TestInstanceSpacesrGroup) {
	TEST_SETUP() {
		env->setup();
	}
	TEST_TEARDOWN() {
		env->teardown();
	}
};

TEST(TestInstanceSpacesrGroup, case01) {
	const File* projectFolder = this->env->getProjectRoot();
	ModularProjectUtils utils(L"src_test/smartcontract_modular/instance/resources/case02/project01/", projectFolder);
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

		InstanceSpace* space = executor.loadFromCache(address);
		InstanceSpaceReleaser __space(space);

		CHECK(space != nullptr);

		{
			ArrayList<AbstractFunctionExtArguments> args;
			args.setDeleteOnExit();
			args.addElement(new NumericArgument(100, AnalyzedType::TYPE_INT));

			UnicodeString module(L"exec");
			UnicodeString methodName(L"setCount");

			SmartcontractExecResult* result = space->invokeMainObjectMethod(&module, &methodName, &args); __STP(result);
			AbstractExtObject* obj = result->getReturnedValue();

			ExtPrimitiveObject* count = dynamic_cast<ExtPrimitiveObject*>(obj);
			int value = count->getIntValue();
			CHECK(value == 100);
		}

		executor.close();
	}
}

