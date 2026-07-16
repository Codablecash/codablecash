/*
 * RemoteUtxoHeightIndex.h
 *
 *  Created on: Jul 7, 2026
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_DATA_REMOTEUTXOHEIGHTINDEX_H_
#define BC_STATUS_CACHE_DATA_REMOTEUTXOHEIGHTINDEX_H_
#include <cstdint>

namespace alinous {
class File;
class DiskCacheManager;
class Btree;
class BtreeReverseScanner;
}
using namespace alinous;

namespace codablecash {

class UtxoId;

class RemoteUtxoHeightIndex {
public:
	static constexpr const wchar_t* HISTORY_BASE_DIR = L"heightindex";

	explicit RemoteUtxoHeightIndex(const File* baseDir);
	virtual ~RemoteUtxoHeightIndex();

	void initBlank();

	void open();
	void close();

	void addUtxo(uint64_t height, const UtxoId* utxoId);
	void remove(uint64_t height);

	BtreeReverseScanner* getReverseScanner();

private:
	File* baseDir;
	DiskCacheManager* cacheManager;

	Btree* btree;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_DATA_REMOTEUTXOHEIGHTINDEX_H_ */
