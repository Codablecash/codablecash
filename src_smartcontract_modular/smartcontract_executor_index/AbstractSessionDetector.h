/*
 * AbstractSessionDetector.h
 *
 *  Created on: Mar 18, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_INDEX_ABSTRACTSESSIONDETECTOR_H_
#define SMARTCONTRACT_EXECUTOR_INDEX_ABSTRACTSESSIONDETECTOR_H_

#include "smartcontract_executor_index/IInstanceSessionContextListner.h"

#include "base/ArrayList.h"

#include "trx/session/base/CdbDatabaseSessionId.h"

using namespace alinous;

namespace codablecash {

class CdbDatabaseSessionId;
class SmartcontractExecContextRegistory;
class SmartcontractExecContextData;

class AbstractSessionDetector : public IInstanceSessionContextListner {
public:
	explicit AbstractSessionDetector(SmartcontractExecContextRegistory* contextRegistory);
	virtual ~AbstractSessionDetector();

	virtual void visit(const InstanceSessionContext* context);

protected:
	void registerLastId(const CdbDatabaseSessionId* lastTrxId);
	bool hasLastId(const CdbDatabaseSessionId* trxId);

	virtual void registerHead(const InstanceSessionContext* context, const SmartcontractExecContextData* data) = 0;

protected:
	ArrayList<CdbDatabaseSessionId, CdbDatabaseSessionId::ValueCompare>* lastIdsList;

	SmartcontractExecContextRegistory* contextRegistory;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_INDEX_ABSTRACTSESSIONDETECTOR_H_ */
