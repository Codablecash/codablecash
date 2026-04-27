

set(__src
	exceptions.cpp
	IInitializeCompilantUnitProvidor.cpp
	VirtualMachine.cpp
	VmSelectPlannerSetter.cpp
)
handle_sub(codablecashlib "${__src}" smartcontract_vm vm)

