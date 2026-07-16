/*
 * RemoteUtxoDetector.h
 *
 *  Created on: Jul 9, 2026
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_CONTEXT_REMOTEUTXODETECTOR_H_
#define BC_STATUS_CACHE_CONTEXT_REMOTEUTXODETECTOR_H_

#include "base/ArrayList.h"
#include <cstdint>

using namespace alinous;

namespace codablecash {

class RemoteUtxoRepository;
class RemoteUtxoHeight;
class UtxoId;
class CodablecashSystemParam;
class BlockchainSoftwareVersion;


class RemoteUtxoDetector {
public:
	RemoteUtxoDetector(RemoteUtxoRepository* finalizedUtxoRepo, uint64_t finalizedHeight, const CodablecashSystemParam* config, const BlockchainSoftwareVersion* version);
	virtual ~RemoteUtxoDetector();

	ArrayList<RemoteUtxoHeight>* getList() const noexcept {
		return this->list;
	}

	void add(const RemoteUtxoHeight* utxo) noexcept;

	bool isRemoteUtxoUsed(const UtxoId* utxoId) const noexcept;
	void consumeRemoteUtxo(const UtxoId* utxoId, uint64_t height);

private:
	bool hasUtxo(const UtxoId* utxoId) const noexcept;

private:
	uint64_t expiredHeight;

	RemoteUtxoRepository* finalizedUtxoRepo;

	ArrayList<RemoteUtxoHeight>* list;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_CONTEXT_REMOTEUTXODETECTOR_H_ */
