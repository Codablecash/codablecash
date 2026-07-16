

set(__src
	AbstractShardExtentionValidator.cpp
	BlockchainController.cpp
	BlockchainStatusCache.cpp
	BlockHead.cpp
	BlockHeadElement.cpp
	BlockHeadElementPadding.cpp
	BlockIdChangedException.cpp
	BlockStack.cpp
	BlockStackElement.cpp
	HeadBlockDetector.cpp
	HeadBlockDetectorCache.cpp
	HeadBlockDetectorCacheElement.cpp
	IBlockchainStoreProvider.cpp
	ZoneStatusCache.cpp
)
handle_sub(codablecashlib "${__src}" blockchain bc_status_cache)

