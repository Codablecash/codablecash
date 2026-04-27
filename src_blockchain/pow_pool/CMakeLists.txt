
set(__src
	NetworkPoWPool.cpp
	Pool2ClientTransferCommandMessage.cpp
	PoWRequest2Client.cpp
	PoWRequestData.cpp
	PoWRequestStatus.cpp
	PoWRequestStatusData.cpp
)
handle_sub(codablecashlib "${__src}" blockchain pow_pool)
