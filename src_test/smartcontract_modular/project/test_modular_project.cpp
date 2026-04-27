/*
 * test_modular_project.cpp
 *
 *  Created on: Sep 19, 2025
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "modular_project/ModularSmartcontractProject.h"
#include "modular_project/ModularConfigException.h"

#include "smartcontract_instance/ModularSmartcontractInstance.h"

#include "../utils/ModularProjectUtils.h"


using namespace codablecash;


TEST_GROUP(TestModularProjectGroup) {
	TEST_SETUP() {
		env->setup();
	}
	TEST_TEARDOWN() {
		env->teardown();
	}
};

TEST(TestModularProjectGroup, case01) {
	const File* projectFolder = this->env->getProjectRoot();

	ModularProjectUtils utils(L"src_test/smartcontract_modular/project/resources/case01/project01/", projectFolder);
	const File* baseDir = utils.getFolder();

	ModularSmartcontractProject project(baseDir);
	project.loadProject();
}


TEST(TestModularProjectGroup, error01) {
	testException<ModularConfigException>();
}


