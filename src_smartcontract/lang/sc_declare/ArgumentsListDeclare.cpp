/*
 * ArgumentsListDeclare.cpp
 *
 *  Created on: 2019/01/19
 *      Author: iizuka
 */

#include "lang/sc_declare/ArgumentsListDeclare.h"
#include "lang/sc_declare/ArgumentDeclare.h"

#include "base/UnicodeString.h"

#include "engine/sc_analyze/AnalyzedType.h"

namespace alinous {

ArgumentsListDeclare::ArgumentsListDeclare() : CodeElement(CodeElement::ARGUMENTS_LIST_DECLARE) {
	this->callSig = nullptr;
	this->typelist = nullptr;
	this->str = nullptr;
}

ArgumentsListDeclare::~ArgumentsListDeclare() {
	this->list.deleteElements();
	delete this->callSig;

	if(this->typelist != nullptr){
		this->typelist->deleteElements();
		delete this->typelist;
	}

	delete this->str;
}

void ArgumentsListDeclare::addArgument(ArgumentDeclare* arg) noexcept {
	this->list.addElement(arg);
}

void ArgumentsListDeclare::preAnalyze(AnalyzeContext* actx) {
	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		ArgumentDeclare* arg = this->list.get(i);
		arg->setParent(this);
		arg->preAnalyze(actx);
	}
}

void ArgumentsListDeclare::analyzeTypeRef(AnalyzeContext* actx) {
	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		ArgumentDeclare* arg = this->list.get(i);
		arg->analyzeTypeRef(actx);
	}
}

void ArgumentsListDeclare::analyze(AnalyzeContext* actx) {
	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		ArgumentDeclare* arg = this->list.get(i);
		arg->analyze(actx);
	}
}

int ArgumentsListDeclare::getSize() const noexcept {
	return this->list.size();
}

const ArrayList<ArgumentDeclare>* ArgumentsListDeclare::getArguments() const noexcept {
	return &this->list;
}


const UnicodeString* ArgumentsListDeclare::getCallSignature() noexcept {
	if(this->callSig == nullptr){
		this->callSig = new UnicodeString(L"");

		int maxLoop = this->list.size();
		for(int i = 0; i != maxLoop; ++i){
			ArgumentDeclare* arg = this->list.get(i);

			AnalyzedType* atype = arg->getAnalyzedType();
			const UnicodeString* str = atype->getSignatureName();

			this->callSig->append(str);
		}
	}

	return this->callSig;
}

ArrayList<AnalyzedType>* ArgumentsListDeclare::getArgumentsAnalyzedType() noexcept {
	if(this->typelist == nullptr){
		this->typelist = new ArrayList<AnalyzedType>();

		int maxLoop = this->list.size();
		for(int i = 0; i != maxLoop; ++i){
			ArgumentDeclare* arg = this->list.get(i);
			AnalyzedType* atype = arg->getAnalyzedType();

			this->typelist->addElement(new AnalyzedType(*atype));
		}
	}

	return this->typelist;
}


int ArgumentsListDeclare::binarySize() const {
	int total = sizeof(uint16_t);

	int maxLoop = this->list.size();
	total += sizeof(uint32_t);

	for(int i = 0; i != maxLoop; ++i){
		ArgumentDeclare* arg = this->list.get(i);
		total += arg->binarySize();
	}

	total += positionBinarySize();

	return total;
}

void ArgumentsListDeclare::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::ARGUMENTS_LIST_DECLARE);

	int maxLoop = this->list.size();
	out->putInt(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		ArgumentDeclare* arg = this->list.get(i);
		arg->toBinary(out);
	}

	positionToBinary(out);
}


void ArgumentsListDeclare::fromBinary(ByteBuffer* in) {
	int maxLoop = in->getInt();
	for(int i = 0; i != maxLoop; ++i){
		CodeElement* element = createFromBinary(in);
		checkKind(element, CodeElement::ARGUMENT_DECLARE);

		ArgumentDeclare* arg = dynamic_cast<ArgumentDeclare*>(element);
		this->list.addElement(arg);
	}

	positionFromBinary(in);
}

const UnicodeString* ArgumentsListDeclare::toString() noexcept {
	if(this->str == nullptr){
		this->str = new UnicodeString(L"");
		makeString();
	}

	return this->str;
}

void ArgumentsListDeclare::makeString() noexcept {
	const ArrayList<ArgumentDeclare>* argList =  getArguments();
	ArrayList<AnalyzedType>* typeList = getArgumentsAnalyzedType();

	int maxLoop = argList->size();
	for(int i = 0; i != maxLoop; ++i){
		ArgumentDeclare* dec = argList->get(i);
		AnalyzedType* atype = typeList->get(i);

		if(i > 0){
			this->str->append(L", ");
		}

		this->str->append(atype->stringName());
		this->str->append(L" ");
		this->str->append(dec->getName());
	}

}

ArgumentsListDeclare* ArgumentsListDeclare::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	ArgumentsListDeclare* inst = new ArgumentsListDeclare();
	inst->copyCodePositions(this);

	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		ArgumentDeclare* argDec = this->list.get(i);
		ArgumentDeclare* copiedArg = argDec->generateGenericsImplement(input);

		inst->addArgument(copiedArg);
	}

	return inst;
}

} /* namespace alinous */
