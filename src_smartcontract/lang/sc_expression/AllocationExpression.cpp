/*
 * AllocationExpression.cpp
 *
 *  Created on: 2019/02/05
 *      Author: iizuka
 */

#include "lang/sc_expression/AllocationExpression.h"

#include "lang/sc_declare/PackageNameDeclare.h"
#include "lang/sc_expression/ConstructorCall.h"
#include "lang/sc_expression/ConstructorArray.h"

#include "engine/sc_analyze/AnalyzedType.h"
#include "engine/sc_analyze/TypeResolver.h"
#include "engine/sc_analyze/AnalyzeContext.h"
#include "engine/sc_analyze/AnalyzedThisClassStackPopper.h"

#include "instance/VmClassInstance.h"

#include "base/StackRelease.h"

#include "engine/sc_analyze/ValidationError.h"

namespace alinous {

AllocationExpression::AllocationExpression() : AbstractExpression(CodeElement::EXP_ALLOCATION) {
	this->constructorCall = nullptr;
	this->array = nullptr;
}

AllocationExpression::~AllocationExpression() {
	delete this->constructorCall;
	delete this->array;
}

void AllocationExpression::preAnalyze(AnalyzeContext* actx) {
	if(this->constructorCall != nullptr){
		this->constructorCall->setParent(this);
		this->constructorCall->preAnalyze(actx);
	}
	if(this->array != nullptr){
		this->array->setParent(this);
		this->array->preAnalyze(actx);
	}
}

void AllocationExpression::analyzeTypeRef(AnalyzeContext* actx) {
	if(this->constructorCall != nullptr){
		this->constructorCall->analyzeTypeRef(actx);
	}
	if(this->array != nullptr){
		this->array->analyzeTypeRef(actx);
	}
}

void AllocationExpression::analyze(AnalyzeContext* actx) {
	if(this->array != nullptr){
		analyzeArray(actx);
		return;
	}

	TypeResolver* typeResolver = actx->getTypeResolver();

	const UnicodeString* className = this->constructorCall->getName();
	AnalyzedType* atype = typeResolver->findClassType(this, className); __STP(atype);
	if(atype == nullptr){
		actx->addValidationError(ValidationError::CODE_ALLOCATION_TYPE_DOES_NOT_EXISTS, this, L"The class '{0}' does not exists.", {className});
		return;
	}

	if(atype->isInterface()){
		actx->addValidationError(ValidationError::CODE_ALLOCATION_TYPE_IS_INTERFACE, this, L"The class '{0}' is interface.", {className});
		return;
	}

	AnalyzedThisClassStackPopper popper(actx, atype->getAnalyzedClass());
	this->constructorCall->analyze(actx);
}

void AllocationExpression::analyzeArray(AnalyzeContext* actx) {
	TypeResolver* typeResolver = actx->getTypeResolver();
	const UnicodeString* className = this->array->getName();

	AnalyzedType* atype = findType(actx, className); __STP(atype);
	if(atype == nullptr){
		actx->addValidationError(ValidationError::CODE_ALLOCATION_TYPE_DOES_NOT_EXISTS, this, L"The class '{0}' does not exists.", {className});
		return;
	}

	actx->setTmpArrayType(atype);
	this->array->analyze(actx);
}

AnalyzedType* AllocationExpression::findType(AnalyzeContext* actx, const UnicodeString* className) const {
	TypeResolver* typeResolver = actx->getTypeResolver();

	AnalyzedType* atype = typeResolver->findBasicType(className);
	if(atype == nullptr){
		atype = typeResolver->findClassType(this, className);
	}

	return atype;
}

void AllocationExpression::setExpression(ConstructorCall* exp) noexcept {
	this->constructorCall = exp;
}

void AllocationExpression::setConstructorArray(ConstructorArray* array) noexcept {
	this->array = array;
}

int AllocationExpression::binarySize() const {
	int total = sizeof(uint16_t);

	total += sizeof(uint8_t);
	if(this->constructorCall != nullptr){
		total += this->constructorCall->binarySize();
	}

	total += sizeof(uint8_t);
	if(this->array != nullptr){
		total += this->array->binarySize();
	}

	total += positionBinarySize();

	return total;
}

void AllocationExpression::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::EXP_ALLOCATION);

	uint8_t bl = (this->constructorCall != nullptr) ? 1 : 0;
	out->put(bl);
	if(this->constructorCall != nullptr){
		this->constructorCall->toBinary(out);
	}

	bl = (this->array != nullptr) ? 1 : 0;
	out->put(bl);
	if(this->array != nullptr){
		this->array->toBinary(out);
	}

	positionToBinary(out);
}

void AllocationExpression::fromBinary(ByteBuffer* in) {
	uint8_t bl = in->get();
	if(bl == 1){
		CodeElement* element = createFromBinary(in);
		checkKind(element, CodeElement::EXP_CONSTRUCTORCALL);
		this->constructorCall = dynamic_cast<ConstructorCall*>(element);
	}

	bl = in->get();
	if(bl == 1){
		CodeElement* element = createFromBinary(in);
		checkKind(element, CodeElement::EXP_CONSTRUCTORARRAY);
		this->array = dynamic_cast<ConstructorArray*>(element);
	}

	positionFromBinary(in);
}

AnalyzedType AllocationExpression::getType(AnalyzeContext* actx) {
	if(this->array != nullptr){
		return this->array->getType(actx);
	}

	return this->constructorCall->getType(actx);
}

void AllocationExpression::init(VirtualMachine* vm) {
	if(this->array != nullptr){
		this->array->init(vm);
		return;
	}

	this->constructorCall->init(vm);
}

AbstractVmInstance* AllocationExpression::interpret(VirtualMachine* vm) {
	if(this->array != nullptr){
		return interpretArray(vm);
	}

	AbstractVmInstance* inst = this->constructorCall->interpret(vm);

	return inst; // expression::interpret()
}

AbstractVmInstance* AllocationExpression::interpretArray(VirtualMachine* vm) {
	AbstractVmInstance* inst = this->array->interpret(vm);

	return inst;
}

AbstractExpression* AllocationExpression::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	AllocationExpression* inst = new AllocationExpression();
	inst->copyCodePositions(this);

	if(this->constructorCall != nullptr){
		AbstractExpression* ex = this->constructorCall->generateGenericsImplement(input);
		ConstructorCall* call = dynamic_cast<ConstructorCall*>(ex);

		inst->setExpression(call);
	}

	if(this->array != nullptr){
		AbstractExpression* ex = this->array->generateGenericsImplement(input);
		ConstructorArray* ar = dynamic_cast<ConstructorArray*>(ex);

		inst->setConstructorArray(ar);
	}

	return inst;
}

} /* namespace alinous */
