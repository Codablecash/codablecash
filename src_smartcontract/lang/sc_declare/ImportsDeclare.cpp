/*
 * ImportsDeclare.cpp
 *
 *  Created on: 2019/06/17
 *      Author: iizuka
 */

#include "lang/sc_declare/ImportsDeclare.h"
#include "lang/sc_declare/ImportDeclare.h"

#include "engine/sc_analyze/AnalyzeContext.h"

#include "base/UnicodeString.h"


namespace alinous {

ImportsDeclare::ImportsDeclare() : CodeElement(CodeElement::IMPORTS_DECLARE) {

}

ImportsDeclare::~ImportsDeclare() {
	this->list.deleteElements();
}

void ImportsDeclare::addImport(ImportDeclare* imp) noexcept {
	this->list.addElement(imp);
}


void ImportsDeclare::preAnalyze(AnalyzeContext* actx) {
	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		ImportDeclare* imp = this->list.get(i);
		imp->setParent(this);
		imp->preAnalyze(actx);
	}
}


int ImportsDeclare::binarySize() const {
	int total = sizeof(uint16_t);

	total += sizeof(uint32_t);
	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		ImportDeclare* imp = this->list.get(i);
		total += imp->binarySize();
	}

	total += positionBinarySize();

	return total;
}

void ImportsDeclare::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::IMPORTS_DECLARE);

	int maxLoop = this->list.size();
	out->putInt(maxLoop);
	for(int i = 0; i != maxLoop; ++i){
		ImportDeclare* imp = this->list.get(i);
		imp->toBinary(out);
	}

	positionToBinary(out);
}

void ImportsDeclare::fromBinary(ByteBuffer* in) {
	int maxLoop = in->getInt();
	for(int i = 0; i != maxLoop; ++i){
		CodeElement* element = CodeElement::createFromBinary(in);
		CodeElement::checkIsImport(element);

		this->list.addElement(dynamic_cast<ImportDeclare*>(element));
	}

	positionFromBinary(in);
}

const ArrayList<ImportDeclare>* ImportsDeclare::getImports() const noexcept {
	return &this->list;
}

ImportsDeclare* ImportsDeclare::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) {
	ImportsDeclare* inst = new ImportsDeclare();
	inst->copyCodePositions(this);

	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		ImportDeclare* imp = this->list.get(i);
		ImportDeclare* copied = imp->generateGenericsImplement(input);

		inst->addImport(copied);
	}

	return inst;
}

void ImportsDeclare::replaceImport(const UnicodeString *from, const UnicodeString *to) {
	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		ImportDeclare* imp = this->list.get(i);
		const UnicodeString* className = imp->getClassName();

		if(from->equals(className)){
			imp->setClassName(new UnicodeString(to));
			break;
		}
	}
}
/*
bool ImportsDeclare::hasImportClass(const UnicodeString *fqn) {
	bool ret = false;

	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		ImportDeclare* imp = this->list.get(i);
		const UnicodeString* className = imp->getClassName();

		if(fqn->equals(className)){
			ret = true;
		}
	}

	return ret;
}*/

} /* namespace alinous */
