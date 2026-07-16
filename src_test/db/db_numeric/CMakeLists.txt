
set(testsrc
	test_big_decimal.cpp
	test_big_integer_misc.cpp
	test_big_integer.cpp
	test_big_integer_rand.cpp
	test_div.cpp
	test_mul.cpp
	test_Tonelli_Shanks.cpp
)
handle_sub(testall "${testsrc}" test db/db_numeric)

