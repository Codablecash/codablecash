
set(testsrc
	test_modular_project_registory.cpp
	test_modular_project.cpp
	test_modular_version_string.cpp
	test_modular_version.cpp
)
handle_sub(testall "${testsrc}" test smartcontract_modular/project)
