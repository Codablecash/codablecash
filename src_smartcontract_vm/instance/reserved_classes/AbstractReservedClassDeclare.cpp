/*
 * AbstractReservedClassDeclare.cpp
 *
 *  Created on: 2020/03/17
 *      Author: iizuka
 */

#include "instance/reserved_classes/AbstractReservedClassDeclare.h"
#include "instance/reserved_classes/ReservedClassRegistory.h"

#include "instance/reserved_classes_string/StringClassDeclare.h"

#include "lang/sc_declare/MethodDeclare.h"
#include "lang/sc_declare/MemberVariableDeclare.h"

#include "base/ArrayList.h"
#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "lang/sc_declare/AccessControlDeclare.h"
#include "lang/sc_declare/ArgumentsListDeclare.h"
#include "lang/sc_declare/ClassExtends.h"
#include "lang/sc_declare/ClassDeclareBlock.h"
#include "lang/sc_declare/ClassImplements.h"
#include "lang/sc_declare/ClassName.h"

#include "lang/sc_statement/StatementBlock.h"

#include "engine/sc/CompilationUnit.h"

#include "engine/sc_analyze/PackageSpace.h"
#include "engine/sc_analyze/AnalyzedType.h"
#include "engine/sc_analyze/AnalyzeContext.h"
#include "engine/sc_analyze/AnalyzedClass.h"

#include "filestore_block/exceptions.h"

#include "instance/instance_exception/ArrayOutOfBoundsExceptionClassDeclare.h"
#include "instance/instance_exception_class/ExceptionClassDeclare.h"
#include "instance/instance_exception/NullPointerExceptionClassDeclare.h"
#include "instance/instance_exception/TypeCastExceptionClassDeclare.h"
#include "instance/instance_exception/ZeroDivisionExceptionClassDeclare.h"

#include "trx/transaction_exception/DatabaseExceptionClassDeclare.h"

#include "instance/reserved_classes/object/ObjectClassDeclare.h"

#include "modular_interfaces/ModularProxyListnerClassDeclare.h"

#include "inter_modular_access/ModularProxyClassDeclare.h"


using namespace codablecash;

namespace alinous {

AbstractReservedClassDeclare::AbstractReservedClassDeclare() : ClassDeclare(RESERVED_CLASS_DECLARE) {
	this->methods = new ArrayList<MethodDeclare>();
	this->members = new ArrayList<MemberVariableDeclare>();
}

AbstractReservedClassDeclare::~AbstractReservedClassDeclare() {
	this->methods->deleteElements();
	delete this->methods;

	this->members->deleteElements();
	delete this->members;
}

ArrayList<MethodDeclare>* AbstractReservedClassDeclare::getMethods() noexcept {
	return this->methods;
}

ArrayList<MemberVariableDeclare>* AbstractReservedClassDeclare::getMemberVariables() noexcept {
	return this->members;
}

void AbstractReservedClassDeclare::preAnalyze(AnalyzeContext* actx) {
	// Object as Extends
	const UnicodeString* packageName = getPackageName();
	if(this->extends == nullptr &&
			!(packageName != nullptr && packageName->equals(&ObjectClassDeclare::PACKAGE)
					&& this->name->equals(ObjectClassDeclare::NAME))){
		this->extends = new ClassExtends();
		ClassName* name = new ClassName();
		name->addStr("lang.Object");
		this->extends->setClassName(name);
	}
	if(this->extends != nullptr){
		this->extends->setParent(this);
		this->extends->preAnalyze(actx);
	}
	if(this->implements != nullptr){
		this->implements->setParent(this);
		this->implements->preAnalyze(actx);
	}

	int maxLoop = this->methods->size();
	for(int i = 0; i != maxLoop; ++i){
		MethodDeclare* method = this->methods->get(i);

		method->setParent(this);
		method->preAnalyze(actx);
	}

	maxLoop = this->members->size();
	for(int i = 0; i != maxLoop; ++i){
		MemberVariableDeclare* member = this->members->get(i);

		member->setParent(this);
		member->preAnalyze(actx);
	}
}

void AbstractReservedClassDeclare::analyzeTypeRef(AnalyzeContext* actx) {
	if(this->extends != nullptr){
		this->extends->analyzeTypeRef(actx);
	}
	if(this->implements != nullptr){
		this->implements->analyzeTypeRef(actx);
	}

	if(actx->hasError()){
		return;
	}

	int maxLoop = this->methods->size();
	for(int i = 0; i != maxLoop; ++i){
		MethodDeclare* method = this->methods->get(i);

		method->analyzeTypeRef(actx);
	}

	maxLoop = this->members->size();
	for(int i = 0; i != maxLoop; ++i){
		MemberVariableDeclare* member = this->members->get(i);

		member->analyzeTypeRef(actx);
	}

	PackageSpace* space = actx->getPackegeSpace(this->getPackageName());

	const UnicodeString* fqn = getName();
	AnalyzedClass* dec = space->getClass(fqn);

	// set analyzed class
	if(this->extends != nullptr){
		AnalyzedType* cls = this->extends->getAnalyzedType();
		dec->setExtends(cls->getAnalyzedClass());
	}

	if(this->implements != nullptr){
		const ArrayList<AnalyzedType>* list = this->implements->getAnalyzedTypes();

		int maxLoop = list->size();
		for(int i = 0; i != maxLoop; ++i){
			AnalyzedType* cls = list->get(i);
			dec->addImplements(cls->getAnalyzedClass());
		}
	}
}

void AbstractReservedClassDeclare::analyze(AnalyzeContext* actx) {
	int maxLoop = this->methods->size();
	for(int i = 0; i != maxLoop; ++i){
		MethodDeclare* method = this->methods->get(i);

		method->analyze(actx);
	}

	maxLoop = this->members->size();
	for(int i = 0; i != maxLoop; ++i){
		MemberVariableDeclare* member = this->members->get(i);

		member->analyze(actx);
	}
}

void AbstractReservedClassDeclare::init(VirtualMachine *vm) {

}

void AbstractReservedClassDeclare::addDefaultConstructor(const UnicodeString* className) noexcept {
	MethodDeclare* m = new MethodDeclare();
	this->methods->addElement(m);

	AccessControlDeclare* ctrl = new AccessControlDeclare();
	ctrl->setCtrl(AccessControlDeclare::PUBLIC);
	m->setAccessControl(ctrl);

	UnicodeString* name = new UnicodeString(className);
	m->setName(name);

	ArgumentsListDeclare* argDeclare = new ArgumentsListDeclare();
	m->setArguments(argDeclare);

	StatementBlock* block = new StatementBlock();
	m->setBlock(block);
}

AbstractReservedClassDeclare* AbstractReservedClassDeclare::createFromBinary(ByteBuffer *in) {
	AbstractReservedClassDeclare* ret = nullptr;

	uint16_t classType = in->getShort();

	switch(classType){
	case TYPE_STRING_CLASS:
		ret = new StringClassDeclare();
		break;
	case TYPE_ARRAY_OUT_OF_BOUNDS_EXCEPTION:
		ret = new ArrayOutOfBoundsExceptionClassDeclare();
		break;
	case TYPE_DATABASE_EXCEPTION:
		ret = new DatabaseExceptionClassDeclare();
		break;
	case TYPE_EXCEPTION:
		ret = new ExceptionClassDeclare();
		break;
	case TYPE_NULL_POINTER_EXCEPTION:
		ret = new NullPointerExceptionClassDeclare();
		break;
	case TYPE_TYPE_CAST_EXCEPTION:
		ret = new TypeCastExceptionClassDeclare();
		break;
	case TYPE_ZERO_DIVISION_EXCEPTION:
		ret = new ZeroDivisionExceptionClassDeclare();
		break;
	case TYPE_OBJECT:
		ret = new ObjectClassDeclare();
		break;
	case TYPE_MODULAR_PROXY:
		ret = new ModularProxyClassDeclare();
		break;
	case TYPE_MODULAR_PROXY_LISTNER:
		ret = new ModularProxyListnerClassDeclare();
		break;
	default:
		throw new BinaryFormatException(__FILE__, __LINE__);
	}

	return ret;
}

int AbstractReservedClassDeclare::binarySize() const {
	checkNotNull(this->name);

	int total = sizeof(uint16_t); // toBinaryHead(out);

	total += sizeof(uint16_t); // out->putShort(getClassType());

	return total;
}

void AbstractReservedClassDeclare::toBinary(ByteBuffer *out) const {
	checkNotNull(this->name);

	toBinaryHead(out);
	out->putShort(getClassType());
}

void AbstractReservedClassDeclare::fromBinary(ByteBuffer *in) {

}

void AbstractReservedClassDeclare::addMethod(MethodDeclare *method) noexcept {
	this->methods->addElement(method);
}

ClassDeclare* AbstractReservedClassDeclare::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) {
	int cap = binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true);

	toBinary(buff);
	buff->position(0);

	CodeElement* element = CodeElement::createFromBinary(buff);

	return dynamic_cast<ClassDeclare*>(element);
}

} /* namespace alinous */
