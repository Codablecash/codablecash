

set(__src
	ModularSmartcontractProjectRegistory.cpp
	ProjectIdKey.cpp
	ProjectIdKeyFactory.cpp
	SmartcontractProjectData.cpp
	SmartcontractProjectDataFactory.cpp
)
handle_sub(codablecashlib "${__src}" smartcontract_modular modular_project_registory)

