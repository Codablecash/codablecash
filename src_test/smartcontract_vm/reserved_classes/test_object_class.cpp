/*
 * test_object_class.cpp
 *
 *  Created on: Nov 2, 2025
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "../VmTestUtils.h"
#include "ext_binary/ExtClassObject.h"



using namespace alinous;

TEST_GROUP(TestObjectClassGroup) {
	TEST_SETUP(){}
	TEST_TEARDOWN(){}
};

TEST(TestObjectClassGroup, case01){
	const File* projectFolder = this->env->getProjectRoot();
	VmTestUtils util(L"src_test/smartcontract_vm/reserved_classes/resources/object/case01/", projectFolder);

	bool result = util.loadAllFiles();
	CHECK(result)
	util.setMain(L"test.fw", L"SmartContract", L"main");

	result = util.analyze();
	CHECK(result)

	result = util.createInstance();
	CHECK(result)

	ExtClassObject* mainobj = util.getMainExtObject(); __STP(mainobj);
	ExtClassObject* obj = mainobj->getExtClassObject(L"obj");

	CHECK(obj != nullptr);
}

