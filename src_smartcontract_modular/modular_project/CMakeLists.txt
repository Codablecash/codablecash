

set(__src
	AbstractDependencyConfig.cpp
	AbstractSmartcontractModule.cpp
	DependencyConfig.cpp
	ExecutableSmartcontractModule.cpp
	LibrarySmartcontractModule.cpp
	LocalProjectModuleDependencyConfig.cpp
	ModularConfigException.cpp
	ModularInstanceConfig.cpp
	ModularProjectConfig.cpp
	ModularInstanceConfig.cpp
	ModularSmartcontractProject.cpp
	ModularSmartcontractVersion.cpp
	ModularConfigException.cpp
	SmartcontractProjectId.cpp
)
handle_sub(codablecashlib "${__src}" smartcontract_modular modular_project)

