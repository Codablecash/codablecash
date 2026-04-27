/*
 * CdlDatabaseSessionId.h
 *
 *  Created on: 2023/07/08
 *      Author: iizuka
 */

#ifndef TRX_SESSION_BASE_CDBDATABASESESSIONID_H_
#define TRX_SESSION_BASE_CDBDATABASESESSIONID_H_

#include "bc_base/Abstract32BytesId.h"

namespace codablecash {

class CdbDatabaseSessionId : public Abstract32BytesId {
public:
	CdbDatabaseSessionId(const CdbDatabaseSessionId& inst);
	CdbDatabaseSessionId();
	CdbDatabaseSessionId(const char *binary, int length);
	virtual ~CdbDatabaseSessionId();

	static CdbDatabaseSessionId* createRandomId();
	static CdbDatabaseSessionId* from32BytesId(const Abstract32BytesId* id);

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static CdbDatabaseSessionId* createFromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;

};

} /* namespace codablecash */

#endif /* TRX_SESSION_BASE_CDBDATABASESESSIONID_H_ */
