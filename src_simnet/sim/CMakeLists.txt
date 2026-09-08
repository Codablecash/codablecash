
set(__src
	CodableSimNet.cpp
	InstanceSimulator.cpp
	SimnetConfigException.cpp
	ZoneInstanceConfig.cpp
	ZoneSimulator.cpp
)
handle_sub(codablecashlib "${__src}" simnet sim)



