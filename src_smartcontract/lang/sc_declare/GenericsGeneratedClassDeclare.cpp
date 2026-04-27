/*
 * GenericsGeneratedClassDeclare.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: iizuka
 */

#include "lang/sc_declare/GenericsGeneratedClassDeclare.h"
#include "lang/sc_declare/GenericsClassDeclare.h"

#include "engine/sc_analyze/AnalyzeContext.h"
#include "engine/sc_analyze/AnalyzedClass.h"
#include "engine/sc_analyze/ValidationError.h"
#include "engine/sc_analyze/PackageSpace.h"

#include "engine/sc/CompilationUnit.h"


namespace alinous {

GenericsGeneratedClassDeclare::GenericsGeneratedClassDeclare(short kind) : ClassDeclare(kind) {
	this->genericsClass = nullptr;
	this->factory = nullptr;
}

GenericsGeneratedClassDeclare::GenericsGeneratedClassDeclare() : ClassDeclare(GENERICS_GENERATED_CLASS_DECLARE) {
	this->genericsClass = nullptr;
	this->factory = nullptr;
}

GenericsGeneratedClassDeclare::~GenericsGeneratedClassDeclare() {
	this->genericsClass = nullptr;
	this->factory = nullptr; // factory is singleton
}

void GenericsGeneratedClassDeclare::setGenericsClassDeclare(GenericsClassDeclare* clazz) {
	this->genericsClass = clazz;
}

CodeElement* GenericsGeneratedClassDeclare::getCanonicalCodeElement() noexcept {
	return this->genericsClass;
}

int GenericsGeneratedClassDeclare::binarySize() const {
	int total = ClassDeclare::binarySize();

	return total;
}

void GenericsGeneratedClassDeclare::toBinary(ByteBuffer *out) {
	ClassDeclare::toBinary(out);
}

void GenericsGeneratedClassDeclare::fromBinary(ByteBuffer *in) {
	ClassDeclare::fromBinary(in);
}

void GenericsGeneratedClassDeclare::preAnalyze(AnalyzeContext *actx) {
	ClassDeclare::preAnalyze(actx);
}

void GenericsGeneratedClassDeclare::analyzeTypeRef(AnalyzeContext *actx) {
	ClassDeclare::analyzeTypeRef(actx);
}

void GenericsGeneratedClassDeclare::analyze(AnalyzeContext *actx) {
	ClassDeclare::analyze(actx);
}

const UnicodeString* GenericsGeneratedClassDeclare::getConstructorName() const noexcept {
	return this->genericsClass->getConstructorName();
}

} /* namespace alinous */
