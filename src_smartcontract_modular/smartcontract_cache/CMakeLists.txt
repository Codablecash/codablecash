

set(__src
	InstanceSpace.cpp
	InstanceSpaceReleaser.cpp
	InstanceSpacesManager.cpp
	ModuleSetupException.cpp
	MolularSmartcontractCacheManager.cpp
	ProjectNotFoundException.cpp
)
handle_sub(codablecashlib "${__src}" smartcontract_modular smartcontract_cache)

