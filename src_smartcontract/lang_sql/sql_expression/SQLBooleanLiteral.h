/*
 * SQLBooleanLiteral.h
 *
 *  Created on: 2019/02/16
 *      Author: iizuka
 */

#ifndef SQL_EXPRESSION_SQLBOOLEANLITERAL_H_
#define SQL_EXPRESSION_SQLBOOLEANLITERAL_H_

#include "lang_sql/sql/AbstractSQLExpression.h"

namespace alinous {

class SQLBooleanLiteral : public AbstractSQLExpression {
public:
	SQLBooleanLiteral();
	virtual ~SQLBooleanLiteral();

	void setValue(bool value) noexcept;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out);
	virtual void fromBinary(ByteBuffer* in);

	virtual void preAnalyze(AnalyzeContext* actx);
	virtual void analyzeTypeRef(AnalyzeContext* actx);
	virtual void analyze(AnalyzeContext* actx);
	virtual AnalyzedType getType(AnalyzeContext* actx);

	virtual void init(VirtualMachine* vm);
	virtual AbstractVmInstance* interpret(VirtualMachine* vm);
	virtual void onSelectTarget(VirtualMachine* vm);
private:
	void interpretOnPlanning(VirtualMachine* vm);
private:
	bool value;
};

} /* namespace alinous */

#endif /* SQL_EXPRESSION_SQLBOOLEANLITERAL_H_ */
