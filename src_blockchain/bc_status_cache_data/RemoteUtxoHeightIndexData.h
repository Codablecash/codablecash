/*
 * RemoteUtxoHeightIndexData.h
 *
 *  Created on: Jul 8, 2026
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_DATA_REMOTEUTXOHEIGHTINDEXDATA_H_
#define BC_STATUS_CACHE_DATA_REMOTEUTXOHEIGHTINDEXDATA_H_

#include "base/ArrayList.h"

#include "filestore_block/IBlockObject.h"

using namespace alinous;

namespace codablecash {

class UtxoId;

class RemoteUtxoHeightIndexData : public alinous::IBlockObject {
public:
	RemoteUtxoHeightIndexData(const RemoteUtxoHeightIndexData& inst);
	RemoteUtxoHeightIndexData();
	virtual ~RemoteUtxoHeightIndexData();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static RemoteUtxoHeightIndexData* createFromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;

	void join(const RemoteUtxoHeightIndexData* value) noexcept;
	bool contains(const UtxoId* utxoId) const noexcept;
	void remove(const UtxoId* utxoId) noexcept;
	int indexof(const UtxoId* utxoId) const noexcept;

	void add(const UtxoId* utxoId) noexcept;

	ArrayList<UtxoId>* getList() const noexcept {
		return this->list;
	}

private:
	ArrayList<UtxoId>* list;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_DATA_REMOTEUTXOHEIGHTINDEXDATA_H_ */
