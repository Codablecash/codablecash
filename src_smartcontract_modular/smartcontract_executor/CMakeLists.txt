

set(__src
	ModularSmartcontractExecutor.cpp
	SmartcontractExecContextData.cpp
	SmartcontractExecContextDataFactory.cpp
	SmartcontractExecContextKey.cpp
	SmartcontractExecContextKeyFactory.cpp
	SmartcontractExecContextRegistory.cpp
	SmartcontractExecResult.cpp
)
handle_sub(codablecashlib "${__src}" smartcontract_modular smartcontract_executor)

