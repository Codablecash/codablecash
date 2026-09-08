/*
 * NodeIdentifierSource.h
 *
 *  Created on: 2022/02/18
 *      Author: iizuka
 */

#ifndef BC_NETWORK_NODEIDENTIFIERSOURCE_H_
#define BC_NETWORK_NODEIDENTIFIERSOURCE_H_
#include <cstdint>

#include "filestore_block/IBlockObject.h"

namespace alinous {
class BigInteger;
}
using namespace alinous;

namespace codablecash {

class IKeyPair;
class NodeIdentifier;
class JsonObject;

class NodeIdentifierSource : public IBlockObject {
public:
	NodeIdentifierSource(IKeyPair* pair);
	virtual ~NodeIdentifierSource();

	static NodeIdentifierSource* create();

	NodeIdentifier toNodeIdentifier() const noexcept;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static NodeIdentifierSource* createFromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;

	const BigInteger* getSecretKey() const noexcept;

	JsonObject* toJsonObject() const;
	static NodeIdentifierSource* fromJson(const JsonObject* configObj);

private:
	IKeyPair* pair;
};

} /* namespace codablecash */

#endif /* BC_NETWORK_NODEIDENTIFIERSOURCE_H_ */
