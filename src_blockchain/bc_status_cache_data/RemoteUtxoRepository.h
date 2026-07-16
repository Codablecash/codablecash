/*
 * RemoteUtxoRepository.h
 *
 *  Created on: Jul 7, 2026
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_DATA_REMOTEUTXOREPOSITORY_H_
#define BC_STATUS_CACHE_DATA_REMOTEUTXOREPOSITORY_H_
#include <cstdint>

namespace alinous {
class File;
}
using namespace alinous;

namespace codablecash {

class UtxoId;

class RemoteUtxoHeightIndex;
class RemoteUtxoHistory;
class RemoteUtxoData;

class RemoteUtxoRepository {
public:
	static constexpr const wchar_t* REMOTE_UTXO_ID = L"utxoid";

	explicit RemoteUtxoRepository(const File* baseDir);
	virtual ~RemoteUtxoRepository();

	void initBlank();

	void open();
	void close();

	void addUtxo(uint64_t height, const UtxoId* utxoId) noexcept;
	RemoteUtxoData* getUtxo(const UtxoId* utxoId);

	void clean(uint64_t cleanHeight);

private:
	File* baseDir;

	RemoteUtxoHistory* utxoHistory;
	RemoteUtxoHeightIndex* heightIndex;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_DATA_REMOTEUTXOREPOSITORY_H_ */
