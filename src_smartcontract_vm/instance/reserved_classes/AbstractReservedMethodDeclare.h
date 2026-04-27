/*
 * AbstractReservedMethodDeclare.h
 *
 *  Created on: Nov 2, 2025
 *      Author: iizuka
 */

#ifndef INSTANCE_RESERVED_CLASSES_ABSTRACTRESERVEDMETHODDECLARE_H_
#define INSTANCE_RESERVED_CLASSES_ABSTRACTRESERVEDMETHODDECLARE_H_

#include "lang/sc_declare/MethodDeclare.h"

namespace alinous {

class AbstractReservedMethodDeclare : public MethodDeclare {
public:
	static constexpr const uint32_t METHOD_OBJECT_OBJECT = 1;
	static constexpr const uint32_t METHOD_MODULAR_INTERFACE = 2;

	static constexpr const uint32_t METHOD_LIST_LIST = 100;

	static constexpr const uint32_t METHOD_MODULE_PROXY_LISTNER_MODULE_DETECTED = 200;

	static constexpr const uint32_t METHOD_STRING_EQUALS = 300;


	explicit AbstractReservedMethodDeclare(uint32_t methodId);
	virtual ~AbstractReservedMethodDeclare();

	static AbstractReservedMethodDeclare* createMethodFromBinary(ByteBuffer* in);

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	void preAnalyze(AnalyzeContext* actx);
	void analyzeTypeRef(AnalyzeContext* actx);
	void analyze(AnalyzeContext* actx);

	virtual void init(VirtualMachine* vm);
	virtual void interpret(FunctionArguments* args, VirtualMachine* vm);

	virtual MethodDeclare* generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const;

protected:
	void interpretFunctionArguments(VirtualMachine* vm);

protected:
	uint32_t methodId;
};

} /* namespace alinous */

#endif /* INSTANCE_RESERVED_CLASSES_ABSTRACTRESERVEDMETHODDECLARE_H_ */
