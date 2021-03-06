/*
 * test_exp_logical.cpp
 *
 *  Created on: 2019/03/01
 *      Author: iizuka
 */

#include "test_utils/t_macros.h"
#include "engine/compiler/SmartContractParser.h"

#include "base/StackRelease.h"
#include "lang/sc_expression_literal/NullLiteral.h"

#include "alinous_lang/AlinousLang.h"

static bool checkBinary(ByteBuffer* buff){
	int lastSize = buff->capacity();

	buff->position(0);
	CodeElement* element = CodeElement::createFromBinary(buff); __STP(element);

	int size = element->binarySize();
	if(lastSize != size){
		return false;
	}

	ByteBuffer* buff2 = ByteBuffer::allocateWithEndian(size, true); __STP(buff2);
	element->toBinary(buff2);

	if(buff2->position() != size){
		return false;
	}

	return Mem::memcmp(buff->array(), buff2->array(), size) == 0;
}

TEST_GROUP(TestExpressionLogicalGroup) {
	TEST_SETUP(){}
	TEST_TEARDOWN(){}
};

TEST(TestExpressionLogicalGroup, and01){
	const File* projectFolder = this->env->getProjectRoot();
	_ST(File, sourceFile, projectFolder->get(L"src_test/smartcontract/resources/expression_logical/and01.alns"))

	SmartContractParser parser(sourceFile);
	AlinousLang* lang = parser.getDebugAlinousLang();

	AbstractExpression* exp = lang->expression(); __STP(exp);

	CHECK(!parser.hasError())
}

TEST(TestExpressionLogicalGroup, and01bin){
	const File* projectFolder = this->env->getProjectRoot();
	_ST(File, sourceFile, projectFolder->get(L"src_test/smartcontract/resources/expression_logical/and01.alns"))

	SmartContractParser parser(sourceFile);
	AlinousLang* lang = parser.getDebugAlinousLang();

	AbstractExpression* exp = lang->expression(); __STP(exp);

	CHECK(!parser.hasError())

	int size = exp->binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(size, true); __STP(buff);

	exp->toBinary(buff);
	CHECK(buff->position() == size)

	bool res = checkBinary(buff);
	CHECK(res)
}

TEST(TestExpressionLogicalGroup, or01){
	const File* projectFolder = this->env->getProjectRoot();
	_ST(File, sourceFile, projectFolder->get(L"src_test/smartcontract/resources/expression_logical/or01.alns"))

	SmartContractParser parser(sourceFile);
	AlinousLang* lang = parser.getDebugAlinousLang();

	AbstractExpression* exp = lang->expression(); __STP(exp);

	CHECK(!parser.hasError())
}

TEST(TestExpressionLogicalGroup, or01bin){
	const File* projectFolder = this->env->getProjectRoot();
	_ST(File, sourceFile, projectFolder->get(L"src_test/smartcontract/resources/expression_logical/or01.alns"))

	SmartContractParser parser(sourceFile);
	AlinousLang* lang = parser.getDebugAlinousLang();

	AbstractExpression* exp = lang->expression(); __STP(exp);

	CHECK(!parser.hasError())

	int size = exp->binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(size, true); __STP(buff);

	exp->toBinary(buff);
	CHECK(buff->position() == size)

	bool res = checkBinary(buff);
	CHECK(res)
}

