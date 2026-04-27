/*
 * ArgumentType.cpp
 *
 *  Created on: Mar 11, 2026
 *      Author: iizuka
 */

#include "ext_arguments/ArgumentType.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "engine/sc_analyze/AnalyzedType.h"
#include "engine/sc_analyze/AnalyzedClass.h"
#include "engine/sc_analyze/TypeResolver.h"

#include "base_io/ByteBuffer.h"

#include "bc_base/BinaryUtils.h"

#include "bc/ExceptionThrower.h"

#include "smartcontract_instance/ClassNotFoundException.h"
using codablecash::BinaryUtils;
using codablecash::ClassNotFoundException;

namespace alinous {

ArgumentType::ArgumentType(const ArgumentType &inst) {
	this->type = inst.type;
	this->dim = inst.dim;

	this->genericsType = inst.genericsType != nullptr ? new UnicodeString(inst.genericsType) : nullptr;
	this->fqn = inst.fqn != nullptr ? new UnicodeString(inst.fqn) : nullptr;
}

ArgumentType::ArgumentType(const AnalyzedType *atype) {
	this->type = atype->getType();
	this->dim = atype->getDim();

	const UnicodeString* genericsType = atype->getGenericsType();
	this->genericsType = genericsType != nullptr ? new UnicodeString(genericsType) : nullptr;

	this->fqn = nullptr;
	AnalyzedClass* clazz = atype->getAnalyzedClass();
	if(clazz != nullptr){
		const UnicodeString* fqn = clazz->getFullQualifiedName();
		this->fqn = new UnicodeString(fqn);
	}
}

ArgumentType::ArgumentType() {
	this->type = 0;
	this->fqn = nullptr; // reference
	this->dim = 0;
	this->genericsType = nullptr;
}

ArgumentType::~ArgumentType() {
	delete this->fqn;
	delete this->genericsType;

}

int ArgumentType::binarySize() const {
	int total = sizeof(uint8_t);

	total += sizeof(uint8_t);
	if(this->fqn != nullptr){
		total += BinaryUtils::stringSize(this->fqn);
	}

	total += sizeof(uint8_t); // dim

	total += sizeof(uint8_t); // genericsType
	if(this->genericsType != nullptr){
		total += BinaryUtils::stringSize(this->genericsType);
	}

	return total;
}

void ArgumentType::toBinary(ByteBuffer *out) const {
	out->put(this->type);

	out->put(this->fqn != nullptr ? 1 : 0);
	if(this->fqn != nullptr){
		BinaryUtils::putString(out, this->fqn);
	}

	out->put(this->dim);

	out->put(this->genericsType != nullptr ? 1 : 0);
	if(this->genericsType != nullptr){
		BinaryUtils::putString(out, this->genericsType);
	}
}

ArgumentType* ArgumentType::createFromBinary(ByteBuffer *in) {
	ArgumentType* inst = new ArgumentType(); __STP(inst);

	inst->type = in->get();

	uint8_t bl = in->get();
	if(bl > 0){
		inst->fqn = BinaryUtils::getString(in);
	}

	inst->dim = in->get();

	bl = in->get();
	if(bl > 0){
		inst->genericsType = BinaryUtils::getString(in);
	}

	return __STP_MV(inst);
}

void ArgumentType::setType(uint8_t type) {
	this->type = type;
}

AnalyzedType* ArgumentType::toAnalyzedType(TypeResolver *resolver, CodeElement *element) const {
	AnalyzedType* atype = new AnalyzedType(this->type); __STP(atype);

	atype->setDim(this->dim);

	if(this->genericsType != nullptr){
		atype->setGenericsType(this->genericsType);
	}

	if(this->fqn != nullptr){
		AnalyzedType* type = resolver->findClassType(element, this->fqn); __STP(type);

		codablecash::ExceptionThrower<ClassNotFoundException>::throwExceptionIfCondition(type == nullptr, L"Class not found", __FILE__, __LINE__);

		const AnalyzedClass* aclazz = type->getAnalyzedClass();
		atype->setAnalyzedClass(new AnalyzedClass(*aclazz));
	}

	return __STP_MV(atype);
}

} /* namespace alinous */
