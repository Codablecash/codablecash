
set(testsrc
	test_statement_error.cpp
	test_exception_std.cpp
)
handle_sub(testall "${testsrc}" test smartcontract_err)

