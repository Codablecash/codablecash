/*
 * BlochchainP2pManager.h
 *
 *  Created on: 2023/04/05
 *      Author: iizuka
 */

#ifndef BC_P2P_BLOCHCHAINP2PMANAGER_H_
#define BC_P2P_BLOCHCHAINP2PMANAGER_H_

#include "pubsub/IPubsubConnectionListener.h"

#include "base/ArrayList.h"
#include "base/HashMap.h"

namespace alinous {
class SysMutex;
}
using namespace alinous;

namespace codablecash {

class P2pZone;
class PubSubId;
class BlockchainNodeHandshake;
class ClientNodeHandshake;
class NodeIdentifier;
class AbstractNodeCommand;
class P2pRequestProcessor;
class AbstractClientNotifyCommand;
class AbstractConsensusNodeCommand;

class BlochchainP2pManager : public IPubsubConnectionListener {
public:
	BlochchainP2pManager();
	virtual ~BlochchainP2pManager();

	void init(int numZones);
	void incNumZones(uint16_t newZone);

	void resetZones() noexcept;

	virtual void onLoginHandshake(P2pHandshake *handshake, const LoginPubSubCommand *loginCommand, const UnicodeString* canonicalName);
	virtual void onLoginClinentHandshake(P2pHandshake *handshake, const LoginClientCommand *clientLoginCommand);
	virtual void onHandshakeEnds(P2pHandshake *handshake);

	void registerHandshake(uint16_t zone, P2pHandshake *handshake, const NodeIdentifier* nodeId, const UnicodeString* canonicalName);

	void disconnect(const NodeIdentifier *nodeId);

	BlockchainNodeHandshake* getNodeHandshake(const PubSubId* pubsubId) const noexcept;
	ClientNodeHandshake* getClientHandshake(const PubSubId* pubsubId) const noexcept;

	BlockchainNodeHandshake* getNodeHandshakeByNodeId(const NodeIdentifier* nodeId) const noexcept;
	ClientNodeHandshake* getClientHandshakeByNodeId(const NodeIdentifier* nodeId) const noexcept;

	void broadCastWithinZone(uint16_t zoneSelf, const AbstractNodeCommand* command, P2pRequestProcessor* processor);
	void bloadCastWithinZone(uint16_t zoneSelf, const NodeIdentifier* excludeNodeId, const AbstractNodeCommand* command, P2pRequestProcessor* processor);

	void broadCastAllZones(const NodeIdentifier* excludeNodeId, const AbstractNodeCommand* command, P2pRequestProcessor* processor);

	void bloadCastHighPriorityWithinZone(uint16_t zoneSelf, const ArrayList<NodeIdentifier>* excludeNodeIds, const AbstractConsensusNodeCommand* command, P2pRequestProcessor* processor);
	void bloadCastHighPriorityAllZones(const ArrayList<NodeIdentifier>* excludeNodeIds, const AbstractConsensusNodeCommand* command, P2pRequestProcessor* processor);

	void broadCastToClients(AbstractClientNotifyCommand* commnad, P2pRequestProcessor* processor);

	int getNumNodes(uint16_t zone) const;
	ArrayList<NodeIdentifier>* getNodeIds(uint16_t zone) const;

	void endCommunication() noexcept;

	int getProtocol() const noexcept {
		return this->protocol;
	}
	void setProtocol(int protocol) noexcept {
		this->protocol = protocol;
	}
	const UnicodeString* getHost() const noexcept {
		return this->host;
	}
	void setHost(const UnicodeString* host) noexcept;
	void setPort(int port) noexcept {
		this->port = port;
	}
	int getPort() const noexcept {
		return this->port;
	}

private:
	void __removeHandshake(const PubSubId* pubsubId);
	void __broadCastWithinZone(uint16_t zone, const NodeIdentifier* excludeNodeId, const AbstractNodeCommand* command, P2pRequestProcessor* processor);

private:
	SysMutex* mutex;
	int numZones;
	ArrayList<P2pZone>* zones;

	HashMap<PubSubId, BlockchainNodeHandshake> blockchainHandshakeHash;
	HashMap<PubSubId, ClientNodeHandshake> clientHandshakeHash;

	// self information defined at P2pNodeRecord
	int protocol;
	UnicodeString* host;
	uint32_t port;

	bool end;
};

} /* namespace codablecash */

#endif /* BC_P2P_BLOCHCHAINP2PMANAGER_H_ */
