
set(testsrc
	MoveTestSessionHeadDitector.cpp
	MoveTestUtil.cpp
	test_build_instance_space.cpp
	test_make_callserial.cpp
	test_move_instance_space.cpp
)
handle_sub(testall "${testsrc}" test smartcontract_modular/instance_move)
