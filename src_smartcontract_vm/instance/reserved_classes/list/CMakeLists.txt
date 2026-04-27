

set(__src
	ListClassDeclare.cpp
	ListClassInstanceFactory.cpp
	ListListMethod.cpp
	VmListClassInstance.cpp
)
handle_sub(codablecashlib "${__src}" smartcontract_vm instance/reserved_classes/list)

