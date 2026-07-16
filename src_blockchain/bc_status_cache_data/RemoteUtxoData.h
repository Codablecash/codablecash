/*
 * RemoteUtxoData.h
 *
 *  Created on: Jul 7, 2026
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_DATA_REMOTEUTXODATA_H_
#define BC_STATUS_CACHE_DATA_REMOTEUTXODATA_H_

#include "filestore_block/IBlockObject.h"

using namespace alinous;

namespace codablecash {

class UtxoId;


class RemoteUtxoData : public IBlockObject {
public:
	RemoteUtxoData(const RemoteUtxoData& inst);
	RemoteUtxoData(const UtxoId* id, uint64_t height);
	virtual ~RemoteUtxoData();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static RemoteUtxoData* createFromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;

	uint64_t getHeight() const noexcept {
		return this->height;
	}

private:
	UtxoId* utxoId;
	uint64_t height;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_DATA_REMOTEUTXODATA_H_ */
