
set(__src
	PoWClientMainThread.cpp
	PoWNonceCalcCommandMessage.cpp
	PoWNonceProcessor.cpp
	PoWNonceProcessorParam.cpp
	PoWPubsubCommandExecutor.cpp
	PoWWorkerClient.cpp
	PoWWorkerId.cpp
)
handle_sub(codablecashlib "${__src}" blockchain pow_pool_client)
