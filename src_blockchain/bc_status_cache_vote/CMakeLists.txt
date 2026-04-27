

set(__src
	HeightVoteData.cpp
	HeightVoteDataFactory.cpp
	VoteManager.cpp
)
handle_sub(codablecashlib "${__src}" blockchain bc_status_cache_vote)

