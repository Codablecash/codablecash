/*
 * ObjectType.cpp
 *
 *  Created on: 2019/06/09
 *      Author: iizuka
 */

#include "lang/sc_declare_types/ObjectType.h"
#include "lang/sc_declare/PackageNameDeclare.h"
#include "lang/sc_declare_types/ITypeVisitor.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"


namespace alinous {

ObjectType::ObjectType(short kind) : AbstractType(kind) {
	this->packageName = nullptr;
	this->className = nullptr;
	this->str = nullptr;
}

ObjectType::ObjectType() : AbstractType(CodeElement::TYPE_OBJECT){
	this->packageName = nullptr;
	this->className = nullptr;
	this->str = nullptr;
}

ObjectType::~ObjectType() {
	delete this->packageName;
	delete this->className;
	delete this->str;
}

void ObjectType::setPackageName(PackageNameDeclare* packageName) noexcept {
	this->packageName = packageName;
}

int ObjectType::binarySize() const {
	int total = sizeof(uint16_t);

	total += sizeof(uint8_t);
	if(this->packageName != nullptr){
		total += this->packageName->binarySize();
	}

	total += stringSize(this->className);

	total += AbstractType::binarySize();

	total += positionBinarySize();

	return total;
}

void ObjectType::toBinary(ByteBuffer* out) const {
	out->putShort(this->kind);

	bool bl = this->packageName != nullptr;
	out->put(bl ? 1 : 0);
	if(bl){
		this->packageName->toBinary(out);
	}

	putString(out, this->className);
	AbstractType::toBinary(out);

	positionToBinary(out);
}

void ObjectType::fromBinary(ByteBuffer* in) {
	uint8_t bl = in->get();
	if(bl > 0){
		CodeElement* element = CodeElement::createFromBinary(in);
		this->packageName = dynamic_cast<PackageNameDeclare*>(element);
	}

	this->className = getString(in);
	AbstractType::fromBinary(in);

	positionFromBinary(in);
}

const UnicodeString* ObjectType::toString() noexcept {
	if(this->str == nullptr){
		this->str = new UnicodeString(L"");

		if(this->packageName != nullptr){
			this->str->append(this->packageName->getName());
			this->str->append(L".");
		}

		this->str->append(getClassName());
	}

	return this->str;
}

void ObjectType::setName(UnicodeString* className) noexcept {
	this->className = className;
}

PackageNameDeclare* ObjectType::getPackageNameDeclare() const noexcept {
	return this->packageName;
}

const UnicodeString* ObjectType::getClassName() const noexcept {
	return this->className;
}

AbstractType* ObjectType::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	{
		AbstractType* inst = getAbstractTypeFromMember(input);
		if(inst != nullptr){
			return inst;
		}
	}

	ObjectType* inst = new ObjectType();
	inst->copyCodePositions(this);

	PackageNameDeclare* pname = this->packageName != nullptr ? this->packageName->generateGenericsImplement(input) : nullptr;
	inst->setPackageName(pname);

	inst->setName(new UnicodeString(this->className));

	return inst;
}

AbstractType* ObjectType::getAbstractTypeFromMember(HashMap<UnicodeString, AbstractType> *input) const {
	AbstractType* inst = nullptr;
	if(this->packageName == nullptr){
		Iterator<UnicodeString>* it = input->keySet()->iterator(); __STP(it);
		while(it->hasNext()){
			const UnicodeString* key = it->next();
			if(this->className->equals(key)){
				AbstractType* type = input->get(key);
				inst = type->generateGenericsImplement(input);
				break;
			}
		}
	}

	return inst;
}

void ObjectType::visit(ITypeVisitor *visitor) {
	visitor->visit(this);
	visitor->exit(this);
}

} /* namespace alinous */
