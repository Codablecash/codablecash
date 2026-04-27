/*
 * SmartcontractCallSequenceBuilder.h
 *
 *  Created on: Mar 23, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_CURSOR_SMARTCONTRACTCALLSEQUENCEBUILDER_H_
#define SMARTCONTRACT_EXECUTOR_CURSOR_SMARTCONTRACTCALLSEQUENCEBUILDER_H_

namespace codablecash {

class SmartcontractCallSequence;
class CdbDatabaseSessionId;
class SmartcontractExecContextRegistory;


class SmartcontractCallSequenceBuilder {
public:
	explicit SmartcontractCallSequenceBuilder(SmartcontractExecContextRegistory* contextRegistory);
	virtual ~SmartcontractCallSequenceBuilder();

	void build(const CdbDatabaseSessionId* trxId);

	const SmartcontractCallSequence* getSequence() const noexcept {
		return this->sequence;
	}

private:
	SmartcontractExecContextRegistory* contextRegistory;
	SmartcontractCallSequence* sequence;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_CURSOR_SMARTCONTRACTCALLSEQUENCEBUILDER_H_ */
