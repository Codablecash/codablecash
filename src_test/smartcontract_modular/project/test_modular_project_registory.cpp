/*
 * test_modular_project_registory.cpp
 *
 *  Created on: Nov 24, 2025
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "modular_project/ModularSmartcontractProject.h"
#include "modular_project/ModularConfigException.h"

#include "smartcontract_instance/ModularSmartcontractInstance.h"

#include "../utils/ModularProjectUtils.h"

#include "base_io/ByteBuffer.h"

#include "modular_project_registory/ModularSmartcontractProjectRegistory.h"
#include "modular_project_registory/SmartcontractProjectData.h"


using namespace codablecash;


TEST_GROUP(TestModularProjectRegisterGroup) {
	TEST_SETUP() {
		env->setup();
	}
	TEST_TEARDOWN() {
		env->teardown();
	}
};

TEST(TestModularProjectRegisterGroup, case01){
	const File* projectFolder = this->env->getProjectRoot();

	ModularProjectUtils utils(L"src_test/smartcontract_modular/project/resources/reg_case01/project01/", projectFolder);
	const File* baseDir = utils.getFolder();

	ModularSmartcontractProject project(baseDir);
	project.loadProject();

	ModularSmartcontractInstance* inst = project.toInstance(); __STP(inst);
	inst->loadCompilantUnits(baseDir);

	bool res = inst->hasCompileError();
	CHECK(res == false);

	{
		int cap = inst->binarySize();
		ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);

		inst->toBinary(buff);
		CHECK(buff->position() == buff->capacity());

		buff->position(0);

		ModularSmartcontractInstance* inst2 = ModularSmartcontractInstance::createFromBinary(buff); __STP(inst2);
		ByteBuffer* buff2 = ByteBuffer::allocateWithEndian(cap, true); __STP(buff2);
		inst2->toBinary(buff2);

		int cmp = buff->binaryCmp(buff2);
		CHECK(cmp == 0);
	}
}

TEST(TestModularProjectRegisterGroup, case02){
	const File* projectFolder = this->env->getProjectRoot();

	File projectTestFolder = this->env->testCaseDir();
	_ST(File, baseTestDir, projectTestFolder.get(L"store"))

	ModularProjectUtils utils(L"src_test/smartcontract_modular/project/resources/reg_case01/project01/", projectFolder);
	const File* baseDir = utils.getFolder();

	ModularSmartcontractProject project(baseDir);
	project.loadProject();

	ModularSmartcontractInstance* inst = project.toInstance(); __STP(inst);
	inst->loadCompilantUnits(baseDir);

	bool res = inst->hasCompileError();
	CHECK(res == false);

	{
		ModularSmartcontractProjectRegistory registory(baseTestDir);
		registory.createBlankDatabase();

		registory.open();
		SmartcontractProjectData* data = inst->createData(); __STP(data);

		const ProjectIdKey* key = data->getKey();
		registory.put(key, data);

		SmartcontractProjectData* data2 = registory.findProjectById(key); __STP(data2);
		CHECK(data != nullptr);

		registory.close();
	}


}
