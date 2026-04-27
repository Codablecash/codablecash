
set(testsrc
	test_pow_calclator.cpp
	test_pow_pool.cpp
)
handle_sub(testall "${testsrc}" test blockchain/pow_pool)

