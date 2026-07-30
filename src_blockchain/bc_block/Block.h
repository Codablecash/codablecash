/*
 * Block.h
 *
 *  Created on: 2023/03/17
 *      Author: iizuka
 */

#ifndef BC_BLOCK_BLOCK_H_
#define BC_BLOCK_BLOCK_H_

#include <cstdint>

namespace alinous {
class ByteBuffer;
class SystemTimestamp;
}
using namespace alinous;

namespace codablecash {

class BlockHeader;
class BlockBody;
class AbstractBlockchainTransaction;
class AbstractBalanceTransaction;
class AbstractControlTransaction;
class AbstractInterChainCommunicationTansaction;
class AbstractSmartcontractTransaction;
class BlockHeaderId;
class PoWNonce;
class VoteBlockTransaction;
class ReservedVotes;
class AbstractBlockHeaderCommand;


class Block {
public:
	Block(const Block& inst);
	Block(uint16_t zone, uint64_t height);
	Block(BlockHeader* header, BlockBody* body);
	virtual ~Block();

	void addBalanceTransaction(const AbstractBalanceTransaction* trx) noexcept;
	void addControlTransaction(const AbstractControlTransaction* trx) noexcept;
	void addInterChainCommunicationTransaction(const AbstractInterChainCommunicationTansaction* trx) noexcept;
	void addSmartcontractTransaction(const AbstractSmartcontractTransaction* trx) noexcept;

	void build();

	uint16_t getZone() const noexcept;
	uint64_t getHeight() const noexcept;

	BlockBody* getBody() const noexcept {
		return body;
	}
	BlockHeader* getHeader() const noexcept {
		return header;
	}

	void setLastHeaderId(const BlockHeaderId *lastHeaderId) noexcept;
	void setPoWNonce(const PoWNonce *nonce) noexcept;

	bool isFinalizing(int votePerBlock) const noexcept;

	int binarySize() const;
	void toBinary(ByteBuffer* buff) const;
	static Block* createFromBinary(ByteBuffer* buff);

	void setGenesisTimestamp();

	const SystemTimestamp* getTimestamp() const noexcept;
	void setNonceGeneratedTimestamp(const SystemTimestamp* tm) const noexcept;

	bool isScheduledBlock() const noexcept;

	void addHeaderCommand(const AbstractBlockHeaderCommand *cmd);

#ifdef __DEBUG__
	bool checkMerkleRoot() const noexcept;
#endif

private:
	void addVote(const VoteBlockTransaction* votetrx);

private:
	BlockHeader* header;
	BlockBody* body;
};

} /* namespace codablecash */

#endif /* BC_BLOCK_BLOCK_H_ */
