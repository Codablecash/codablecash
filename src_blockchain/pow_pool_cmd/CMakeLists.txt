
set(__src
	AbstractPoWClientNotifyCommand.cpp
	PoWPoolNotifyDataChangedCommand.cpp
)
handle_sub(codablecashlib "${__src}" blockchain pow_pool_cmd)
