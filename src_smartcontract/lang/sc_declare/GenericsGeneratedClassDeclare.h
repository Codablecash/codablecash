/*
 * GenericsGeneratedClassDeclare.h
 *
 *  Created on: Oct 25, 2025
 *      Author: iizuka
 */

#ifndef LANG_SC_DECLARE_GENERICSGENERATEDCLASSDECLARE_H_
#define LANG_SC_DECLARE_GENERICSGENERATEDCLASSDECLARE_H_

#include "lang/sc_declare/ClassDeclare.h"

namespace alinous {

class GenericsClassDeclare;

class GenericsGeneratedClassDeclare : public ClassDeclare {
public:
	GenericsGeneratedClassDeclare(short kind);
	GenericsGeneratedClassDeclare();
	virtual ~GenericsGeneratedClassDeclare();

	void setGenericsClassDeclare(GenericsClassDeclare* clazz);

	virtual CodeElement* getCanonicalCodeElement() noexcept;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out);
	virtual void fromBinary(ByteBuffer* in);

	virtual void preAnalyze(AnalyzeContext* actx);
	virtual void analyzeTypeRef(AnalyzeContext* actx);
	virtual void analyze(AnalyzeContext* actx);

	virtual const UnicodeString* getConstructorName() const noexcept;

	virtual IVmInstanceFactory* getFactory() const noexcept {
		return this->factory;
	}
	void setFactory(IVmInstanceFactory* factory){
		this->factory = factory;
	}

protected:
	GenericsClassDeclare* genericsClass;
	IVmInstanceFactory* factory;
};

} /* namespace alinous */

#endif /* LANG_SC_DECLARE_GENERICSGENERATEDCLASSDECLARE_H_ */
