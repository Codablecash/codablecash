/*
 * P2pServerConnectionManager.cpp
 *
 *  Created on: 2023/01/10
 *      Author: iizuka
 */

#include "p2pserver/P2pServerConnectionManager.h"

#include "pubsub/CommandPublisher.h"
#include "pubsub/PubSubId.h"
#include "pubsub/P2pHandshake.h"
#include "pubsub/CommandSubscriber.h"
#include "pubsub/IPubsubConnectionListener.h"

#include "base_thread/SysMutex.h"
#include "base_thread/StackUnlocker.h"

#include "osenv/funcs.h"

namespace codablecash {

P2pServerConnectionManager::P2pServerConnectionManager(ISystemLogger* logger, IPubsubCommandExecutor* executor) {
	this->waitlist = new ArrayList<CommandPublisher>();
	this->waitlistMutex = new SysMutex();

	this->hslist = new ArrayList<P2pHandshake>();
	this->hslistMutex = new SysMutex();

	this->expireSec = 200;
	this->logger = logger;

	this->listnersMutex = new SysMutex();
	this->conlistsners = new ArrayList<IPubsubConnectionListener>();
	this->executor = executor;
}

P2pServerConnectionManager::~P2pServerConnectionManager() {
	delete this->waitlist;
	delete this->waitlistMutex;

	delete this->hslist;
	delete this->hslistMutex;

	this->logger = nullptr;

	delete this->conlistsners;
	delete this->listnersMutex;
}

void P2pServerConnectionManager::dispose() {
	{
		StackUnlocker __lock(this->listnersMutex, __FILE__, __LINE__);
		this->conlistsners->reset();
	}

	this->waitlist->deleteElements();
	this->hslist->deleteElements();
}

void P2pServerConnectionManager::registerPublisher(CommandPublisher *publisher) {
	StackUnlocker __lock(this->waitlistMutex, __FILE__, __LINE__);
	removeExpiredWaitingPublisher();

	this->waitlist->addElement(publisher);
}

CommandPublisher* P2pServerConnectionManager::getPublisher(const PubSubId *pubsubId) {
	return getPublisher(pubsubId, false);
}

CommandPublisher* P2pServerConnectionManager::getPublisher(	const PubSubId *pubsubId, bool remove) {
	StackUnlocker __lock(this->waitlistMutex, __FILE__, __LINE__);
	removeExpiredWaitingPublisher();

	CommandPublisher* ret = nullptr;

	int maxLoop = this->waitlist->size();
	for(int i = 0; i != maxLoop; ++i){
		CommandPublisher* pub = this->waitlist->get(i);
		const PubSubId* psid = pub->getPubsubId();

		if(pubsubId->equals(psid)){
			ret = pub;
			break;
		}
	}

	if(remove && ret != nullptr){
		this->waitlist->removeByObj(ret);
	}

	return ret;
}

void P2pServerConnectionManager::addConnectionListener(IPubsubConnectionListener *listner) noexcept {
	StackUnlocker __lock(this->listnersMutex, __FILE__, __LINE__);

	this->conlistsners->addElement(listner);
}

void P2pServerConnectionManager::removeExpiredWaitingPublisher() {
	uint64_t nowtm = Os::getTimestampLong();
	uint64_t beforelimit = nowtm - this->expireSec; // 20 seconds

	ArrayList<CommandPublisher> dellist;

	// check
	int maxLoop = this->waitlist->size();
	for(int i = 0; i != maxLoop; ++i){
		CommandPublisher* pub = this->waitlist->get(i);

		uint64_t tm = pub->getTimestamp();
		if(tm <= beforelimit){
			dellist.addElement(pub);
		}
	}

	// delete
	maxLoop = dellist.size();
	for(int i = 0; i != maxLoop; ++i){
		CommandPublisher* pub = dellist.get(i);

		this->waitlist->removeByObj(pub);
		pub->close();
	}
	dellist.deleteElements();
}

void P2pServerConnectionManager::newHandshake(CommandPublisher *publisher, CommandSubscriber *subscriber) {
	{
		StackUnlocker __lock(this->hslistMutex, __FILE__, __LINE__);
		P2pHandshake* handshake = new P2pHandshake(publisher, subscriber, this->logger, this, this->executor);

		this->hslist->addElement(handshake);
	}

	subscriber->startSubscribe();
}

void P2pServerConnectionManager::registerHandshake(P2pHandshake *handshake) {
	StackUnlocker __lock(this->hslistMutex, __FILE__, __LINE__);
	__removeDisabledHandshake();

	this->hslist->addElement(handshake);
}

ArrayList<PubSubId>* P2pServerConnectionManager::getHandshakeIdList() const noexcept {
	StackUnlocker __lock(this->hslistMutex, __FILE__, __LINE__);

	ArrayList<PubSubId>* list = new ArrayList<PubSubId>();

	int maxLoop = this->hslist->size();
	for(int i = 0; i != maxLoop; ++i){
		P2pHandshake* hs = this->hslist->get(i);

		if(!hs->is2Delete()){
			const PubSubId* id = hs->getPubsubid();
			list->addElement(new PubSubId(*id));
		}
	}

	return list;
}

P2pHandshake* P2pServerConnectionManager::getP2pHandshake(const PubSubId *pubsubId) const noexcept {
	StackUnlocker __lock(this->hslistMutex, __FILE__, __LINE__);

	P2pHandshake* ret = nullptr;

	int maxLoop = this->hslist->size();
	for(int i = 0; i != maxLoop; ++i){
		P2pHandshake* hs = this->hslist->get(i);
		const PubSubId *id = hs->getPubsubid();

		if(id->equals(pubsubId)){
			ret = hs;
			hs->addRef();
		}
	}

	return ret;
}

bool P2pServerConnectionManager::handShakeExists(const PubSubId *pubsubId) const noexcept {
	StackUnlocker __lock(this->hslistMutex, __FILE__, __LINE__);

	bool ret = false;

	int maxLoop = this->hslist->size();
	for(int i = 0; i != maxLoop; ++i){
		P2pHandshake* hs = this->hslist->get(i);
		const PubSubId *id = hs->getPubsubid();

		if(id->equals(pubsubId)){
			ret = true;
		}
	}

	return ret;
}

void P2pServerConnectionManager::removeDisabledHandshake() noexcept {
	StackUnlocker __lock(this->hslistMutex, __FILE__, __LINE__);

	__removeDisabledHandshake();
}

void P2pServerConnectionManager::__removeDisabledHandshake() noexcept {
	ArrayList<P2pHandshake> dellist;

	// check
	int maxLoop = this->hslist->size();
	for(int i = 0; i != maxLoop; ++i){
		P2pHandshake* hs = this->hslist->get(i);
		if(hs->is2Delete()){
			dellist.addElement(hs);
		}
	}

	// delete
	maxLoop = dellist.size();
	for(int i = 0; i != maxLoop; ++i){
		P2pHandshake* hs = dellist.get(i);

		this->hslist->removeByObj(hs);
	}

	dellist.deleteElements();
}

void P2pServerConnectionManager::fireHandshakeEnds(P2pHandshake *handshake) {
	StackUnlocker __lock(this->listnersMutex, __FILE__, __LINE__);

	int maxLoop = this->conlistsners->size();
	for(int i = 0; i != maxLoop; ++i){
		IPubsubConnectionListener* listner = this->conlistsners->get(i);
		listner->onHandshakeEnds(handshake);
	}
}
void P2pServerConnectionManager::fireHandshakeEndsWithExeption(P2pHandshake *handshake, const Exception *e) {
	StackUnlocker __lock(this->listnersMutex, __FILE__, __LINE__);

	int maxLoop = this->conlistsners->size();
	for(int i = 0; i != maxLoop; ++i){
		IPubsubConnectionListener* listner = this->conlistsners->get(i);
		listner->onHandshakeEnds(handshake);
	}
}

void P2pServerConnectionManager::fireLoginHandshake(P2pHandshake *handshake, const LoginPubSubCommand *loginCommand, const UnicodeString* canonicalName) {
	StackUnlocker __lock(this->listnersMutex, __FILE__, __LINE__);

	int maxLoop = this->conlistsners->size();
	for(int i = 0; i != maxLoop; ++i){
		IPubsubConnectionListener* listner = this->conlistsners->get(i);
		listner->onLoginHandshake(handshake, loginCommand, canonicalName);
	}
}

void P2pServerConnectionManager::fireClientLoginHandshake(P2pHandshake *handshake, const LoginClientCommand *clientLoginCommand) {
	StackUnlocker __lock(this->listnersMutex, __FILE__, __LINE__);

	int maxLoop = this->conlistsners->size();
	for(int i = 0; i != maxLoop; ++i){
		IPubsubConnectionListener* listner = this->conlistsners->get(i);
		listner->onLoginClinentHandshake(handshake, clientLoginCommand);
	}
}

} /* namespace codablecash */
