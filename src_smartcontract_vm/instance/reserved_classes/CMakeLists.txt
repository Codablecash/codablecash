

set(__src
	AbstractReservedClassDeclare.cpp
	AbstractReservedMethodDeclare.cpp
	AbstractVmReservedInstance.cpp
	ReservedClassRegistory.cpp
)
handle_sub(codablecashlib "${__src}" smartcontract_vm instance/reserved_classes)

