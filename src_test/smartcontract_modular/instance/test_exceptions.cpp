/*
 * test_exceptions.cpp
 *
 *  Created on: Dec 1, 2025
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "smartcontract_cache/ProjectNotFoundException.h"
#include "smartcontract_cache/ModuleSetupException.h"

#include "smartcontract_instance/SmartcontractExecutionException.h"

#include "smartcontract_executor_operations/ModuleNotFoundException.h"


using codablecash::ModuleNotFoundException;
using codablecash::ModuleSetupException;
using codablecash::ProjectNotFoundException;
using codablecash::SmartcontractExecutionException;


TEST_GROUP(TestModularExceptionsGroup) {
	TEST_SETUP() {
	}
	TEST_TEARDOWN() {
	}
};

TEST(TestModularExceptionsGroup, case01) {
	testException<ProjectNotFoundException>();
}

TEST(TestModularExceptionsGroup, case02) {
	testException<ModuleSetupException>();
}

TEST(TestModularExceptionsGroup, case03) {
	testException<SmartcontractExecutionException>();
}

TEST(TestModularExceptionsGroup, case04){
	testException<ModuleNotFoundException>();
}

