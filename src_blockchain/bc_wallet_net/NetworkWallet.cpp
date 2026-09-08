/*
 * NetworkWallet.cpp
 *
 *  Created on: Feb 17, 2025
 *      Author: iizuka
 */

#include "bc_wallet_net/NetworkWallet.h"
#include "bc_wallet_net/WalletNetworkManager.h"
#include "bc_wallet_net/StakingSeedManager.h"
#include "bc_wallet_net/WalletConfig.h"
#include "bc_wallet_net/NetworkTransactionHandler.h"

#include "bc_wallet_net_processor/NetworkClientCommandProcessor.h"

#include "bc_wallet/HdWallet.h"
#include "bc_wallet/HdWalletSeed.h"
#include "bc_wallet/WalletAccount.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"

#include "base_io/File.h"

#include "bc_wallet_encoder/PasswordEncoder.h"

#include "bc/CodablecashSystemParam.h"
#include "bc_wallet_filter/BloomFilter1024.h"

#include "bc_wallet_net_sync/NetworkWalletSync.h"

#include "bc_wallet_net_data/NetworkWalletData.h"

#include "bc_network/NodeIdentifierSource.h"

namespace codablecash {

NetworkWallet::NetworkWallet(const File* baseDir, ISystemLogger* logger, const CodablecashSystemParam *config, const WalletConfig* walletConfig) {
	this->baseDir = new File(*baseDir);
	this->logger = logger;
	this->config = new CodablecashSystemParam(*config);
	this->walletConfig = new WalletConfig(*walletConfig);
	this->networkManager = nullptr;

	this->stakingSeedManager = new StakingSeedManager(baseDir);

	this->clientCommandProcessor = new NetworkClientCommandProcessor(this, logger);

	File* dataBaseDir = baseDir->get(L"data"); __STP(dataBaseDir);
	this->walletData = new NetworkWalletData(dataBaseDir, logger, config);
}

NetworkWallet::~NetworkWallet() {
	shutdownNetwork();
	closeData();

	delete this->baseDir;

	delete this->config;
	delete this->walletConfig;
	delete this->stakingSeedManager;

	this->logger = nullptr;
}

void NetworkWallet::init() {
	this->clientCommandProcessor->init();
}

void NetworkWallet::resetComamndQueue() {
	this->clientCommandProcessor->init();
	this->clientCommandProcessor->createBlank();
}

void NetworkWallet::closeData() noexcept {
	if(this->walletData != nullptr){
		this->walletData->close();
		delete this->walletData;
		this->walletData = nullptr;
	}
}

NetworkWallet* NetworkWallet::createNewWallet(const File *dir, const UnicodeString *pass, uint16_t zone, int defaultMaxAddress, ISystemLogger* logger
		, const CodablecashSystemParam *config, const WalletConfig* walletConfig) {
	NetworkWallet* wallet = new NetworkWallet(dir, logger, config, walletConfig); __STP(wallet);
	wallet->init();

	HdWalletSeed* seed = HdWalletSeed::newSeed(); __STP(seed);
	PasswordEncoder enc(pass);

	wallet->doCreateWallet(&enc, seed, zone, defaultMaxAddress);
	wallet->createData();

	return __STP_MV(wallet);
}

void NetworkWallet::doCreateWallet(const IWalletDataEncoder *encoder, const HdWalletSeed *seed, uint16_t zone, int defaultMaxAddress) {
	this->walletData->createHdWallet(seed, zone, encoder, defaultMaxAddress);
	this->init();
}

HdWalletSeed* NetworkWallet::getRootSeed(const IWalletDataEncoder* encoder) const {
	HdWallet* hdWallet = this->walletData->getHdWallet();

	return hdWallet->getRootSeed(encoder);
}

NetworkWallet* NetworkWallet::resotreWallet(const File *dir, const UnicodeString *pass, uint16_t zone, const HdWalletSeed* rootSeed, int defaultMaxAddress, ISystemLogger* logger
		, const CodablecashSystemParam *config, const WalletConfig* walletConfig) {
	NetworkWallet* wallet = new NetworkWallet(dir, logger, config, walletConfig); __STP(wallet);
	wallet->init();

	PasswordEncoder enc(pass);
	wallet->doCreateWallet(&enc, rootSeed, zone, defaultMaxAddress);
	wallet->createData();

	return __STP_MV(wallet);
}

NetworkWallet* NetworkWallet::openWallet(const File *dir, const UnicodeString *pass,
		ISystemLogger *logger, const CodablecashSystemParam *config, const WalletConfig *walletConfig) {
	NetworkWallet* wallet = new NetworkWallet(dir, logger, config, walletConfig); __STP(wallet);
	wallet->resetComamndQueue();

	PasswordEncoder enc(pass);
	wallet->doOpeneWallet(&enc);
	wallet->openData();

	return __STP_MV(wallet);
}

void NetworkWallet::doOpeneWallet(const IWalletDataEncoder *encoder) {
	this->walletData->openHdWallet(encoder);
	this->resetComamndQueue();
}

void NetworkWallet::setNetworkSeeder(INetworkSeeder *seeder) noexcept {
	HdWallet* hdWallet = this->walletData->getHdWallet();

	if(hdWallet != nullptr && this->networkManager == nullptr){
		uint16_t zone = hdWallet->getDefaultZone();
		this->networkManager = new WalletNetworkManager(zone, this->logger, this->clientCommandProcessor);
	}

	this->networkManager->setSeeder(seeder);
}

AddressDescriptor* NetworkWallet::getAddressDescriptor(int accountIndex, int addressIndex) const {
	HdWallet* hdWallet = this->walletData->getHdWallet();

	WalletAccount* account = hdWallet->getAccount(accountIndex);

	return account->getReceivingAddressDescriptor(addressIndex);
}

void NetworkWallet::createData() {
	this->walletData->createBlankData();

	this->clientCommandProcessor->createBlank();
}

void NetworkWallet::openData() {
	this->walletData->openData();
}

void NetworkWallet::initNetwork(INetworkSeeder *seeder, const IWalletDataEncoder* encoder) {
	setNetworkSeeder(seeder);

	// start client processor
	{
		this->clientCommandProcessor->startProcessors(true);
	}

	HdWallet* hdWallet = this->walletData->getHdWallet();
	ArrayList<BloomFilter1024>* filters = hdWallet->getBloomFilters(encoder); __STP(filters);
	filters->setDeleteOnExit();

	this->networkManager->maintainNetwork(filters);
}

void NetworkWallet::syncBlockchain(bool resume) {
	NetworkWalletSync sync((uint64_t)1, this->walletData, this->networkManager, this->walletConfig, this->logger);
	sync.sync();

	if(resume){
		resumeNetwork();
	}
}

void NetworkWallet::resumeNetwork() {
	if(this->clientCommandProcessor->__isSuspended()){
		this->clientCommandProcessor->resumeRequestProcessor();
	}
}

void NetworkWallet::setStakingSourceId(const NodeIdentifierSource *source, const IWalletDataEncoder* encoder) {
	this->stakingSeedManager->setSource(source, encoder);
}

NodeIdentifierSource* NetworkWallet::getStakingSourceId(const IWalletDataEncoder* encoder) const noexcept {
	return this->stakingSeedManager->getSource(encoder);
}

NetworkTransactionHandler* NetworkWallet::getNetworkTransactionHandler(int accountIndex) noexcept {
	NetworkTransactionHandler* handler = new NetworkTransactionHandler(accountIndex, this, this->logger);

	return handler;
}

void NetworkWallet::shutdownNetwork() {
	if(this->networkManager != nullptr){
		this->networkManager->shutdownNetwork();
	}

	if(this->clientCommandProcessor != nullptr){
		this->clientCommandProcessor->shurdownProcessors();

		delete this->clientCommandProcessor;
		this->clientCommandProcessor = nullptr;
	}

	if(this->networkManager != nullptr){
		delete this->networkManager;
		this->networkManager = nullptr;
	}
}

} /* namespace codablecash */
