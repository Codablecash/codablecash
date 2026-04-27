/*
 * GenericsObjectType.cpp
 *
 *  Created on: Oct 20, 2025
 *      Author: iizuka
 */

#include "lang/sc_declare_types/GenericsObjectType.h"
#include "lang/sc_declare_types/ObjectType.h"
#include "lang/sc_declare_types/AbstractType.h"
#include "lang/sc_declare_types/ITypeVisitor.h"

#include "lang/sc_declare/PackageNameDeclare.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"
#include "base/Exception.h"

#include "engine/sc_analyze/TypeResolver.h"
#include "engine/sc_analyze/AnalyzedType.h"
#include "engine/sc_analyze/AnalyzedClass.h"
#include "engine/sc_analyze/GeneratedGenericsTypeNaming.h"
#include "engine/sc_analyze/AnalyzeContext.h"
#include "engine/sc_analyze/GenericsTypeRefChecker.h"

#include "vm/exceptions.h"

#include "bc/ExceptionThrower.h"

#include "engine/sc_analyze/ValidationError.h"


using codablecash::ExceptionThrower;

namespace alinous {

GenericsObjectType::GenericsObjectType() : ObjectType(TYPE_GENERICS_OBJECT) {
	this->genericsType = new ArrayList<AbstractType>();
	this->gclassName = nullptr;
}

GenericsObjectType::GenericsObjectType(const ObjectType *objectType) : ObjectType(TYPE_GENERICS_OBJECT) {
	const PackageNameDeclare* package = objectType->getPackageNameDeclare();

	this->packageName = package != nullptr ? new PackageNameDeclare(*package) : nullptr;
	this->className = new UnicodeString(objectType->getClassName());

	this->genericsType = new ArrayList<AbstractType>();
	this->gclassName = nullptr;
}

GenericsObjectType::~GenericsObjectType() {
	this->genericsType->deleteElements();
	delete this->genericsType;

	delete this->gclassName;
}

int GenericsObjectType::binarySize() const {
	int total = ObjectType::binarySize();

	int maxLoop = this->genericsType->size();
	total += sizeof(uint16_t);

	for(int i = 0; i != maxLoop; ++i){
		AbstractType* t = this->genericsType->get(i);
		total += t->binarySize();
	}

	return total;
}

void GenericsObjectType::toBinary(ByteBuffer *out) const {
	ObjectType::toBinary(out);

	int maxLoop = this->genericsType->size();
	out->putShort(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		AbstractType* t = this->genericsType->get(i);
		t->toBinary(out);
	}
}

void GenericsObjectType::fromBinary(ByteBuffer *in) {
	ObjectType::fromBinary(in);

	int maxLoop = in->getShort();
	for(int i = 0; i != maxLoop; ++i){
		CodeElement* element = CodeElement::createFromBinary(in); __STP(element);
		AbstractType* t = dynamic_cast<AbstractType*>(element);

		checkNotNull(t);
		this->genericsType->addElement(t);

		__STP_MV(element);
	}
}

void GenericsObjectType::addGenericsType(AbstractType *type) noexcept {
	this->genericsType->addElement(type);
	delete this->gclassName;
	this->gclassName = nullptr;
}

const UnicodeString* GenericsObjectType::toString() noexcept {
	if(this->str == nullptr){
		this->str = new UnicodeString(L"");

		if(this->packageName != nullptr){
			this->str->append(this->packageName->getName());
			this->str->append(L".");
		}

		this->str->append(this->className);

		this->str->append(L"<");

		int maxLoop = this->genericsType->size();
		for(int i = 0; i != maxLoop; ++i){
			if(i != 0){
				this->str->append(L",");
			}

			AbstractType* t = this->genericsType->get(i);
			this->str->append(t->toString());
		}

		this->str->append(L">");
	}

	return this->str;
}

AbstractType* GenericsObjectType::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	GenericsObjectType* inst = new GenericsObjectType();
	inst->copyCodePositions(this);

	PackageNameDeclare* pname = this->packageName != nullptr ? this->packageName->generateGenericsImplement(input) : nullptr;
	inst->setPackageName(pname);
	inst->setName(new UnicodeString(this->className));


	int maxLoop = this->genericsType->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractType* atypet = this->genericsType->get(i);

		AbstractType* type = atypet->generateGenericsImplement(input);
		inst->addGenericsType(type);
	}

	return inst;
}

const UnicodeString* GenericsObjectType::getClassName() const noexcept {
	if(this->gclassName == nullptr){
		this->gclassName = GeneratedGenericsTypeNaming::fromGenericsObjectType(this);
	}
	return this->gclassName;
}

const UnicodeString* GenericsObjectType::getRawClassName() const noexcept {
	return this->className;
}

void GenericsObjectType::preAnalyze(AnalyzeContext *actx) {
	int maxLoop = this->genericsType->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractType* atypet = this->genericsType->get(i);

		atypet->setParent(this);
		atypet->preAnalyze(actx);
	}
}

void GenericsObjectType::analyzeTypeRef(AnalyzeContext *actx) {
	TypeResolver* resolver = actx->getTypeResolver();

	GenericsTypeRefChecker checker(resolver);
	visit(&checker);

	if(checker.isError()){
		const UnicodeString* errTypeName = checker.getErrorTypeName();
		actx->addValidationError(ValidationError::CODE_CYCRIC_TYPE_REFERENCE, this, L"The class {0} is referred cyclicly.", {errTypeName} );
	}
}

void GenericsObjectType::analyze(AnalyzeContext *actx) {
}

void GenericsObjectType::visit(ITypeVisitor *visitor) {
	visitor->visit(this);

	int maxLoop = this->genericsType->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractType* atypet = this->genericsType->get(i);

		atypet->visit(visitor);
	}

	visitor->exit(this);
}

} /* namespace alinous */
