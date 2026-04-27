

set(__src
	ProjectIdIndex.cpp
	ProjectIdIndexKey.cpp
	ProjectIdIndexKeyFactory.cpp
	ProjectIndexData.cpp
	ProjectIndexDataFactory.cpp
	SmartcontractVersionKey.cpp
)
handle_sub(codablecashlib "${__src}" smartcontract_modular smartcontract_executor_index_projectid)

