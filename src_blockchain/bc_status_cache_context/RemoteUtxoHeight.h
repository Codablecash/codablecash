/*
 * RemoteUtxoHeight.h
 *
 *  Created on: Jul 9, 2026
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_CONTEXT_REMOTEUTXOHEIGHT_H_
#define BC_STATUS_CACHE_CONTEXT_REMOTEUTXOHEIGHT_H_
#include <cstdint>

namespace codablecash {

class UtxoId;

class RemoteUtxoHeight {
public:
	RemoteUtxoHeight(const RemoteUtxoHeight& inst);
	RemoteUtxoHeight(uint64_t height, const UtxoId* utxoId);
	virtual ~RemoteUtxoHeight();

	uint64_t getHeight() const noexcept {
		return this->height;
	}
	UtxoId* getUtxoId() const noexcept {
		return this->utxoId;
	}

private:
	uint64_t height;
	UtxoId* utxoId;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_CONTEXT_REMOTEUTXOHEIGHT_H_ */
