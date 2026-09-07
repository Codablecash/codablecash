

set(__dirsrc "")

# import source codes
add_subdirectory(sim)
add_subdirectory(sim_net)
add_subdirectory(sim_net_client)

message(STATUS "src_simnet : ${__dirsrc}")

set_parentv("${__dirsrc}" src_simnet top_src_simnet)

