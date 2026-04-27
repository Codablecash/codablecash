
set(testsrc
	test_exceptions.cpp
	test_instance_spaces.cpp
	test_inter_module_access.cpp
	test_modular_instance.cpp
	test_smartcontract_executor.cpp
)
handle_sub(testall "${testsrc}" test smartcontract_modular/instance)
