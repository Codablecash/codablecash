

set(__src
	CachedStatusCache.cpp
	CachedStatusCacheContext.cpp
	CachedStatusCacheRepository.cpp
	IStatusCacheContext.cpp
	NullLockinManager.cpp
	RemoteUtxoDetector.cpp
	RemoteUtxoHeight.cpp
	RemovedDummyUtxo.cpp
	StatusCacheContext.cpp
	TransactionContextCache.cpp
	UtxoCacheContext.cpp
)
handle_sub(codablecashlib "${__src}" blockchain bc_status_cache_context)

