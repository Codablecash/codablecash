

set(__src
	CommandPublisher.cpp
	CommandPublisherSetting.cpp
	CommandSubscriber.cpp
	CommandSubscriberThread.cpp
	HandshakeConnectionException.cpp
	IPubsubCommandExecutor.cpp
	IPubsubCommandListner.cpp
	IPubsubConnectionListener.cpp
	P2pHandshake.cpp
	PubsubCommandException.cpp
	PubSubId.cpp
	PubsubNetworkException.cpp
	PubsubUtils.cpp
)
handle_sub(codablecashlib "${__src}" blockchain_p2p pubsub)

