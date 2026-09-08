
set(__src
	SimnetClient.cpp
	SimnetPubsubCommandExecutor.cpp
)
handle_sub(codablecashlib "${__src}" simnet sim_net_client)

