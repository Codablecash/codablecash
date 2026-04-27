
set(testsrc
	test_argument_binary.cpp
	test_argument_type.cpp
	test_call_maininst.cpp
	test_function_call_arg.cpp
	test_function_call.cpp
	test_select_methods.cpp
	test_static_invoke.cpp
	test_virtual_func_call.cpp
)
handle_sub(testall "${testsrc}" test smartcontract_vm/method_invoke)

