/*
 * test_exception_std.cpp
 *
 *  Created on: Apr 28, 2026
 *      Author: iizuka
 */
#include <exception>

#include "test_utils/t_macros.h"

using std::__exception_ptr::exception_ptr;

TEST_GROUP(TestExceptionStrGroup) {
	TEST_SETUP(){}
	TEST_TEARDOWN(){}
};

class ErrObject {
public:
};

TEST(TestExceptionStrGroup, case01){
	try {
		throw new ErrObject();
	}
	catch (...) {
		std::exception_ptr p = std::current_exception();

		try{
			std::rethrow_exception(p);
		}catch(ErrObject* e){
			if(e != nullptr){
				delete e;
			}
		}

	}
}



