/*
 * VoteManager.h
 *
 *  Created on: 2024/04/29
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_VOTE_VOTEMANAGER_H_
#define BC_STATUS_CACHE_VOTE_VOTEMANAGER_H_
#include <cstdint>

namespace alinous {
class File;
class SysMutex;
class DiskCacheManager;
class Btree;
class SystemTimestamp;
}
using namespace alinous;

namespace codablecash {

class CodablecashSystemParam;
class BlockHeader;
class HeightVoteData;


class VoteManager {
public:
	static const constexpr wchar_t* VOTE_HEIGHT_DATA{L"vote_data"};

	VoteManager(const File* baseDir, const CodablecashSystemParam* config);
	virtual ~VoteManager();

	void initBlank();

	void open();
	void close() noexcept;

	bool registerBlockHeader(const BlockHeader* header, const CodablecashSystemParam* param);

	SystemTimestamp* getPosVoteLimit(uint64_t lastHeight);

private:
	HeightVoteData* getHeightVoteData(uint64_t height);
	bool updateLimitTime(uint64_t height, HeightVoteData* data, const SystemTimestamp* limit);

private:
	File* baseDir;
	SysMutex* mutex;

	DiskCacheManager* cacheManager;
	Btree* heightVoteDataIndex;

	const CodablecashSystemParam* config;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_VOTE_VOTEMANAGER_H_ */
