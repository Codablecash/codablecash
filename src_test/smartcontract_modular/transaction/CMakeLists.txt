
set(testsrc
	test_random_address.cpp
	test_smartcontract_address.cpp
)
handle_sub(testall "${testsrc}" test smartcontract_modular/transaction)
