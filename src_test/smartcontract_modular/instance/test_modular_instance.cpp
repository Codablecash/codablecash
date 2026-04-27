/*
 * test_modular_instance.cpp
 *
 *  Created on: Sep 19, 2025
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "../utils/ModularProjectUtils.h"

#include "modular_project/ModularSmartcontractProject.h"

#include "smartcontract_instance/ModularSmartcontractInstance.h"

#include "modular_project_registory/SmartcontractProjectData.h"
#include "modular_project_registory/ModularSmartcontractProjectRegistory.h"

#include "trx/session/base/CdbDatabaseSessionId.h"

#include "transaction/SmartcontractInstanceAddress.h"


using namespace codablecash;


TEST_GROUP(TestModularInstanceGroup) {
	TEST_SETUP() {
		env->setup();
	}
	TEST_TEARDOWN() {
		env->teardown();
	}
};


TEST(TestModularInstanceGroup, case01){
	const File* projectFolder = this->env->getProjectRoot();

	ModularProjectUtils utils(L"src_test/smartcontract_modular/instance/resources/case01/project01/", projectFolder);
	const File* baseDir = utils.getFolder();

	ModularSmartcontractProject project(baseDir);
	project.loadProject();


	ModularSmartcontractInstance* inst = project.toInstance(); __STP(inst);
	inst->loadCompilantUnits(baseDir);

	bool res = inst->hasCompileError();
	CHECK(res == false);

	res = inst->preAnalyzeGoup();
	CHECK(res == false);

	res = inst->analyzeTypeRefGroup();
	CHECK(res == false);

	res = inst->analyzeMetadataGroup();
	CHECK(res == false);

	res = inst->analyzeGroup();
	CHECK(res == false);

	inst->setMainInstance();
	res = inst->createMainInstance();
	CHECK(res == false);

	inst->resetRootReference();

	inst->setMainInstance();
	res = inst->createMainInstance();
	CHECK(res == false);
}

TEST(TestModularInstanceGroup, case02){
	File projectTestFolder = this->env->testCaseDir();
	_ST(File, baseTestDir, projectTestFolder.get(L"store"))

	const File* projectFolder = this->env->getProjectRoot();

	ModularProjectUtils utils(L"src_test/smartcontract_modular/instance/resources/case02/project01/", projectFolder);
	const File* baseDir = utils.getFolder();

	ModularSmartcontractProject project(baseDir);
	project.loadProject();

	ModularSmartcontractInstance* inst = project.toInstance(); __STP(inst);
	inst->loadCompilantUnits(baseDir);

	bool res = inst->hasCompileError();
	CHECK(res == false);

	SmartcontractInstanceAddress* address = SmartcontractInstanceAddress::createAddress(0); __STP(address);
	inst->setSmartcontractInstanceAddress(address);

	inst->setDatabaseDir(baseTestDir);
	inst->createDatabase();
	inst->loadDatabase();

	res = inst->preAnalyzeGoup();
	res |= inst->analyzeTypeRefGroup();
	res |= inst->analyzeMetadataGroup();
	res |= inst->analyzeGroup();
	CHECK(res == false);

	inst->setMainInstance();
	res = inst->createMainInstance();
	CHECK(res == false);
}

TEST(TestModularInstanceGroup, generateSession01){
	CdbDatabaseSessionId* baseId = CdbDatabaseSessionId::createRandomId(); __STP(baseId);
	CdbDatabaseSessionId* copy = CdbDatabaseSessionId::from32BytesId(baseId); __STP(copy);

	int cmp = baseId->compareTo(copy);
	CHECK(cmp == 0);
}

