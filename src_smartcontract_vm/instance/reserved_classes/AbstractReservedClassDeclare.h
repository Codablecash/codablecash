/*
 * AbstractReservedClassDeclare.h
 *
 *  Created on: 2020/03/17
 *      Author: iizuka
 */

#ifndef RESERVED_CLASSES_ABSTRACTRESERVEDCLASSDECLARE_H_
#define RESERVED_CLASSES_ABSTRACTRESERVEDCLASSDECLARE_H_

#include "lang/sc_declare/ClassDeclare.h"


namespace alinous {

class AbstractReservedClassDeclare : public ClassDeclare {
public:
	static constexpr const uint16_t TYPE_STRING_CLASS = 1;
	static constexpr const uint16_t TYPE_ARRAY_OUT_OF_BOUNDS_EXCEPTION = 2;
	static constexpr const uint16_t TYPE_DATABASE_EXCEPTION = 3;
	static constexpr const uint16_t TYPE_EXCEPTION = 4;
	static constexpr const uint16_t TYPE_NULL_POINTER_EXCEPTION = 5;
	static constexpr const uint16_t TYPE_TYPE_CAST_EXCEPTION = 6;
	static constexpr const uint16_t TYPE_ZERO_DIVISION_EXCEPTION = 7;
	static constexpr const uint16_t TYPE_INTER_MODULE_ACCESS_EXCEPTION = 8;

	static constexpr const uint16_t TYPE_OBJECT = 10;

	static constexpr const uint16_t TYPE_MODULAR_PROXY = 11;
	static constexpr const uint16_t TYPE_MODULAR_PROXY_LISTNER = 12;

	AbstractReservedClassDeclare();
	virtual ~AbstractReservedClassDeclare();

	static AbstractReservedClassDeclare* createFromBinary(ByteBuffer* in);

	virtual bool isReserved() const noexcept {
		return true;
	}

	virtual void preAnalyze(AnalyzeContext* actx);
	virtual void analyzeTypeRef(AnalyzeContext* actx);
	virtual void analyze(AnalyzeContext* actx);

	virtual void init(VirtualMachine *vm);

	virtual ArrayList<MethodDeclare>* getMethods() noexcept;
	virtual ArrayList<MemberVariableDeclare>* getMemberVariables() noexcept;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	virtual ClassDeclare* generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input);

	virtual uint16_t getClassType() const noexcept = 0;

	void addMethod(MethodDeclare* method) noexcept;

protected:
	void addDefaultConstructor(const UnicodeString* className) noexcept;

protected:
	ArrayList<MethodDeclare>* methods;
	ArrayList<MemberVariableDeclare>* members;
};

} /* namespace alinous */

#endif /* RESERVED_CLASSES_ABSTRACTRESERVEDCLASSDECLARE_H_ */
