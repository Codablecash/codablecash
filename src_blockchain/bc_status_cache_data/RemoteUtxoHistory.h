/*
 * RemoteUtxoHistory.h
 *
 *  Created on: Jul 7, 2026
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_DATA_REMOTEUTXOHISTORY_H_
#define BC_STATUS_CACHE_DATA_REMOTEUTXOHISTORY_H_
#include <cstdint>

namespace alinous {
class File;
class DiskCacheManager;
class Btree;
}
using namespace alinous;

namespace codablecash {

class UtxoId;
class RemoteUtxoData;

class RemoteUtxoHistory {
public:
	static constexpr const wchar_t* HISTORY_FILE = L"utxohistory";

	explicit RemoteUtxoHistory(const File* baseDir);
	virtual ~RemoteUtxoHistory();

	void initBlank();

	void open();
	void close();

	void add(const UtxoId* utxoId, uint64_t height);
	void remove(const UtxoId* utxoId);
	RemoteUtxoData* getData(const UtxoId* utxoId);

private:
	File* baseDir;
	DiskCacheManager* cacheManager;

	Btree* btree;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_DATA_REMOTEUTXOHISTORY_H_ */
