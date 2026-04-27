
set(testsrc
	test_create_exec_operation.cpp
	test_instance_index.cpp
	test_operation_misc.cpp
	test_projectid_key.cpp
)
handle_sub(testall "${testsrc}" test smartcontract_modular/operation)
