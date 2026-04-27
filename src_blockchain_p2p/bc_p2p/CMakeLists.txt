

set(__src
	BlochchainP2pManager.cpp
	BlockchainNodeHandshake.cpp
	BlockchainNodeHandshakeException.cpp
	BlockchainZoneException.cpp
	ClientNodeHandshake.cpp
	P2pZone.cpp
	StackHandshakeReleaser.cpp
	StackRawHandshakeReleaser.cpp
)
handle_sub(codablecashlib "${__src}" blockchain_p2p bc_p2p)

