

set(__src
	StringClassDeclare.cpp
	StringEqualsMethodDeclare.cpp
)
handle_sub(codablecashlib "${__src}" smartcontract_vm instance/reserved_classes_string)

