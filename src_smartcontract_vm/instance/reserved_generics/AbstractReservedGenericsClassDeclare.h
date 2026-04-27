/*
 * AbstractReservedGenericsClassDeclare.h
 *
 *  Created on: Nov 6, 2025
 *      Author: iizuka
 */

#ifndef INSTANCE_RESERVED_GENERICS_ABSTRACTRESERVEDGENERICSCLASSDECLARE_H_
#define INSTANCE_RESERVED_GENERICS_ABSTRACTRESERVEDGENERICSCLASSDECLARE_H_

#include "lang/sc_declare/GenericsClassDeclare.h"

namespace alinous {

class ReservedGeneratedGenericsClassDeclare;

class AbstractReservedGenericsClassDeclare : public GenericsClassDeclare {
public:
	static constexpr const uint16_t TYPE_GENERICS_LIST_CLASS = 1;

	AbstractReservedGenericsClassDeclare();
	virtual ~AbstractReservedGenericsClassDeclare();

	static AbstractReservedGenericsClassDeclare* createFromBinary(ByteBuffer* in);

	virtual bool isReserved() const noexcept {
		return true;
	}

	virtual uint16_t getGenericsClassType() const noexcept = 0;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	virtual void preAnalyze(AnalyzeContext* actx);
	virtual void analyzeTypeRef(AnalyzeContext* actx);
	virtual void analyze(AnalyzeContext* actx);

	virtual ArrayList<MethodDeclare>* getMethods() noexcept {
		return this->methods;
	}
	virtual ArrayList<MemberVariableDeclare>* getMemberVariables() noexcept {
		return this->members;
	}

	void addMethod(MethodDeclare* method) noexcept;

	virtual void doGenerateGenericsImplement(ReservedGeneratedGenericsClassDeclare* inst, HashMap<UnicodeString, AbstractType> *input) const;

protected:
	ArrayList<MethodDeclare>* methods;
	ArrayList<MemberVariableDeclare>* members;

};

} /* namespace alinous */

#endif /* INSTANCE_RESERVED_GENERICS_ABSTRACTRESERVEDGENERICSCLASSDECLARE_H_ */
