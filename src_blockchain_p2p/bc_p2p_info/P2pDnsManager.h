/*
 * P2pDnsManager.h
 *
 *  Created on: 2023/09/06
 *      Author: iizuka
 */

#ifndef BC_P2P_INFO_P2PDNSMANAGER_H_
#define BC_P2P_INFO_P2PDNSMANAGER_H_

#include "base/ArrayList.h"

#include <cstdint>

namespace alinous {
class File;
class SysMutex;
}
using namespace alinous;

namespace codablecash {

class P2pNodeDatabase;
class P2pNodeRecord;
class ISystemLogger;
class NodeIdentifier;
class NodeNetworkInfo;

class P2pDnsManager {
public:
	explicit P2pDnsManager(const File* baseDir);
	virtual ~P2pDnsManager();

	void createBlankDatabase();
	void open();
	void close();

	void importSeeds(int numZone, const P2pNodeRecord* nodeRecord, ISystemLogger* logger);

	ArrayList<P2pNodeRecord>* getZoneRecords(uint16_t zone, int maxNum);

	bool removeRecord(const NodeIdentifier* nodeId);

	void updateOrAddRecord(const NodeNetworkInfo *nodeNetInfo, ISystemLogger* logger);

private:
	void doImportSeeds(uint16_t zone, const UnicodeString* host, int port, ISystemLogger *logger);

private:
	P2pNodeDatabase* database;
	SysMutex* mutex;
};

} /* namespace codablecash */

#endif /* BC_P2P_INFO_P2PDNSMANAGER_H_ */
