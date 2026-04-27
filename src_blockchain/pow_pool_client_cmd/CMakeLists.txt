
set(__src
	AbstractPowClientRequestCommand.cpp
	PoWPoolCheckDataCommand.cpp
	PoWPoolCheckDataCommandResponse.cpp
	PoWPoolNonceCalculatedCommand.cpp
	PoWPoolStatusCommand.cpp
	PoWPoolStatusCommandResponse.cpp
)
handle_sub(codablecashlib "${__src}" blockchain pow_pool_client_cmd)
