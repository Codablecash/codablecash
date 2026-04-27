

set(__src
	ClientConnectionSourceDescriptor.cpp
	IClientSocket.cpp
	IpV4ClientConnection.cpp
	IpV6ClientConnection.cpp
	IpV4ServerConnection.cpp
	IpV6ServerConnection.cpp
	IServerSocket.cpp
	StackConnectionCloser.cpp
	UnexpectedProtocolException.cpp
)
handle_sub(codablecashlib "${__src}" blockchain_p2p ipconnect)

