/*
 * P2pServerConnectionManager.h
 *
 *  Created on: 2023/01/10
 *      Author: iizuka
 */

#ifndef P2PSERVER_P2PSERVERCONNECTIONMANAGER_H_
#define P2PSERVER_P2PSERVERCONNECTIONMANAGER_H_

#include <cstdint>

#include "base/ArrayList.h"

namespace alinous {
class SysMutex;
class Exception;
class UnicodeString;
}
using namespace alinous;

namespace codablecash {

class CommandPublisher;
class CommandSubscriber;
class PubSubId;
class P2pHandshake;
class ISystemLogger;
class LoginPubSubCommand;
class LoginClientCommand;
class IPubsubConnectionListener;
class IPubsubCommandExecutor;

class P2pServerConnectionManager {
public:
	P2pServerConnectionManager(ISystemLogger* logger, IPubsubCommandExecutor* executor);
	virtual ~P2pServerConnectionManager();

	void dispose();

	void registerPublisher(CommandPublisher* publisher);
	CommandPublisher* getPublisher(const PubSubId* pubsubId);
	CommandPublisher* getPublisher(const PubSubId* pubsubId, bool remove);

	void newHandshake(CommandPublisher* publisher, CommandSubscriber* subscriber);
	void registerHandshake(P2pHandshake* handshake);
	void removeDisabledHandshake() noexcept;

	P2pHandshake* getP2pHandshake(const PubSubId *pubsubId) const noexcept;

	bool handShakeExists(const PubSubId *pubsubId) const noexcept;

	void setExpireSec(uint64_t expireSec) noexcept {
		this->expireSec = expireSec;
	}

	ArrayList<PubSubId>* getHandshakeIdList() const noexcept;

	void addConnectionListener(IPubsubConnectionListener* listner) noexcept;

	void fireHandshakeEnds(P2pHandshake* handshake);
	void fireHandshakeEndsWithExeption(P2pHandshake* handshake, const Exception *e);
	void fireLoginHandshake(P2pHandshake* handshake, const LoginPubSubCommand* loginCommand, const UnicodeString* canonicalName);
	void fireClientLoginHandshake(P2pHandshake* handshake, const LoginClientCommand* clientLoginCommand);

#ifdef __DEBUG__
	void __testRemoveExpiredWaitingPublisher(){
		removeExpiredWaitingPublisher();
	}
#endif

private:
	void removeExpiredWaitingPublisher();
	void __removeDisabledHandshake() noexcept;

private:
	ArrayList<CommandPublisher>* waitlist;
	SysMutex* waitlistMutex;
	uint64_t expireSec;

	ArrayList<P2pHandshake>* hslist;
	SysMutex* hslistMutex;

	ISystemLogger* logger;

	SysMutex* listnersMutex;
	ArrayList<IPubsubConnectionListener>* conlistsners;
	IPubsubCommandExecutor* executor; // instance
};

} /* namespace codablecash */

#endif /* P2PSERVER_P2PSERVERCONNECTIONMANAGER_H_ */
