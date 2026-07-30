

set(__src
	GenerateNewGenesisBlockCommandMessage.cpp
	NotifyShardExtendRequestCommandMessage.cpp
	NotifyZoneExtendRequestedTransaction.cpp
	SendNotifyZoneExtendRequestedNodeCommand.cpp
)
handle_sub(codablecashlib "${__src}" blockchain bc_status_cache_extend_shard)

