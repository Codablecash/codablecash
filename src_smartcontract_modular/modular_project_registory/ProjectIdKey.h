/*
 * ProjectIdKey.h
 *
 *  Created on: Nov 27, 2025
 *      Author: iizuka
 */

#ifndef MODULAR_PROJECT_REGISTORY_PROJECTIDKEY_H_
#define MODULAR_PROJECT_REGISTORY_PROJECTIDKEY_H_

#include "btree/AbstractBtreeKey.h"

using namespace alinous;

namespace codablecash {

class SmartcontractProjectId;

class ProjectIdKey : public AbstractBtreeKey {
public:
	ProjectIdKey();
	virtual ~ProjectIdKey();

	virtual bool isInfinity() const { return false; }
	virtual bool isNull() const { return false; }

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static ProjectIdKey* fromBinary(ByteBuffer* in);

	virtual int compareTo(const AbstractBtreeKey* key) const noexcept;
	virtual AbstractBtreeKey* clone() const noexcept;

	void setProjectId(const SmartcontractProjectId* id);
	const SmartcontractProjectId* getProjectId() const noexcept {
		return this->projectId;
	}

private:
	SmartcontractProjectId* projectId;
};

} /* namespace codablecash */

#endif /* MODULAR_PROJECT_REGISTORY_PROJECTIDKEY_H_ */
