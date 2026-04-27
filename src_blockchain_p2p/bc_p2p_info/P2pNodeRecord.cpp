/*
 * P2pNodeRecord.cpp
 *
 *  Created on: 2023/08/11
 *      Author: iizuka
 */

#include "bc_p2p_info/P2pNodeRecord.h"
#include "bc_p2p_info/P2pNodeRecordReference.h"

#include "bc_network/NodeIdentifier.h"

#include "osenv/funcs.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "bc_base/BinaryUtils.h"

#include "bc_p2p_info/P2PZoneConnectionInfo.h"

#include "numeric/BigInteger.h"


namespace codablecash {

P2pNodeRecord::P2pNodeRecord(const P2pNodeRecord &inst) {
	this->zone = inst.zone;
	this->nodeId = inst.nodeId != nullptr ? dynamic_cast<NodeIdentifier*>(inst.nodeId->copyData()) : nullptr;
	this->canonicalName = inst.canonicalName != nullptr ? new UnicodeString(inst.canonicalName) : nullptr;
	this->protocol = inst.protocol;
	this->host = inst.host != nullptr ? new UnicodeString(inst.host) : nullptr;
	this->port = inst.port;

	this->lastUpdatedTime = inst.lastUpdatedTime;
	this->createdTime = inst.createdTime;

	this->list = new ArrayList<P2PZoneConnectionInfo>();

	int maxLoop = inst.list->size();
	for(int i = 0; i != maxLoop; ++i){
		P2PZoneConnectionInfo* ref = inst.list->get(i);

		this->list->addElement(new P2PZoneConnectionInfo(*ref));
	}
}

P2pNodeRecord::P2pNodeRecord() {
	this->zone = 0;
	this->nodeId = nullptr;
	this->canonicalName = nullptr;
	this->protocol = TCP_IP_V6;
	this->host = nullptr;
	this->port = 0;

	this->lastUpdatedTime = Os::getTimestampLong();
	this->createdTime = this->lastUpdatedTime;

	this->list = new ArrayList<P2PZoneConnectionInfo>();
}

P2pNodeRecord* P2pNodeRecord::createIpV6Record(uint16_t zone, const NodeIdentifier *nodeId, const UnicodeString* canonicalName, const wchar_t* host, int port) {
	UnicodeString hostStr(host);

	return createIpV6Record(zone, nodeId, canonicalName, &hostStr, port);
}

P2pNodeRecord* P2pNodeRecord::createIpV6Record(uint16_t zone, const NodeIdentifier *nodeId, const UnicodeString *canonicalName, const UnicodeString *host, int port) {
	P2pNodeRecord* inst = new P2pNodeRecord();

	inst->setNodeId(nodeId);
	inst->setCanonicalName(canonicalName);
	inst->setZone(zone);

	inst->setAddress(host, port);

	return inst;
}

P2pNodeRecord::~P2pNodeRecord() {
	this->list->deleteElements();
	delete this->list;

	delete this->nodeId;
	delete this->canonicalName;
	delete this->host;
}

int P2pNodeRecord::binarySize() const {
	BinaryUtils::checkNotNull(this->nodeId);
	BinaryUtils::checkNotNull(this->host);

	int total = sizeof(this->zone);

	total += this->nodeId->binarySize();

	total += sizeof(uint8_t); // bool
	if(this->canonicalName != nullptr){
		total += BinaryUtils::stringSize(this->canonicalName);
	}

	total += sizeof(int8_t); // protocol
	total += BinaryUtils::stringSize(this->host);
	total += sizeof(this->port);

	total += sizeof(this->lastUpdatedTime);
	total += sizeof(this->createdTime);

	int maxLoop = this->list->size();
	total += sizeof(uint16_t);

	for(int i = 0; i != maxLoop; ++i){
		P2PZoneConnectionInfo* ref = this->list->get(i);

		total += ref->binarySize();
	}

	return total;
}

void P2pNodeRecord::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->nodeId);
	BinaryUtils::checkNotNull(this->host);

	out->putShort(this->zone);
	this->nodeId->toBinary(out);

	bool cano = this->canonicalName != nullptr;
	out->put(cano ? 1 : 0);
	if(cano){
		BinaryUtils::putString(out, this->canonicalName);
	}

	out->put(this->protocol);
	BinaryUtils::putString(out, this->host);
	out->putInt(this->port);

	out->putLong(this->lastUpdatedTime);
	out->putLong(this->createdTime);

	int maxLoop = this->list->size();
	out->putShort(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		P2PZoneConnectionInfo* ref = this->list->get(i);

		ref->toBinary(out);
	}
}

P2pNodeRecord* P2pNodeRecord::fromBinary(ByteBuffer *in) {
	P2pNodeRecord* inst = new P2pNodeRecord(); __STP(inst);

	inst->zone = in->getShort();
	inst->nodeId = NodeIdentifier::fromBinary(in);
	BinaryUtils::checkNotNull(inst->nodeId);

	uint8_t cano = in->get();
	if(cano > 0){
		inst->canonicalName = BinaryUtils::getString(in);
	}

	inst->protocol = in->get();
	inst->host = BinaryUtils::getString(in);
	BinaryUtils::checkNotNull(inst->host);
	inst->port = in->getInt();

	inst->lastUpdatedTime = in->getLong();
	inst->createdTime = in->getLong();

	int maxLoop = in->getShort();
	for(int i = 0; i != maxLoop; ++i){
		P2PZoneConnectionInfo* ref = P2PZoneConnectionInfo::fromBinary(in); __STP(ref);
		BinaryUtils::checkNotNull(ref);

		inst->list->addElement(__STP_MV(ref));
	}

	return __STP_MV(inst);
}

IBlockObject* P2pNodeRecord::copyData() const noexcept {
	return new P2pNodeRecord(*this);
}

void P2pNodeRecord::setNodeId(const NodeIdentifier *nodeId) {
	delete this->nodeId;
	this->nodeId = new NodeIdentifier(*nodeId);
}

void P2pNodeRecord::setCanonicalName(const UnicodeString *cname) {
	delete this->canonicalName;
	this->canonicalName = cname != nullptr ? new UnicodeString(cname) : nullptr;
}

void P2pNodeRecord::setAddress(const UnicodeString *host, int port) {
	delete this->host;
	this->host = new UnicodeString(host);

	this->port = port;
}

void P2pNodeRecord::addConnectedNode(uint16_t zone, const NodeIdentifier *nodeId) {
	P2PZoneConnectionInfo* zoneInfo = getZoneInfo(zone);
	zoneInfo->addConnectedNode(nodeId);

	updateLastupdatedTime();
}

void P2pNodeRecord::removeConnectedNode(uint16_t zone, const NodeIdentifier *nodeId) {
	P2PZoneConnectionInfo* zoneInfo = getZoneInfo(zone);
	zoneInfo->removeConnectedNode(nodeId);

	updateLastupdatedTime();
}

void P2pNodeRecord::updateLastupdatedTime() noexcept {
	this->lastUpdatedTime = Os::getTimestampLong();
}

P2PZoneConnectionInfo* P2pNodeRecord::getZoneInfo(uint16_t zone) noexcept {
	if(this->list->size() <= zone){
		int maxLoop = zone + 1;
		for(int i = this->list->size(); i != maxLoop; ++i){
			this->list->addElement(new P2PZoneConnectionInfo());
		}
	}

	return this->list->get(zone);
}

UnicodeString* P2pNodeRecord::toString() const noexcept {
	UnicodeString* str = new UnicodeString(L""); __STP(str);

	str->append(L"@");
	str->append((int64_t)this->zone);

	{
		UnicodeString node = this->nodeId->getPublicKey()->toString(16);
		str->append(L"[");
		str->append(&node);
		str->append(L"]");

		if(this->canonicalName != nullptr){
			str->append(L"=");

			str->append(L" ");
		}
	}

	if(this->protocol == TCP_IP_V4){
		str->append(L"ipv4://");
	}else{
		str->append(L"ipv6://");
	}
	str->append(this->host);
	str->append(L":");
	str->append((int64_t)this->port);

	return __STP_MV(str);
}

void P2pNodeRecord::stripConnections() noexcept {
	this->list->deleteElements();
	this->list->reset();
}

} /* namespace codablecash */
