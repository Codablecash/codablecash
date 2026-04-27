

set(__src
	InterModuleAccessException.cpp
	ModularProxyClassDeclare.cpp
	ModularProxyMethodDeclare.cpp
	ModularProxyObjectInstanceFactory.cpp
	ModuleProxyInstance.cpp
)
handle_sub(codablecashlib "${__src}" smartcontract_modular inter_modular_access)

