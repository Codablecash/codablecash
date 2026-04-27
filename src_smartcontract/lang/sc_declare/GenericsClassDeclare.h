/*
 * GenericsClassDeclare.h
 *
 *  Created on: Oct 18, 2025
 *      Author: iizuka
 */

#ifndef LANG_SC_DECLARE_GENERICSCLASSDECLARE_H_
#define LANG_SC_DECLARE_GENERICSCLASSDECLARE_H_

#include "lang/sc_declare/ClassDeclare.h"

#include "base/ArrayList.h"
#include "base/HashMap.h"

namespace alinous {

class GenericsParameter;
class GenericsGeneratedClassDeclare;
class AbstractType;

class GenericsClassDeclare : public ClassDeclare {
public:
	GenericsClassDeclare(short kind);
	GenericsClassDeclare();
	virtual ~GenericsClassDeclare();

	virtual bool isGenerics() const noexcept {
		return true;
	}

	void addGenericsParameter(GenericsParameter* param) noexcept;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	virtual void preAnalyze(AnalyzeContext* actx);
	virtual void analyzeTypeRef(AnalyzeContext* actx);
	virtual void analyze(AnalyzeContext* actx);

	const GenericsParameter* findGenericsType(const UnicodeString* name) const noexcept;

	ArrayList<GenericsParameter>* getParams() const noexcept {
		return this->genericsParams;
	}

	virtual ClassDeclare* generateGenericsImplement(HashMap<UnicodeString, AbstractType>* input);

	virtual void init(VirtualMachine* vm);

protected:
	ArrayList<GenericsParameter>* genericsParams;

};

} /* namespace alinous */

#endif /* LANG_SC_DECLARE_GENERICSCLASSDECLARE_H_ */
