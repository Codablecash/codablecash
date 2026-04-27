

set(__src
	AbstractSessionDetector.cpp
	IInstanceSessionContextListner.cpp
	InstanceIdIndex.cpp
	InstanceIdIndexData.cpp
	InstanceIdIndexDataFactory.cpp
	InstanceIdIndexKey.cpp
	InstanceIdIndexKeyFactory.cpp
	InstanceSessionContext.cpp
)
handle_sub(codablecashlib "${__src}" smartcontract_modular smartcontract_executor_index)

