/*
 * P2pServer.h
 *
 *  Created on: 2022/08/23
 *      Author: iizuka
 */

#ifndef P2PSERVER_P2PSERVER_H_
#define P2PSERVER_P2PSERVER_H_

namespace alinous {
class UnicodeString;
class SysMutex;
}
using namespace alinous;

namespace codablecash {

class IServerSocket;
class P2pConnectionListeningThread;
class P2pServerConnectionManager;
class AbstractConnectionCommand;
class CommandPublisher;
class CommandSubscriber;
class ISystemLogger;
class IPubsubConnectionListener;
class IPubsubCommandExecutor;
class P2pHandshake;

class P2pServer {
public:
	explicit P2pServer(ISystemLogger* logger, IPubsubCommandExecutor* executor);
	virtual ~P2pServer();

	void startListening(int protocol, const UnicodeString *host, int port, const UnicodeString* nodeName);
	void startIpV6Listening(const UnicodeString* host, int port, const UnicodeString* nodeName);
	void startIpV4Listening(const UnicodeString* host, int port, const UnicodeString* nodeName);
	void stopListning();

	void close() noexcept;

	void dispose() noexcept;

	void addConnectionListener(IPubsubConnectionListener* listner) noexcept;

	void registerPublisher(CommandPublisher* publisher);
	void newHandShake(CommandSubscriber* subscriber);
	void registerHandshake(P2pHandshake* handshake);



	ISystemLogger* getLogger() const noexcept {
		return logger;
	}

	P2pServerConnectionManager* getConnectionManager() const noexcept {
		return this->conManager;
	}

private:
	void closeServerSocket() noexcept;
	void waitForStarted();

private:
	P2pConnectionListeningThread* thread;
	IServerSocket* srvSocket;
	int portSelf;

	P2pServerConnectionManager* conManager;
	SysMutex* pubsubWaitLock;

	ISystemLogger* logger;
};

} /* namespace codablecash */

#endif /* P2PSERVER_P2PSERVER_H_ */
