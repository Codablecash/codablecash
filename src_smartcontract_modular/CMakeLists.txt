
set(__dirsrc "")

# import source codes
add_subdirectory(inter_modular_access)
add_subdirectory(modular_interfaces)
add_subdirectory(modular_project)
add_subdirectory(modular_project_registory)
add_subdirectory(smartcontract_cache)
add_subdirectory(smartcontract_executor)
add_subdirectory(smartcontract_executor_cursor)
add_subdirectory(smartcontract_executor_index)
add_subdirectory(smartcontract_executor_index_projectid)
add_subdirectory(smartcontract_executor_operations)
add_subdirectory(smartcontract_executor_operations_data)
add_subdirectory(smartcontract_instance)
add_subdirectory(transaction)

message(STATUS "src_smartcontract_modular : ${__dirsrc}")

set_parentv("${__dirsrc}" src_smartcontract_modular top_src_smartcontract_modular)
