

set(__src
	AbstractSmartcontractOperation.cpp
	CreateSmartcontractInstanceOperation.cpp
	ModuleNotFoundException.cpp
	ProcessSmartcontractOperation.cpp
	RegisterSmartcontractProjectOperation.cpp
)
handle_sub(codablecashlib "${__src}" smartcontract_modular smartcontract_executor_operations)

