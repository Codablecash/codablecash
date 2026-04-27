/*
 * GenericsObjectType.h
 *
 *  Created on: Oct 20, 2025
 *      Author: iizuka
 */

#ifndef LANG_SC_DECLARE_TYPES_GENERICSOBJECTTYPE_H_
#define LANG_SC_DECLARE_TYPES_GENERICSOBJECTTYPE_H_

#include "lang/sc_declare_types/ObjectType.h"

#include "base/ArrayList.h"


namespace alinous {

class TypeResolver;

class GenericsObjectType : public ObjectType {
public:
	GenericsObjectType();
	explicit GenericsObjectType(const ObjectType* objectType);
	virtual ~GenericsObjectType();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	void addGenericsType(AbstractType* type) noexcept;

	ArrayList<AbstractType>* getGenericTypes() const noexcept {
		return this->genericsType;
	}

	virtual bool isGenericsType() const noexcept {
		return true;
	}

	virtual const UnicodeString* toString() noexcept;

	virtual const UnicodeString* getClassName() const noexcept;
	const UnicodeString* getRawClassName() const noexcept;

	virtual AbstractType* generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const;

	virtual void preAnalyze(AnalyzeContext *actx);
	virtual void analyzeTypeRef(AnalyzeContext* actx);
	virtual void analyze(AnalyzeContext* actx);

	virtual void visit(ITypeVisitor* visitor);

private:
	ArrayList<AbstractType>* genericsType;

	mutable UnicodeString* gclassName;
};

} /* namespace alinous */

#endif /* LANG_SC_DECLARE_TYPES_GENERICSOBJECTTYPE_H_ */
