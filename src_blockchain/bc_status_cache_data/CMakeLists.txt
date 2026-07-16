

set(__src
	FinalizedDataCache.cpp
	FinalizedUtxoRepository.cpp
	FinalizedVoterRepository.cpp
	RemoteUtxoData.cpp
	RemoteUtxoDataFactory.cpp
	RemoteUtxoHeightIndex.cpp
	RemoteUtxoHeightIndexData.cpp
	RemoteUtxoHeightIndexDataFactory.cpp
	RemoteUtxoHistory.cpp;
	RemoteUtxoRepository.cpp
)
handle_sub(codablecashlib "${__src}" blockchain bc_status_cache_data)

