/*
 * FinalizerConfig.h
 *
 *  Created on: 2023/12/09
 *      Author: iizuka
 */

#ifndef BC_NETWORK_INSTANCE_FINALIZERCONFIG_H_
#define BC_NETWORK_INSTANCE_FINALIZERCONFIG_H_

namespace codablecash {

class NodeIdentifierSource;
class JsonObject;

class FinalizerConfig {
public:
	FinalizerConfig(const FinalizerConfig& inst);
	FinalizerConfig();
	virtual ~FinalizerConfig();

	void setVoterSource(const NodeIdentifierSource* source) noexcept;
	const NodeIdentifierSource* getVoterSource() const noexcept {
		return this->voterSource;
	}

	JsonObject* toJsonObject() const;
	static FinalizerConfig* fromJson(const JsonObject* configObj);

private:
	NodeIdentifierSource* voterSource;
};

} /* namespace codablecash */

#endif /* BC_NETWORK_INSTANCE_FINALIZERCONFIG_H_ */
