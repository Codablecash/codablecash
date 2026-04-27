/*
 * ClassDeclare.h
 *
 *  Created on: 2019/01/15
 *      Author: iizuka
 */

#ifndef SC_DECLARE_CLASSDECLARE_H_
#define SC_DECLARE_CLASSDECLARE_H_

#include "engine/sc/CodeElement.h"

#include "base/ArrayList.h"
#include "base/HashMap.h"


namespace alinous {

class ClassDeclareBlock;
class UnicodeString;
class AnalyzeContext;
class ClassExtends;
class ClassImplements;
class VirtualMachine;
class AnalyzedType;
class MethodDeclare;
class MemberVariableDeclare;
class AnalyzedClass;
class VTableRegistory;
class VTableClassEntry;
class IVmInstanceFactory;
class AbstractType;

class ClassDeclare : public CodeElement {
public:
	ClassDeclare();
	ClassDeclare(short kind);
	virtual ~ClassDeclare();

	void setInterface(bool interface) noexcept;
	bool isInterface() const noexcept;

	virtual bool isGenerics() const noexcept {
		return false;
	}
	virtual bool isReserved() const noexcept {
		return false;
	}

	virtual void preAnalyze(AnalyzeContext* actx);
	virtual void analyzeTypeRef(AnalyzeContext* actx);
	virtual void analyze(AnalyzeContext* actx);

	virtual void init(VirtualMachine* vm);

	void setBlock(ClassDeclareBlock* block) noexcept;
	void setName(UnicodeString* name) noexcept;
	void setExtends(ClassExtends* extends) noexcept;
	void setImplements(ClassImplements* implements) noexcept;

	ClassExtends* getExtends() const noexcept;

	virtual const UnicodeString* getConstructorName() const noexcept;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	virtual ClassDeclare* generateGenericsImplement(HashMap<UnicodeString, AbstractType>* input);

	ClassImplements* getImplements() const noexcept {
		return this->implements;
	}

protected:
	void toBinaryCheck(ByteBuffer* out) const;
	void toBinaryHead(ByteBuffer* out) const;
	void toBinaryBody(ByteBuffer* out) const;

public:
	virtual ClassDeclare* getBaseClass() const noexcept;
	int getInheritIndex() const noexcept;
	void setInheritIndex(int inheritIndex) noexcept;

	virtual const UnicodeString* getName() const noexcept;
	virtual const UnicodeString* getFullQualifiedName() noexcept;

	virtual ArrayList<MethodDeclare>* getMethods() noexcept;
	virtual ArrayList<MemberVariableDeclare>* getMemberVariables() noexcept;

	virtual IVmInstanceFactory* getFactory() const noexcept;

private:
	void checkImplementsInterfaces(AnalyzeContext* actx);
	void checkImplementsInterface(AnalyzeContext* actx, AnalyzedClass* aclass, VTableRegistory* vreg, VTableClassEntry* thisEntry);
	bool isImplemented(VTableClassEntry* thisEntry, MethodDeclare* method);
	void addDefaultConstructor() noexcept;

protected:
	bool interface;
	ClassDeclareBlock* block;
	UnicodeString* name;
	ClassExtends* extends;
	ClassImplements* implements;

	int inheritIndex;
	UnicodeString* fqn;
};

} /* namespace alinous */

#endif /* SC_DECLARE_CLASSDECLARE_H_ */
