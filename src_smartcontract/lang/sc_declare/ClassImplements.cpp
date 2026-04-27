/*
 * ClassImplements.cpp
 *
 *  Created on: 2019/06/19
 *      Author: iizuka
 */

#include "lang/sc_declare/ClassImplements.h"
#include "lang/sc_declare/ClassName.h"

#include "engine/sc_analyze/AnalyzeContext.h"
#include "engine/sc_analyze/AnalyzedType.h"
#include "engine/sc_analyze/TypeResolver.h"

#include "engine/sc_analyze/ValidationError.h"
namespace alinous {

ClassImplements::ClassImplements() : CodeElement(CodeElement::CLASS_IMPLEMENTS) {

}

ClassImplements::~ClassImplements() {
	this->list.deleteElements();
	this->typelist.deleteElements();
}

void ClassImplements::addClassName(ClassName* name) noexcept {
	this->list.addElement(name);
}

const ArrayList<AnalyzedType>* ClassImplements::getAnalyzedTypes() const noexcept {
	return &this->typelist;
}

void ClassImplements::preAnalyze(AnalyzeContext* actx) {
	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		ClassName* n = this->list.get(i);
		n->setParent(this);
	}
}

void ClassImplements::analyzeTypeRef(AnalyzeContext* actx) {
	TypeResolver* res = actx->getTypeResolver();

	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		ClassName* n = this->list.get(i);
		const UnicodeString* name = n->getName();

		AnalyzedType* type = res->findClassType(this, name);

		actx->addValidationError(!type->isInterface(), ValidationError::CODE_WRONG_CLASS_NAME, this, L"The {0} is not type.", {name});
		actx->addValidationError(!type->isInterface(), ValidationError::CODE_CLASS_IMPLEMENTS_NOT_INTERFACE_CLASS, this, L"The {0} must be interface.", {name});

		this->typelist.addElement(type);
	}
}

int ClassImplements::binarySize() const {
	int total = sizeof(uint16_t);
	total += sizeof(uint32_t);

	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		ClassName* n = this->list.get(i);
		total += n->binarySize();
	}

	total += positionBinarySize();

	return total;
}

void ClassImplements::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::CLASS_IMPLEMENTS);

	int maxLoop = this->list.size();
	out->putInt(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		ClassName* n = this->list.get(i);
		n->toBinary(out);
	}

	positionToBinary(out);
}

void ClassImplements::fromBinary(ByteBuffer* in) {
	int maxLoop = in->getInt();
	for(int i = 0; i != maxLoop; ++i){
		CodeElement* element = CodeElement::createFromBinary(in);
		checkKind(element, CodeElement::CLASS_NAME);

		ClassName* n = dynamic_cast<ClassName*>(element);
		this->list.addElement(n);
	}

	positionFromBinary(in);
}

ClassImplements* ClassImplements::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	ClassImplements* inst = new ClassImplements();
	inst->copyCodePositions(this);

	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		ClassName* n = this->list.get(i);

		ClassName* copy = new ClassName(*n);
		copy->copyCodePositions(n);

		inst->addClassName(copy);
	}

	return inst;
}

} /* namespace alinous */
