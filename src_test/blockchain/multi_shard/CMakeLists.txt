
set(testsrc
	NewShardAdder.cpp
	NewShardValidator.cpp
	test_multi_shard.cpp
	test_netwallet_stake.cpp
)
handle_sub(testall "${testsrc}" test blockchain/multi_shard)

