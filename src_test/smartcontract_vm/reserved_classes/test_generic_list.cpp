/*
 * test_generic_list.cpp
 *
 *  Created on: Nov 6, 2025
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "../VmTestUtils.h"
#include "ext_binary/ExtClassObject.h"



using namespace alinous;

TEST_GROUP(TestGenericListGroup) {
	TEST_SETUP(){}
	TEST_TEARDOWN(){}
};


TEST(TestGenericListGroup, case01){
	const File* projectFolder = this->env->getProjectRoot();
	VmTestUtils util(L"src_test/smartcontract_vm/reserved_classes/resources/list/case01/", projectFolder);

	bool result = util.loadAllFiles();
	CHECK(result)
	util.setMain(L"test.fw", L"SmartContract", L"main");

	result = util.analyze();
	CHECK(result)

	result = util.createInstance();
	CHECK(result)

	// FIXME TestGenericListGroup
}

TEST(TestGenericListGroup, case02){

}

