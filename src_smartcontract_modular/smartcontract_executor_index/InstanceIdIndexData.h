/*
 * InstanceIdIndexData.h
 *
 *  Created on: Nov 28, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_INDEX_INSTANCEIDINDEXDATA_H_
#define SMARTCONTRACT_EXECUTOR_INDEX_INSTANCEIDINDEXDATA_H_

#include "filestore_block/IBlockObject.h"

#include "base/ArrayList.h"

using namespace alinous;

namespace codablecash {

class CdbDatabaseSessionId;
class InstanceSessionContext;
class SessionContextReverseCompare;

class InstanceIdIndexData : public IBlockObject {
public:
	InstanceIdIndexData(const InstanceIdIndexData& inst);
	InstanceIdIndexData();

	virtual ~InstanceIdIndexData();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static InstanceIdIndexData* createFromBinary(ByteBuffer *in);

	virtual IBlockObject* copyData() const noexcept;

	void addSessionContext(InstanceSessionContext* sessionId);
	void sort();
	void removeSessionId(const CdbDatabaseSessionId* sessionId);

	int size() const noexcept;
	const InstanceSessionContext* get(int i) const noexcept;

	bool isEmpty() const noexcept;

private:
	ArrayList<InstanceSessionContext, SessionContextReverseCompare>* trxIdList; // = SessionId
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_INDEX_INSTANCEIDINDEXDATA_H_ */
