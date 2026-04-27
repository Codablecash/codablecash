/*
 * ReservedGeneratedGenericsClassDeclare.h
 *
 *  Created on: Nov 7, 2025
 *      Author: iizuka
 */

#ifndef INSTANCE_RESERVED_GENERICS_RESERVEDGENERATEDGENERICSCLASSDECLARE_H_
#define INSTANCE_RESERVED_GENERICS_RESERVEDGENERATEDGENERICSCLASSDECLARE_H_

#include "lang/sc_declare/GenericsGeneratedClassDeclare.h"

namespace alinous {

class ReservedGeneratedGenericsClassDeclare : public GenericsGeneratedClassDeclare {
public:
	ReservedGeneratedGenericsClassDeclare();
	virtual ~ReservedGeneratedGenericsClassDeclare();

	virtual bool isReserved() const noexcept {
		return true;
	}

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out);
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
	void addMemberVariable(MemberVariableDeclare* variable) noexcept;

protected:
	ArrayList<MethodDeclare>* methods;
	ArrayList<MemberVariableDeclare>* members;
};

} /* namespace alinous */

#endif /* INSTANCE_RESERVED_GENERICS_RESERVEDGENERATEDGENERICSCLASSDECLARE_H_ */
