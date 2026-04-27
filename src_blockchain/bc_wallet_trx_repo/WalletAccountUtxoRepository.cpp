/*
 * WalletAccountUtxo.cpp
 *
 *  Created on: 2023/02/23
 *      Author: iizuka
 */

#include "bc_wallet_trx_repo/WalletAccountUtxoRepository.h"

#include "bc_base_utxo_index/AddressDescriptorUtxoDataFactory.h"
#include "bc_base_utxo_index/AddressDescriptorKeyFactory.h"
#include "bc_base_utxo_index/AddressDescriptorKey.h"
#include "bc_base_utxo_index/AddressDescriptorUtxoData.h"
#include "bc_base_utxo_index/AddressDescriptorDataFactory.h"
#include "bc_base_utxo_index/UtxoIdKeyFactory.h"
#include "bc_base_utxo_index/UtxoIdKey.h"
#include "bc_base_utxo_index/AddressDescriptorData.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "btree/BtreeConfig.h"
#include "btree/Btree.h"
#include "btree/BtreeScanner.h"

#include "random_access_file/DiskCacheManager.h"

#include "bc_trx_balance/BalanceUtxo.h"

#include "bc_trx/UtxoId.h"

namespace codablecash {

const UnicodeString WalletAccountUtxoRepository::FILE_NAME(L"utxos");
const UnicodeString WalletAccountUtxoRepository::UTXO_ID_INDEX_FILE_NAME(L"utxo_id_index");


WalletAccountUtxoRepository::WalletAccountUtxoRepository(const File* accountBaseDir) {
	this->accountBaseDir = accountBaseDir;
	this->cacheManager = new DiskCacheManager();
	this->btree = nullptr;
	this->utxoIdBtree = nullptr;
}

WalletAccountUtxoRepository::~WalletAccountUtxoRepository() {
	close();
	this->accountBaseDir = nullptr;
	delete this->cacheManager;
}

void WalletAccountUtxoRepository::init() {
	{
		UnicodeString fileName(WalletAccountUtxoRepository::FILE_NAME);

		AddressDescriptorKeyFactory* keyFactory = new AddressDescriptorKeyFactory(); __STP(keyFactory);
		AddressDescriptorUtxoDataFactory* dataFactory = new AddressDescriptorUtxoDataFactory(); __STP(dataFactory);

		Btree btree(this->accountBaseDir, &fileName, this->cacheManager, keyFactory, dataFactory);

		BtreeConfig config;
		config.nodeNumber = 8;
		config.defaultSize = 1024;
		config.blockSize = 32;
		btree.create(&config);
	}

	{
		UnicodeString fileName(WalletAccountUtxoRepository::UTXO_ID_INDEX_FILE_NAME);

		UtxoIdKeyFactory* keyFactory = new UtxoIdKeyFactory(); __STP(keyFactory);
		AddressDescriptorDataFactory* dataFactory = new AddressDescriptorDataFactory(); __STP(dataFactory);

		Btree btree(this->accountBaseDir, &fileName, this->cacheManager, keyFactory, dataFactory);

		BtreeConfig config;
		config.nodeNumber = 8;
		config.defaultSize = 1024;
		config.blockSize = 32;
		btree.create(&config);
	}
}

void WalletAccountUtxoRepository::open() {
	close();

	{
		UnicodeString fileName(WalletAccountUtxoRepository::FILE_NAME);

		AddressDescriptorKeyFactory* keyFactory = new AddressDescriptorKeyFactory(); __STP(keyFactory);
		AddressDescriptorUtxoDataFactory* dataFactory = new AddressDescriptorUtxoDataFactory(); __STP(dataFactory);

		this->btree = new Btree(this->accountBaseDir, &fileName, this->cacheManager, keyFactory, dataFactory);

		BtreeOpenConfig opconf;
		opconf.numDataBuffer = 256;
		opconf.numNodeBuffer = 512;
		this->btree->open(&opconf);
	}

	{
		UnicodeString fileName(WalletAccountUtxoRepository::UTXO_ID_INDEX_FILE_NAME);

		UtxoIdKeyFactory* keyFactory = new UtxoIdKeyFactory(); __STP(keyFactory);
		AddressDescriptorDataFactory* dataFactory = new AddressDescriptorDataFactory(); __STP(dataFactory);

		this->utxoIdBtree = new Btree(this->accountBaseDir, &fileName, this->cacheManager, keyFactory, dataFactory);

		BtreeOpenConfig opconf;
		opconf.numDataBuffer = 256;
		opconf.numNodeBuffer = 512;
		this->utxoIdBtree->open(&opconf);
	}

}

void WalletAccountUtxoRepository::close() noexcept {
	if(this->btree != nullptr){
		this->btree->close();
		delete this->btree;
		this->btree = nullptr;
	}

	if(this->utxoIdBtree != nullptr){
		this->utxoIdBtree->close();
		delete this->utxoIdBtree;
		this->utxoIdBtree = nullptr;
	}
}

void WalletAccountUtxoRepository::importUtxo(const AbstractUtxo *utxo) {
	{
		AddressDescriptorKey key(utxo->getAddress());
		AddressDescriptorUtxoData data;
		data.add(utxo);

		this->btree->putData(&key, &data);
	}

	{
		const UtxoId* utxoId = utxo->getId();
		const AddressDescriptor* desc = utxo->getAddress();
		UtxoIdKey key(utxoId);
		AddressDescriptorData data(desc);

		this->utxoIdBtree->putData(&key, &data);
	}
}

void WalletAccountUtxoRepository::removeUtxo(const UtxoId *utxoId) {
	AbstractUtxo* utxo = getUtxo(utxoId); __STP(utxo);
	assert(utxo != nullptr);

	if(utxo != nullptr){
		const AddressDescriptor* desc = utxo->getAddress();
		{
			AddressDescriptorKey key(desc);
			key.setUtxo(utxo);
			this->btree->remove(&key);
		}
		{
			UtxoIdKey key(utxoId);
			this->utxoIdBtree->remove(&key);
		}
	}

}

BalanceUnit WalletAccountUtxoRepository::getTotalAmount() const noexcept {
	BalanceUnit ret(0L);

	BtreeScanner* scanner = this->btree->getScanner(); __STP(scanner);
	while(scanner->hasNext()){
		const IBlockObject* obj = scanner->next();
		const AddressDescriptorUtxoData* data = dynamic_cast<const AddressDescriptorUtxoData*>(obj);

		ret = ret + data->getTotalAmount();
	}

	return ret;
}

BalanceUtxo* WalletAccountUtxoRepository::getBalanceUtxo(const UtxoId *utxoId) {
	UtxoIdKey key(utxoId);
	IBlockObject* obj = this->utxoIdBtree->findByKey(&key); __STP(obj);

	AddressDescriptorData* data = dynamic_cast<AddressDescriptorData*>(obj);
	if(data == nullptr){
		return nullptr;
	}

	ArrayList<BalanceUtxo>* list = getBalanceUtxos(data->getDescriptor()); __STP(list);
	assert(list != nullptr);
	list->setDeleteOnExit();

	BalanceUtxo* ret = nullptr;

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		BalanceUtxo* utxo = list->get(i);
		const UtxoId* id = utxo->getId();

		if(id->compareTo(utxoId) == 0){
			ret = dynamic_cast<BalanceUtxo*>(utxo->copyData());
			break;
		}
	}

	return ret;
}

ArrayList<BalanceUtxo>* WalletAccountUtxoRepository::getBalanceUtxos(const AddressDescriptor *desc) {
	AddressDescriptorKey key(desc);
	IBlockObject* obj = this->btree->findByKey(&key); __STP(obj);

	AddressDescriptorUtxoData* data = dynamic_cast<AddressDescriptorUtxoData*>(obj);
	if(data == nullptr){
		return nullptr;
	}

	ArrayList<BalanceUtxo>* list = new ArrayList<BalanceUtxo>();

	const ArrayList<AbstractUtxo>* l = data->getList();
	int maxLoop = l->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractUtxo* utxo = l->get(i);

		BalanceUtxo* butxo = dynamic_cast<BalanceUtxo*>(utxo);
		if(butxo != nullptr){
			list->addElement(dynamic_cast<BalanceUtxo*>(butxo->copyData()));
		}
	}

	return list;
}

AbstractUtxo* WalletAccountUtxoRepository::getUtxo(const UtxoId *utxoId) {
	UtxoIdKey key(utxoId);
	IBlockObject* obj = this->utxoIdBtree->findByKey(&key); __STP(obj);

	AddressDescriptorData* data = dynamic_cast<AddressDescriptorData*>(obj);
	if(data == nullptr){
		return nullptr;
	}

	ArrayList<AbstractUtxo>* list = getUtxos(data->getDescriptor()); __STP(list);
	assert(list != nullptr);
	list->setDeleteOnExit();

	AbstractUtxo* ret = nullptr;

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractUtxo* utxo = list->get(i);
		const UtxoId* id = utxo->getId();

		if(id->compareTo(utxoId) == 0){
			ret = dynamic_cast<AbstractUtxo*>(utxo->copyData());
			break;
		}
	}

	return ret;
}

ArrayList<AbstractUtxo>* WalletAccountUtxoRepository::getUtxos(const AddressDescriptor *desc) {
	AddressDescriptorKey key(desc);
	IBlockObject* obj = this->btree->findByKey(&key); __STP(obj);

	AddressDescriptorUtxoData* data = dynamic_cast<AddressDescriptorUtxoData*>(obj);
	if(data == nullptr){
		return nullptr;
	}

	ArrayList<AbstractUtxo>* list = new ArrayList<AbstractUtxo>();

	const ArrayList<AbstractUtxo>* l = data->getList();
	int maxLoop = l->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractUtxo* utxo = l->get(i);

		list->addElement(dynamic_cast<AbstractUtxo*>(utxo->copyData()));
	}

	return list;
}



BtreeScanner* WalletAccountUtxoRepository::getScanner() const {
	BtreeScanner* scanner = this->utxoIdBtree->getScanner();

	return scanner;
}

} /* namespace codablecash */
