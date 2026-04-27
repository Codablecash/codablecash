/*
 * GenericsParameter.cpp
 *
 *  Created on: Oct 18, 2025
 *      Author: iizuka
 */

#include "lang/sc_declare/GenericsParameter.h"
#include "lang/sc_declare/ClassName.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "engine/sc_analyze/AnalyzeContext.h"
#include "engine/sc_analyze/TypeResolver.h"
#include "engine/sc_analyze/AnalyzedType.h"

#include "engine/sc_analyze/ValidationError.h"

namespace alinous {

GenericsParameter::GenericsParameter() : CodeElement(GENERICS_PARAM) {
	this->genericsName = nullptr;
	this->genericsExtendsName = nullptr;
	this->aExtendType = nullptr;
}

GenericsParameter::~GenericsParameter() {
	delete this->genericsName;
	delete this->genericsExtendsName;
	delete this->aExtendType;
}

int GenericsParameter::binarySize() const {
	checkNotNull(this->genericsName);

	int total = sizeof(uint16_t);

	total += stringSize(this->genericsName);

	total += sizeof(uint8_t);
	if(this->genericsExtendsName != nullptr){
		total += this->genericsExtendsName->binarySize();
	}

	total += positionBinarySize();

	return total;
}

void GenericsParameter::toBinary(ByteBuffer *out) const {
	checkNotNull(this->genericsName);

	out->putShort(this->kind);
	putString(out, this->genericsName);

	uint8_t bl = this->genericsExtendsName != nullptr ? 1 : 0;
	out->put(bl);
	if(bl > 0){
		this->genericsExtendsName->toBinary(out);
	}

	positionToBinary(out);
}

void GenericsParameter::fromBinary(ByteBuffer *in) {
	this->genericsName = getString(in);

	uint8_t bl = in->get();
	if(bl > 0){
		CodeElement* element = CodeElement::createFromBinary(in); __STP(element);
		this->genericsExtendsName = dynamic_cast<ClassName*>(element);
		checkNotNull(this->genericsExtendsName);

		__STP_MV(element);
	}

	positionFromBinary(in);
}

void GenericsParameter::setGenericsName(UnicodeString *name) noexcept {
	this->genericsName = name;
}

void GenericsParameter::setGenericsExtendsName(ClassName *name) noexcept {
	this->genericsExtendsName = name;
}

void GenericsParameter::preAnalyze(AnalyzeContext *actx) {

}

void GenericsParameter::analyzeTypeRef(AnalyzeContext *actx) {
	TypeResolver* typeResolver = actx->getTypeResolver();

	if(this->genericsExtendsName != nullptr){
		const UnicodeString* fqn = this->genericsExtendsName->getName();
		this->aExtendType = typeResolver->findClassType(this, fqn);

		if(this->aExtendType == nullptr){
			actx->addValidationError(ValidationError::CODE_WRONG_CLASS_NAME, this, L"The class {0} is not generics type.", {fqn} );
		}
	}
}

void GenericsParameter::analyze(AnalyzeContext *actx) {
}

} /* namespace alinous */
