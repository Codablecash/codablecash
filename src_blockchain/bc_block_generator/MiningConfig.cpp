/*
 * MiningConfig.cpp
 *
 *  Created on: 2023/03/29
 *      Author: iizuka
 */

#include "bc_block_generator/MiningConfig.h"

#include "bc_base/AddressDescriptor.h"

#include "pow/PoWCalculator.h"

#include "pow_pool/NetworkPoWPool.h"

#include "base/UnicodeString.h"

#include "ipconnect/IServerSocket.h"

#include "bc_p2p_info/P2pNodeRecord.h"
namespace codablecash {

MiningConfig::MiningConfig(const MiningConfig &inst) {
	this->addressDesc = inst.addressDesc != nullptr ? dynamic_cast<AddressDescriptor*>(inst.addressDesc->copyData()) : nullptr;
	this->calcType = inst.calcType;

	this->protocol = inst.protocol;
	this->srvhost = inst.srvhost != nullptr ? new UnicodeString(inst.srvhost) : nullptr;
	this->port = inst.port;
}

MiningConfig::MiningConfig() {
	this->addressDesc = nullptr;
	this->calcType = MiningConfig::TYPE_DEBUG;

	this->protocol = P2pNodeRecord::TCP_IP_V6;
	this->srvhost = nullptr;
	this->port = 18080;
}

MiningConfig::~MiningConfig() {
	delete this->addressDesc;
	delete this->srvhost;
}

void MiningConfig::setAddressDescriptor(const AddressDescriptor *addressDesc) noexcept {
	delete this->addressDesc;
	this->addressDesc = dynamic_cast<AddressDescriptor*>(addressDesc->copyData());
}

AbstractPoWCalculator* MiningConfig::getPoWCalculator(PoWManager* powManager) const noexcept {
	AbstractPoWCalculator* inst = nullptr;
	if(this->calcType == TYPE_POOL){
		inst = new NetworkPoWPool(powManager);
		initNetworkPool(dynamic_cast<NetworkPoWPool*>(inst));
	}else{
		inst = new PoWCalculator();
	}

	return inst;
}

void MiningConfig::setCalclatorType(int type) noexcept {
	this->calcType = type;
}

void MiningConfig::setProtocol(int type) {
	this->protocol = type;
}

void MiningConfig::setServerHost(const UnicodeString *host) {
	this->srvhost = new UnicodeString(host);
}

void MiningConfig::setPort(int port) {
	this->port = port;
}

void MiningConfig::initNetworkPool(NetworkPoWPool *pool) const {
	pool->setProtocol(this->protocol);
	pool->setServerHost(this->srvhost);
	pool->setPort(this->port);

}

} /* namespace codablecash */
