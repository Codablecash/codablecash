

set(__src
	AbstractExecutableModuleInstance.cpp
	ClassNotFoundException.cpp
	ExecutableModuleInstance.cpp
	InstanceDependencyContext.cpp
	InstanceDependencyHandler.cpp
	LibraryExectableModuleInstance.cpp
	ModularSmartcontractInstance.cpp
	ModuleInstanceClassLoader.cpp
	SmartcontractExecutionException.cpp
)
handle_sub(codablecashlib "${__src}" smartcontract_modular smartcontract_instance)

