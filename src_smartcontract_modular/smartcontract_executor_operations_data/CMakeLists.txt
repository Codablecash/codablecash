

set(__src
	AbstractSmartcontractOperationData.cpp
	CreateSmartcontractInstanceOperationData.cpp
	ProcessSmartcontractOperationData.cpp
)
handle_sub(codablecashlib "${__src}" smartcontract_modular smartcontract_executor_operations_data)

