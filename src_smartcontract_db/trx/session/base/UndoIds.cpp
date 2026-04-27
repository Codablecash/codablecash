/*
 * UndoIds.cpp
 *
 *  Created on: 2023/07/09
 *      Author: iizuka
 */

#include "trx/session/base/UndoIds.h"
#include "trx/session/base/CdbDatabaseSessionId.h"
#include "trx/session/base/CdbDatabaseSession.h"

#include "schema_table/schema/SchemaManager.h"

#include "snapshot/IdsBackupCommand.h"

#include "bc_base/BinaryUtils.h"

#include "trx/transaction/CdbTransactionManager.h"


namespace codablecash {

UndoIds::UndoIds(const UndoIds &inst) : AbstractUndoLog(AbstractUndoLog::UNDO_TYPE_UNDO_IDS) {
	this->lastSessionId = dynamic_cast<CdbDatabaseSessionId*>(inst.lastSessionId->copyData());

	this->maxTransactionId = inst.maxTransactionId;
	this->maxObjectId = inst.maxObjectId;
	this->maxSchemaObjectId = inst.maxSchemaObjectId;
	this->maxCommitId = inst.maxCommitId;
}

UndoIds::UndoIds() : AbstractUndoLog(AbstractUndoLog::UNDO_TYPE_UNDO_IDS){
	this->lastSessionId = nullptr;
	this->maxTransactionId = 0L;
	this->maxObjectId = 0L;
	this->maxSchemaObjectId = 0L;
	this->maxCommitId = 0L;
}

UndoIds::~UndoIds() {
	delete this->lastSessionId;
}

int UndoIds::binarySize() const {
	BinaryUtils::checkNotNull(this->lastSessionId);

	int total = sizeof(uint8_t);

	total += this->lastSessionId->binarySize();

	total += sizeof(this->maxTransactionId);
	total += sizeof(this->maxObjectId);
	total += sizeof(this->maxSchemaObjectId);
	total += sizeof(this->maxCommitId);

	return total;
}

void UndoIds::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->lastSessionId);

	out->put(this->type);

	this->lastSessionId->toBinary(out);

	out->putLong(this->maxTransactionId);
	out->putLong(this->maxObjectId);
	out->putLong(this->maxSchemaObjectId);
	out->putLong(this->maxCommitId);
}

void UndoIds::fromBinary(ByteBuffer *in) {
	this->lastSessionId = CdbDatabaseSessionId::createFromBinary(in);
	BinaryUtils::checkNotNull(this->lastSessionId);

	this->maxTransactionId = in->getLong();
	this->maxObjectId = in->getLong();
	this->maxSchemaObjectId = in->getLong();
	this->maxCommitId = in->getLong();
}

IBlockObject* UndoIds::copyData() const noexcept {
	return new UndoIds(*this);
}

void UndoIds::putCommand(const AbstractSnapshotCommand *command) {
	const IdsBackupCommand* backupCommand = dynamic_cast<const IdsBackupCommand*>(command);

	this->maxTransactionId = backupCommand->getMaxTransactionId();
	this->maxObjectId = backupCommand->getMaxObjectId();
	this->maxSchemaObjectId = backupCommand->getMaxSchemaObjectId();
	this->maxCommitId = backupCommand->getMaxCommitId();
}

void UndoIds::setLastSessionId(const CdbDatabaseSessionId *sessionId) noexcept {
	delete this->lastSessionId;
	this->lastSessionId = dynamic_cast<CdbDatabaseSessionId*>(sessionId->copyData());
}

void UndoIds::execute(CdbTransactionManager *trxManager, UndoLogPartsRepo* partsRepo, UndoLogsIdkey* key) {
	SchemaManager* schemaManager = trxManager->getSchemaManager();
	schemaManager->recoverObjectIds(this->maxTransactionId, this->maxObjectId, this->maxSchemaObjectId, this->maxCommitId);

	CdbDatabaseSession* session = trxManager->getCdbDatabaseSession();
	session->setSessionId(this->lastSessionId);
}

} /* namespace codablecash */
