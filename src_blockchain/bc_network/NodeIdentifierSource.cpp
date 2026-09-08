/*
 * NodeIdentifierSource.cpp
 *
 *  Created on: 2022/02/18
 *      Author: iizuka
 */

#include "bc_network/NodeIdentifierSource.h"
#include "bc_network/NodeIdentifier.h"

#include "crypto/Schnorr.h"

#include "base_io/ByteBuffer.h"

#include "base/StackRelease.h"

#include "json_object/JsonObject.h"

#include "json_object/JsonValuePair.h"

#include "json_object/JsonStringValue.h"

#include "bc/ExceptionThrower.h"

#include "sim/SimnetConfigException.h"

#include "crypto/SchnorrKeyPair.h"
namespace codablecash {

NodeIdentifierSource::NodeIdentifierSource(IKeyPair* pair) {
	this->pair = pair;
}

NodeIdentifierSource::~NodeIdentifierSource() {
	delete this->pair;
}

NodeIdentifierSource* NodeIdentifierSource::create() {
	IKeyPair* pair = Schnorr::generateKey();

	NodeIdentifierSource* src = new NodeIdentifierSource(pair);
	return src;
}

NodeIdentifier NodeIdentifierSource::toNodeIdentifier() const noexcept {
	const BigInteger* pubkey = this->pair->getPubKey();

	return NodeIdentifier(pubkey);
}

int NodeIdentifierSource::binarySize() const {
	int total = this->pair->binarySize();

	return total;
}

void NodeIdentifierSource::toBinary(ByteBuffer* out) const {
	this->pair->toBinary(out);
}

NodeIdentifierSource* NodeIdentifierSource::createFromBinary(ByteBuffer* in) {
	IKeyPair* pair = IKeyPair::createFromBinary(in);

	return new NodeIdentifierSource(pair);
}

IBlockObject* NodeIdentifierSource::copyData() const noexcept {
	return new NodeIdentifierSource(this->pair->clone());
}

const BigInteger* NodeIdentifierSource::getSecretKey() const noexcept {
	return this->pair->getSecretKey();
}

JsonObject* NodeIdentifierSource::toJsonObject() const {
	JsonObject* configObj = new JsonObject(); __STP(configObj);

	// pubkey
	{
		const BigInteger* pub = this->pair->getPubKey();

		JsonValuePair* pair = new JsonValuePair();
		pair->setKey(new JsonStringValue(L"pubkey"));

		UnicodeString str = pub->toString(16);
		pair->setValue(new JsonStringValue(&str));

		configObj->add(pair);
	}
	// secretkey
	{
		const BigInteger* sec = this->pair->getSecretKey();

		JsonValuePair* pair = new JsonValuePair();
		pair->setKey(new JsonStringValue(L"secretkey"));

		UnicodeString str = sec->toString(16);
		pair->setValue(new JsonStringValue(&str));

		configObj->add(pair);
	}

	return __STP_MV(configObj);
}

NodeIdentifierSource* NodeIdentifierSource::fromJson(const JsonObject *configObj) {
	NodeIdentifierSource* ret = nullptr;

	const JsonValuePair* pubPair = configObj->get(L"pubkey");
	const JsonValuePair* secPair = configObj->get(L"secretkey");

	ExceptionThrower<SimnetConfigException>::throwExceptionIfCondition(pubPair == nullptr || secPair == nullptr, L"secretkey and publickey is essential.", __FILE__, __LINE__);

	if(pubPair != nullptr && secPair != nullptr){
		AbstractJsonObject* pv = pubPair->getValue();
		JsonStringValue* pvalue = dynamic_cast<JsonStringValue*>(pv);
		ExceptionThrower<SimnetConfigException>::throwExceptionIfCondition(pvalue == nullptr, L"pubkey of the voter must be string type.", __FILE__, __LINE__);

		AbstractJsonObject* sv = secPair->getValue();
		JsonStringValue* svalue = dynamic_cast<JsonStringValue*>(sv);
		ExceptionThrower<SimnetConfigException>::throwExceptionIfCondition(svalue == nullptr, L"secretkey of the voter must be string type.", __FILE__, __LINE__);

		BigInteger p(pvalue->getValue(), 16);
		BigInteger s(svalue->getValue(), 16);

		SchnorrKeyPair* sch = new SchnorrKeyPair(s, p);
		ret = new NodeIdentifierSource(sch);
	}

	return ret;
}

} /* namespace codablecash */
