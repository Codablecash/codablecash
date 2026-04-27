/*
 * ProjectIndexData.h
 *
 *  Created on: Jan 29, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_INDEX_PROJECTID_PROJECTINDEXDATA_H_
#define SMARTCONTRACT_EXECUTOR_INDEX_PROJECTID_PROJECTINDEXDATA_H_

#include "filestore_block/IBlockObject.h"

using namespace alinous;

namespace codablecash {

class SmartcontractProjectId;

class ProjectIndexData : public IBlockObject {
public:
	ProjectIndexData(const ProjectIndexData& inst);
	ProjectIndexData();
	virtual ~ProjectIndexData();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static ProjectIndexData* createFromBinary(ByteBuffer *in);

	virtual IBlockObject* copyData() const noexcept;

	void setProjectId(const SmartcontractProjectId* id);
	const SmartcontractProjectId* getProjectId() const noexcept {
		return this->projectId;
	}

private:
	SmartcontractProjectId* projectId;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_INDEX_PROJECTID_PROJECTINDEXDATA_H_ */
