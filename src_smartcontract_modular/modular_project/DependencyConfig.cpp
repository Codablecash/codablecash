/*
 * DependencyConfig.cpp
 *
 *  Created on: Oct 8, 2025
 *      Author: iizuka
 */

#include "modular_project/DependencyConfig.h"
#include "modular_project/AbstractDependencyConfig.h"
#include "modular_project/ModularConfigException.h"
#include "modular_project/LocalProjectModuleDependencyConfig.h"

#include "json_object/JsonArrayObject.h"
#include "json_object/JsonValuePair.h"
#include "json_object/AbstractJsonObject.h"
#include "json_object/JsonObject.h"

#include "bc/ExceptionThrower.h"

#include "base_io/ByteBuffer.h"

#include "base/StackRelease.h"


namespace codablecash {

DependencyConfig::DependencyConfig(const DependencyConfig &inst) {
	this->list = new ArrayList<AbstractDependencyConfig>();

	int maxLoop = inst.list->size();
	for(int i = 0; i != maxLoop; ++i){
		const AbstractDependencyConfig* cfg = inst.list->get(i);

		this->list->addElement(cfg->copy());
	}
}

DependencyConfig::DependencyConfig() {
	this->list = new ArrayList<AbstractDependencyConfig>();

}

DependencyConfig::~DependencyConfig() {
	this->list->deleteElements();
	delete this->list;
}

void DependencyConfig::load(const JsonArrayObject *dependencies) {
	int maxLoop = dependencies->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractJsonObject* jobj = dependencies->get(i);
		JsonObject* jsonObj = dynamic_cast<JsonObject*>(jobj);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(jsonObj == nullptr, L"The dependencies must be array object with json objects.", __FILE__, __LINE__);

		if(jsonObj->hasNamedPair(LocalProjectModuleDependencyConfig::LOCALLOC)){
			LocalProjectModuleDependencyConfig* dconf = LocalProjectModuleDependencyConfig::fromJsonObject(jsonObj);
			this->list->addElement(dconf);
		}
	}
}

int DependencyConfig::binarySize() const {
	int total = 0;

	int maxLoop = this->list->size();
	total += sizeof(uint16_t);
	for(int i = 0; i != maxLoop; ++i){
		AbstractDependencyConfig* conf = this->list->get(i);
		total += conf->binarySize();
	}

	return total;
}

void DependencyConfig::toBinary(ByteBuffer *out) const {
	int maxLoop = this->list->size();
	out->putShort(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		AbstractDependencyConfig* conf = this->list->get(i);
		conf->toBinary(out);
	}
}

DependencyConfig* DependencyConfig::createFromBinary(ByteBuffer *in) {
	DependencyConfig* inst = new DependencyConfig(); __STP(inst);

	int maxLoop = in->getShort();
	for(int i = 0; i != maxLoop; ++i){
		AbstractDependencyConfig* conf = AbstractDependencyConfig::createFromBinary(in);
		inst->list->addElement(conf);
	}

	return __STP_MV(inst);
}

} /* namespace codablecash */
