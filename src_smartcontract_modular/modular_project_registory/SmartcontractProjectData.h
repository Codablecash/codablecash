/*
 * SmartcontractProjectData.h
 *
 *  Created on: Nov 27, 2025
 *      Author: iizuka
 */

#ifndef MODULAR_PROJECT_REGISTORY_SMARTCONTRACTPROJECTDATA_H_
#define MODULAR_PROJECT_REGISTORY_SMARTCONTRACTPROJECTDATA_H_

#include "filestore_block/IBlockObject.h"

using namespace alinous;

namespace codablecash {

class ProjectIdKey;
class ModularSmartcontractInstance;
class SoftwareVersion;

class SmartcontractProjectData : public IBlockObject {
public:
	SmartcontractProjectData(const SmartcontractProjectData& inst);
	SmartcontractProjectData();
	virtual ~SmartcontractProjectData();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static SmartcontractProjectData* createFromBinary(ByteBuffer *in);

	void setData(const ByteBuffer* data);

	void setKey(const ProjectIdKey* key);
	const ProjectIdKey* getKey() const noexcept {
		return this->key;
	}

	void setVersion(const SoftwareVersion* version);
	const SoftwareVersion* getVersion() const noexcept {
		return this->version;
	}

	virtual IBlockObject* copyData() const noexcept;

	ModularSmartcontractInstance* toSmartcontractInstance() const;

private:
	ByteBuffer* projectData;
	ProjectIdKey* key;
	SoftwareVersion* version;
};

} /* namespace codablecash */

#endif /* MODULAR_PROJECT_REGISTORY_SMARTCONTRACTPROJECTDATA_H_ */
