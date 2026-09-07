/*
 * test_sim_config.cpp
 *
 *  Created on: Sep 6, 2026
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "json/JsonHandler.h"

using namespace codablecash;

// JsonHandler

TEST_GROUP(TestSimConfigGroup) {
	TEST_SETUP(){this->env->setup();}
	TEST_TEARDOWN(){this->env->teardown();}
};

TEST(TestSimConfigGroup, case01){
	File projectFolder = this->env->testCaseDir();

	//JsonHandler
	// JsonArrayObject


}
