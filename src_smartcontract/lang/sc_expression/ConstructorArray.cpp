/*
 * ConstructorArray.cpp
 *
 *  Created on: 2020/02/12
 *      Author: iizuka
 */

#include "lang/sc_expression/ConstructorArray.h"

#include "engine/sc_analyze/AnalyzeContext.h"
#include "engine/sc_analyze/AnalyzedType.h"
#include "engine/sc_analyze/AnalyzedThisClassStackPopper.h"
#include "engine/sc_analyze/ValidationError.h"

#include "lang/sc_declare_types/AbstractType.h"

#include "instance/AbstractVmInstance.h"

#include "vm/VirtualMachine.h"

#include "base_io/ByteBuffer.h"

#include "lang/sc_expression/VariableIdentifier.h"
#include "lang/sc_expression_literal/NumberLiteral.h"

#include "instance/instance_array/VmArrayInstanceUtils.h"
#include "instance/instance_ref/PrimitiveReference.h"

#include "base/StackRelease.h"

#include "lang/sc_declare_types/ObjectType.h"
namespace alinous {

ConstructorArray::ConstructorArray() : AbstractExpression(CodeElement::EXP_CONSTRUCTORARRAY) {
	this->valId = nullptr;
	this->atype = nullptr;
}

ConstructorArray::~ConstructorArray() {
	delete this->valId;
	this->dims.deleteElements();
	delete this->atype;
}

int ConstructorArray::binarySize() const {
	checkNotNull(this->valId);

	int total = sizeof(uint16_t);
	total += this->valId->binarySize();

	int maxLoop = this->dims.size();
	total += sizeof(int32_t);

	for(int i = 0; i != maxLoop; ++i){
		AbstractExpression* exp = this->dims.get(i);
		total += exp->binarySize();
	}

	total += positionBinarySize();

	return total;
}

void ConstructorArray::toBinary(ByteBuffer* out) const {
	checkNotNull(this->valId);

	out->putShort(CodeElement::EXP_CONSTRUCTORARRAY);

	this->valId->toBinary(out);

	int maxLoop = this->dims.size();
	out->putInt(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		AbstractExpression* exp = this->dims.get(i);
		exp->toBinary(out);
	}

	positionToBinary(out);
}

void ConstructorArray::fromBinary(ByteBuffer* in) {
	CodeElement* element = createFromBinary(in); __STP(element);

	this->valId = dynamic_cast<AbstractType*>(element);
	checkNotNull(this->valId);
	__STP_MV(element);

	int maxLoop = in->getInt();
	for(int i = 0; i != maxLoop; ++i){
		element = createFromBinary(in);
		checkIsExp(element);

		AbstractExpression* exp = dynamic_cast<AbstractExpression*>(element);
		this->dims.addElement(exp);
	}

	positionFromBinary(in);
}

void ConstructorArray::preAnalyze(AnalyzeContext* actx) {
	this->valId->setParent(this);

	int maxLoop = this->dims.size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractExpression* exp = this->dims.get(i);

		exp->setParent(this);
		exp->preAnalyze(actx);
	}

	this->valId->preAnalyze(actx);
	actx->detectGenericsType(this->valId);
}

void ConstructorArray::analyzeTypeRef(AnalyzeContext* actx) {
	int maxLoop = this->dims.size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractExpression* exp = this->dims.get(i);
		exp->analyzeTypeRef(actx);
	}

	this->valId->analyzeTypeRef(actx);
}

void ConstructorArray::analyze(AnalyzeContext* actx) {
	{
		int maxLoop = this->dims.size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractExpression* exp = this->dims.get(i);
			exp->analyze(actx);

			// check array index type
			AnalyzedType type = exp->getType(actx);
			bool res = VmArrayInstanceUtils::isArrayIndex(type);
			if(!res){
				actx->addValidationError(ValidationError::CODE_ARRAY_INDEX_MUST_BE_NUMERIC, this, L"Array index must be numeric value.", {});
			}

		}
	}

	this->atype = new AnalyzedType(*actx->getTmpArrayType());
	int dim = this->dims.size();
	this->atype->setDim(dim);

	this->valId->analyze(actx);
}

AnalyzedType ConstructorArray::getType(AnalyzeContext* actx) {
	return *this->atype;
}

void ConstructorArray::init(VirtualMachine* vm) {
	int maxLoop = this->dims.size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractExpression* exp = this->dims.get(i);
		exp->init(vm);
	}
}

AbstractVmInstance* ConstructorArray::interpret(VirtualMachine* vm) {
	int dim = this->atype->getDim();

	int* arrayDim = new int[dim];
	StackArrayRelease<int> __releaseArrayDim(arrayDim);
	for(int i = 0; i != dim; ++i){
		AbstractExpression* idxExp = this->dims.get(i);
		AbstractVmInstance* idxInst = idxExp->interpret(vm);

		PrimitiveReference* primitive = dynamic_cast<PrimitiveReference*>(idxInst);
		int d = primitive->getIntValue();

		arrayDim[i] = d;
	}

	vm->setLastElement(this);
	return VmArrayInstanceUtils::buildArrayInstance(vm, arrayDim, dim, this->atype);
}


void ConstructorArray::setValId(AbstractType* valId) noexcept {
	this->valId = valId;
}

void ConstructorArray::addDim(AbstractExpression* dim) noexcept {
	this->dims.addElement(dim);
}

const UnicodeString* ConstructorArray::getName() const noexcept {
	ObjectType* otype = dynamic_cast<ObjectType*>(this->valId);
	if(otype != nullptr){
		return otype->getClassName();
	}

	return this->valId->toString();
}

AbstractExpression* ConstructorArray::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	ConstructorArray* inst = new ConstructorArray();

	AbstractType* nameType = this->valId->generateGenericsImplement(input);
	inst->setValId(nameType);

	int maxLoop = this->dims.size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractExpression* exp = this->dims.get(i);
		AbstractExpression* copied = exp->generateGenericsImplement(input);

		inst->addDim(copied);
	}

	return inst;
}

} /* namespace alinous */
