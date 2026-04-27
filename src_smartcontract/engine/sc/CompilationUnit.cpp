/*
 * CompilationUnit.cpp
 *
 *  Created on: 2019/01/15
 *      Author: iizuka
 */

#include "engine/sc/CompilationUnit.h"

#include "lang/sc_declare/ClassDeclare.h"
#include "lang/sc_declare/PackageDeclare.h"
#include "lang/sc_declare/ImportsDeclare.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

namespace alinous {

CompilationUnit::CompilationUnit() : classes(4), CodeElement(CodeElement::COMPILANT_UNIT) {
	this->package = nullptr;
	this->imports = new ImportsDeclare();
	this->projectRelativePath = nullptr;
}

CompilationUnit::~CompilationUnit() {
	delete this->package;
	delete this->imports;

	this->classes.deleteElements();

	delete this->projectRelativePath;
}

void CompilationUnit::preAnalyze(AnalyzeContext* actx) {
	if(this->imports != nullptr){
		this->imports->setParent(this);
		this->imports->preAnalyze(actx);
	}

	int maxLoop = this->classes.size();
	for(int i = 0; i != maxLoop; ++i){
		ClassDeclare* dec = this->classes.get(i);
		dec->setParent(this);
		dec->preAnalyze(actx);
	}
}

void CompilationUnit::analyzeType(AnalyzeContext* actx) {
	int maxLoop = this->classes.size();
	for(int i = 0; i != maxLoop; ++i){
		ClassDeclare* dec = this->classes.get(i);
		dec->analyzeTypeRef(actx);
	}

}

void CompilationUnit::analyze(AnalyzeContext* actx) {
	int maxLoop = this->classes.size();
	for(int i = 0; i != maxLoop; ++i){
		ClassDeclare* dec = this->classes.get(i);
		dec->analyze(actx);
	}
}

void CompilationUnit::init(VirtualMachine* vm) {
	int maxLoop = this->classes.size();
	for(int i = 0; i != maxLoop; ++i){
		ClassDeclare* dec = this->classes.get(i);
		dec->init(vm);
	}
}


void CompilationUnit::setPackage(PackageDeclare* package) {
	this->package = package;
}


const UnicodeString* CompilationUnit::getPackageName() noexcept {
	if(this->package == nullptr){
		return nullptr;
	}
	return this->package->getPackageName();
}


void CompilationUnit::setImports(ImportsDeclare* imports) noexcept {
	delete this->imports;
	this->imports = imports;
}

void CompilationUnit::addClassDeclare(ClassDeclare* clazz) {
	this->classes.addElement(clazz);
}

ClassDeclare* CompilationUnit::getClassDeclare(int pos) const noexcept {
	return this->classes.get(pos);
}

ImportsDeclare* CompilationUnit::getImportDeclare() const noexcept {
	return this->imports;
}


int CompilationUnit::binarySize() const {
	int total = sizeof(uint16_t);

	total += sizeof(char);
	if(this->package){
		total += this->package->binarySize();
	}

	total += sizeof(char);
	if(this->imports){
		total += this->imports->binarySize();
	}

	total += sizeof(uint32_t);
	int maxLoop = this->classes.size();
	for(int i = 0; i != maxLoop; ++i){
		ClassDeclare* dec = this->classes.get(i);
		total += dec->binarySize();
	}

	total += sizeof(char);
	if(this->projectRelativePath != nullptr){
		total += stringSize(this->projectRelativePath);
	}

	total += positionBinarySize();

	return total;
}

void CompilationUnit::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::COMPILANT_UNIT);

	char bl = this->package != nullptr ? (char)1 : (char)0;
	out->put(bl);

	if(this->package != nullptr){
		this->package->toBinary(out);
	}

	bl = this->imports != nullptr ? (char)1 : (char)0;
	out->put(bl);

	if(this->imports != nullptr){
		this->imports->toBinary(out);
	}

	uint32_t maxLoop = this->classes.size();
	out->putInt(maxLoop);
	for(int i = 0; i != maxLoop; ++i){
		ClassDeclare* dec = this->classes.get(i);
		dec->toBinary(out);
	}

	bl = (this->projectRelativePath != nullptr);
	out->put(bl ? 1 : 0);
	if(bl > 0){
		putString(out, this->projectRelativePath);
	}

	positionToBinary(out);
}

void CompilationUnit::fromBinary(ByteBuffer* in) {
	char bl = in->get();
	if(bl == (char)1){
		CodeElement* element = createFromBinary(in);
		checkKind(element, CodeElement::PACKAGE_DECLARE);
		this->package = dynamic_cast<PackageDeclare*>(element);
	}

	bl = in->get();
	if(bl == (char)1){
		CodeElement* element = createFromBinary(in);
		checkKind(element, CodeElement::IMPORTS_DECLARE);

		delete this->imports;
		this->imports = dynamic_cast<ImportsDeclare*>(element);
	}

	int maxLoop = in->getInt();
	for(int i = 0; i != maxLoop; ++i){
		CodeElement* element = createFromBinary(in);
		checkKind(element, CodeElement::CLASS_DECLARE);

		ClassDeclare* dec = dynamic_cast<ClassDeclare*>(element);
		this->classes.addElement(dec);
	}

	bl = in->get();
	if(bl > 0){
		this->projectRelativePath = getString(in);
	}

	positionFromBinary(in);
}

CompilationUnit* CompilationUnit::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) {
	CompilationUnit* inst = new CompilationUnit();

	if(this->package != nullptr){
		PackageDeclare* pkg = this->package->generateGenericsImplement(input);
		inst->setPackage(pkg);
	}

	if(this->imports != nullptr){
		ImportsDeclare* imports = this->imports->generateGenericsImplement(input);
		inst->setImports(imports);
	}

	int maxLoop = this->classes.size();
	for(int i = 0; i != maxLoop; ++i){
		ClassDeclare* clazz = this->classes.get(i);
		ClassDeclare* copied = clazz->generateGenericsImplement(input);

		inst->addClassDeclare(copied);
	}

	return inst;
}

ClassDeclare* CompilationUnit::getClassDeclare(const UnicodeString *name) const noexcept {
	ClassDeclare* ret = nullptr;

	int maxLoop = this->classes.size();
	for(int i = 0; i != maxLoop; ++i){
		ClassDeclare* clazz = this->classes.get(i);
		const UnicodeString* className = clazz->getName();

		if(className->equals(name)){
			ret = clazz;
			break;
		}
	}

	return ret;
}

void CompilationUnit::setProjectRelativePath(const UnicodeString *path) noexcept {
	delete this->projectRelativePath;
	this->projectRelativePath = new UnicodeString(path);
}

CompilationUnit* CompilationUnit::copy() const {
	int cap = binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);

	toBinary(buff);

	buff->position(0);
	CodeElement* element = CodeElement::createFromBinary(buff); __STP(element);
	CompilationUnit* unit = dynamic_cast<CompilationUnit*>(element);

	checkNotNull(unit);
	__STP_MV(element);

	return unit;
}

} /* namespace alinous */
