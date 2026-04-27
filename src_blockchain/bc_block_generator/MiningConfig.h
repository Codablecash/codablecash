/*
 * MiningConfig.h
 *
 *  Created on: 2023/03/29
 *      Author: iizuka
 */

#ifndef BC_BLOCK_GENERATOR_MININGCONFIG_H_
#define BC_BLOCK_GENERATOR_MININGCONFIG_H_


namespace alinous {
class UnicodeString;
}
using namespace alinous;

namespace codablecash {

class AddressDescriptor;
class AbstractPoWCalculator;
class NetworkPoWPool;
class PoWManager;

class MiningConfig {
public:
	static const constexpr int TYPE_DEBUG = 1;
	static const constexpr int TYPE_POOL = 2;

	MiningConfig(const MiningConfig& inst);
	MiningConfig();
	virtual ~MiningConfig();

	void setAddressDescriptor(const AddressDescriptor* addressDesc) noexcept;
	const AddressDescriptor* getAddressDescriptor() const noexcept {
		return this->addressDesc;
	}

	void setCalclatorType(int type) noexcept;

	void setProtocol(int type);
	void setServerHost(const UnicodeString* host);
	void setPort(int port);

	AbstractPoWCalculator* getPoWCalculator(PoWManager* powManager) const noexcept;

private:
	void initNetworkPool(NetworkPoWPool* pool) const;

private:
	AddressDescriptor* addressDesc;
	int calcType;

	int protocol;
	UnicodeString* srvhost;
	int port;
};

} /* namespace codablecash */

#endif /* BC_BLOCK_GENERATOR_MININGCONFIG_H_ */
