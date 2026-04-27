

set(__src
	AbstractVmInstance.cpp
	IAbstractVmInstanceSubstance.cpp
	IInstanceContainer.cpp
	VmAllocatee.cpp
	VmClassInstance.cpp
	VmNullprtInstance.cpp
)
handle_sub(codablecashlib "${__src}" smartcontract_vm instance)

