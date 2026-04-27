/*
 * ProjectId.h
 *
 *  Created on: Nov 27, 2025
 *      Author: iizuka
 */

#ifndef MODULAR_PROJECT_SMARTCONTRACTPROJECTID_H_
#define MODULAR_PROJECT_SMARTCONTRACTPROJECTID_H_

#include "bc_base/Abstract32BytesId.h"

namespace codablecash {

class SmartcontractProjectId : public Abstract32BytesId {
public:
	SmartcontractProjectId(const SmartcontractProjectId& inst);
	SmartcontractProjectId();
	SmartcontractProjectId(const char *binary, int length);
	virtual ~SmartcontractProjectId();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static SmartcontractProjectId* fromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;
};

} /* namespace codablecash */

#endif /* MODULAR_PROJECT_SMARTCONTRACTPROJECTID_H_ */
