/*
 * JsonParser.cpp
 *
 *  Created on: Sep 26, 2025
 *      Author: iizuka
 */

#include "json/JsonParser.h"
#include "json/AbstractJsonToken.h"
#include "json/JsonTokenizer.h"
#include "json/JsonParseException.h"
#include "json/StackJsonToknizerForeseeReleaser.h"

#include "json_object/JsonObject.h"
#include "json_object/JsonArrayObject.h"
#include "json_object/JsonNumericValue.h"
#include "json_object/JsonStringValue.h"
#include "json_object/JsonBooleanValue.h"
#include "json_object/JsonValuePair.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"
#include "base/Integer.h"

using alinous::Integer;
using alinous::UnicodeString;

namespace codablecash {

JsonParser::JsonParser(JsonTokenizer* tokenizer) {
	this->tokenizer = tokenizer;
}

JsonParser::~JsonParser() {
	this->tokenizer = nullptr;
}

AbstractJsonObject* JsonParser::parse() {
	return allObjects();
}

AbstractJsonObject* JsonParser::allObjects() {
	AbstractJsonObject* retObject = nullptr;

	if(testJsonObject(false)){
		retObject = jsonObject();
	}
	else if(testJsonArrayObject(false)){
		retObject = jsonArrayObject();
	}
	else if(testAbstractJsonValue(false)){
		retObject = abstractJsonValue();
	}
	else{
		UnicodeString message(L"Unexpected token error.");
		throw new JsonParseException(message.towString(), __FILE__, __LINE__);
	}

	return retObject;
}

bool JsonParser::testAllObjects(bool consume) {
	bool bl = false;

	// JsonObject
	{
		StackJsonToknizerForeseeReleaser __stack(this->tokenizer, consume);
		bl = testJsonObject(false);
	}
	if(bl){
		if(consume){
			testJsonObject(true);
		}
		return true;
	}

	// JsonArrayObject
	{
		StackJsonToknizerForeseeReleaser __stack(this->tokenizer, consume);
		bl = testJsonArrayObject(false);
	}
	if(bl){
		if(consume){
			testJsonArrayObject(true);
		}
		return true;
	}

	// AbstractJsonValue
	{
		StackJsonToknizerForeseeReleaser __stack(this->tokenizer, consume);
		bl = testAbstractJsonValue(false);
	}
	if(bl){
		if(consume){
			testAbstractJsonValue(true);
		}
		return true;
	}

	return false;
}

JsonObject* JsonParser::jsonObject() {
	JsonObject* obj = new JsonObject(); __STP(obj);

	{
		AbstractJsonToken* token = getToken(); __STP(token); // BRACE_BEGIN
		checkTokenType(token, TokenType::BRACE_BEGIN, L"'{'");
	}

	bool readFinalToken = false;
	while(true){
		{
			StackJsonToknizerForeseeReleaser __stack(this->tokenizer, false);
			const AbstractJsonToken* token = foreseeToken();
			if(token->getType() == TokenType::BRACE_END){
				readFinalToken = true;
				break;
			}
		}

		JsonValuePair* pair = jsonValuePair();
		obj->add(pair);

		{
			AbstractJsonToken* token = getToken(); __STP(token); // BRACE_END
			if(token->getType() == TokenType::BRACE_END){
				break;
			}

			// check comma
			checkTokenType(token, TokenType::COMMA, L"COMMA");
		}
	}

	if(readFinalToken){
		AbstractJsonToken* token = getToken(); __STP(token); // BRACE_END
	}

	return __STP_MV(obj);
}

bool JsonParser::testJsonObject(bool consume) {
	StackJsonToknizerForeseeReleaser __stack(this->tokenizer, consume);

	const AbstractJsonToken* token = foreseeToken();
	if(token->getType() != TokenType::BRACE_BEGIN){
		return false;
	}

	while(true){
		bool bl = testJsonValuePair(true);
		if(!bl){
			token = foreseeToken();
			if(token->getType() == TokenType::BRACE_END){
				break;
			}

			return false;
		}

		{
			token = foreseeToken();
			if(token->getType() == TokenType::BRACE_END){
				break;
			}

			// check comma
			if(token->getType() != TokenType::COMMA){
				return false;
			}
		}
	}

	return true;
}

JsonArrayObject* JsonParser::jsonArrayObject() {
	JsonArrayObject* arrayObj = new JsonArrayObject(); __STP(arrayObj);

	{
		AbstractJsonToken* token = getToken(); __STP(token); // BRACKET_BEGIN
		checkTokenType(token, TokenType::BRACKET_BEGIN, L"'['");
	}

	bool readFinalToken = false;
	while(true){
		{
			StackJsonToknizerForeseeReleaser __stack(this->tokenizer, false);
			const AbstractJsonToken* token = foreseeToken();
			if(token->getType() == TokenType::BRACKET_END){
				readFinalToken = true;
				break;
			}
		}

		AbstractJsonObject* obj = allObjects();
		arrayObj->add(obj);

		{
			AbstractJsonToken* token = getToken(); __STP(token); // BRACKET_END
			if(token->getType() == TokenType::BRACKET_END){
				break;
			}

			// check comma
			checkTokenType(token, TokenType::COMMA, L"COMMA");
		}
	}

	if(readFinalToken){
		AbstractJsonToken* token = getToken(); __STP(token); // BRACKET_END
	}

	return __STP_MV(arrayObj);
}

bool JsonParser::testJsonArrayObject(bool consume) {
	StackJsonToknizerForeseeReleaser __stack(this->tokenizer, consume);

	const AbstractJsonToken* token = foreseeToken();
	if(token->getType() != TokenType::BRACKET_BEGIN){
		return false;
	}

	while(true){
		bool bl = testAllObjects(true);
		if(!bl){
			token = foreseeToken();
			if(token->getType() == TokenType::BRACKET_END){
				break;
			}

			return false;
		}

		{
			token = foreseeToken();
			if(token->getType() == TokenType::BRACKET_END){
				break;
			}

			// check comma
			if(token->getType() != TokenType::COMMA){
				return false;
			}
		}
	}

	return true;
}

JsonValuePair* JsonParser::jsonValuePair() {
	JsonValuePair* pair = new JsonValuePair(); __STP(pair);

	{
		AbstractJsonValue* value = abstractJsonValue();
		pair->setKey(value);
	}

	{
		AbstractJsonToken* token = getToken(); __STP(token); // COLON
		checkTokenType(token, TokenType::COLON, L"':'");
	}

	{
		AbstractJsonObject* obj = allObjects();
		pair->setValue(obj);
	}

	return __STP_MV(pair);
}

bool JsonParser::testJsonValuePair(bool consume) {
	StackJsonToknizerForeseeReleaser __stack(this->tokenizer, consume);

	const AbstractJsonToken* token = nullptr;

	bool bl = testAbstractJsonValue(true);
	if(!bl){
		return false;
	}

	token = foreseeToken();
	if(token->getType() != TokenType::COLON){
		return false;
	}

	bl = testAllObjects(true);
	if(!bl){
		return false;
	}

	return true;
}

AbstractJsonValue* JsonParser::abstractJsonValue() {
	bool bl = false;

	{
		StackJsonToknizerForeseeReleaser __stack(this->tokenizer, false);
		bl = testJsonStringValue(false);
	}
	if(bl){
		return jsonStringValue();
	}

	{
		StackJsonToknizerForeseeReleaser __stack(this->tokenizer, false);
		bl = testJsonBooleanValue(false);
	}
	if(bl){
		return jsonBooleanValue();
	}

	{
		StackJsonToknizerForeseeReleaser __stack(this->tokenizer, false);
		bl = testJsonNumericValue(false);
	}
	if(bl){
		return jsonNumericValue();
	}

	UnicodeString message(L"String value, Boolean value, or Numeric value is expected.");
	throw new JsonParseException(message.towString(), __FILE__, __LINE__);
}

bool JsonParser::testAbstractJsonValue(bool consume) {
	bool bl = false;
	{
		StackJsonToknizerForeseeReleaser __stack(this->tokenizer, false);
		bl = testJsonStringValue(false);
	}
	if(bl){
		if(consume){
			testJsonStringValue(true);
		}
		return true;
	}

	{
		StackJsonToknizerForeseeReleaser __stack(this->tokenizer, false);
		bl = testJsonBooleanValue(false);
	}
	if(bl){
		if(consume){
			testJsonBooleanValue(true);
		}
		return true;
	}

	{
		StackJsonToknizerForeseeReleaser __stack(this->tokenizer, false);
		bl = testJsonNumericValue(false);
	}
	if(bl){
		if(consume){
			testJsonNumericValue(true);
		}
		return true;
	}

	return false;
}

JsonStringValue* JsonParser::jsonStringValue() {
	AbstractJsonToken* token = getToken(); __STP(token);
	const UnicodeString* str = token->getBodyString();
	UnicodeString* bodyStr = str->substring(1, str->length() - 1); __STP(bodyStr);


	JsonStringValue* value = new JsonStringValue();
	value->setValue(bodyStr);

	return value;
}

bool JsonParser::testJsonStringValue(bool consume) {
	StackJsonToknizerForeseeReleaser __stack(this->tokenizer, consume);

	const AbstractJsonToken* token = foreseeToken();

	return token->getType() == TokenType::STRING;
}

JsonBooleanValue* JsonParser::jsonBooleanValue() {
	AbstractJsonToken* token = getToken(); __STP(token);
	const UnicodeString* str = token->getBodyString();

	JsonBooleanValue* value = new JsonBooleanValue();
	value->setValue(str->equals(L"true"));

	return value;
}

bool JsonParser::testJsonBooleanValue(bool consume) {
	StackJsonToknizerForeseeReleaser __stack(this->tokenizer, consume);
	const AbstractJsonToken* token = foreseeToken();

	return token->getType() == TokenType::BOOLEAN;
}

JsonNumericValue* JsonParser::jsonNumericValue() {
	AbstractJsonToken* token = getToken(); __STP(token);
	const UnicodeString* str = token->getBodyString();

	int numberValue = Integer::parseInt(str);

	JsonNumericValue* value = new JsonNumericValue();
	value->setValue(numberValue);

	return value;
}

bool JsonParser::testJsonNumericValue(bool consume) {
	StackJsonToknizerForeseeReleaser __stack(this->tokenizer, consume);

	const AbstractJsonToken* token = foreseeToken();

	return token->getType() == TokenType::NUMERIC;
}

const AbstractJsonToken* JsonParser::foreseeToken() {
	checkNotEof();
	const AbstractJsonToken* token = this->tokenizer->foreseeToken();

	while(token->getType() == TokenType::NONE){
		checkNotEof();
		token = this->tokenizer->foreseeToken();
	}

	return token;
}

AbstractJsonToken* JsonParser::getToken() {
	checkNotEof();
	AbstractJsonToken* token = this->tokenizer->getToken();

	while(token->getType() == TokenType::NONE){
		delete token;

		checkNotEof();
		token = this->tokenizer->getToken();
	}

	return token;
}

void JsonParser::checkNotEof() {
	if(this->tokenizer->isEndOfFile()){
		throw new JsonParseException(L"Unexpected end of file.", __FILE__, __LINE__);
	}
}

void JsonParser::checkTokenType(const AbstractJsonToken *token, TokenType type, const wchar_t *required) {
	if(token->getType() != type){
		UnicodeString msg(L"Unexpected token. ");
		msg.append(required);
		msg.append(L" is required, at line : ");
		msg.append((int64_t)token->getLine());

		throw new JsonParseException(msg.towString(), __FILE__, __LINE__);
	}
}

} /* namespace codablecash */
