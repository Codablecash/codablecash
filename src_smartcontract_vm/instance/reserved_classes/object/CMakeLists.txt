

set(__src
	ObjectClassDeclare.cpp
	ObjectClassInstanceFactory.cpp
	ObjectObjectMethod.cpp
	VmObjectClassInstance.cpp
)
handle_sub(codablecashlib "${__src}" smartcontract_vm instance/reserved_classes/object)

