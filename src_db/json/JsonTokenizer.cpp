/*
 * JsonTokenizer.cpp
 *
 *  Created on: Sep 25, 2025
 *      Author: iizuka
 */

#include "json/JsonTokenizer.h"
#include "json/IJsonStream.h"
#include "json/AbstractJsonToken.h"
#include "json/SkippedToken.h"
#include "json/JsonToken.h"
#include "json/JsonStringToken.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"

#include "json/JsonTokenizeException.h"
namespace codablecash {

const constexpr wchar_t JsonTokenizer::SKIP_CHARS[];
const constexpr wchar_t JsonTokenizer::TOKEN_CHARS[];

JsonTokenizer::JsonTokenizer(IJsonStream* stream) {
	this->stream = stream;
	this->line = 0;
	this->pos = 0;
	this->foreseeList = new ArrayList<AbstractJsonToken>();
	this->foreseeCount = 0;
	this->stack = new RawArrayPrimitive<int>(4);
}

JsonTokenizer::~JsonTokenizer() {
	delete this->stream;

	this->foreseeList->deleteElements();
	delete this->foreseeList;

	delete this->stack;
}

AbstractJsonToken* JsonTokenizer::getToken(bool foresee) {
	if(foresee == false && !this->foreseeList->isEmpty()){
		AbstractJsonToken* ret = this->foreseeList->remove(0);
		return ret;
	}

	wchar_t ch = getChar();
	AbstractJsonToken* retToken = nullptr;

	if(isSkippedToken(ch)){
		SkippedToken* token = new SkippedToken(this->line, this->pos); __STP(token);
		token->addChar(ch);

		handleSkippedToken(token);

		retToken = __STP_MV(token);
	}
	else if(ch == STRING_CHAR){
		JsonStringToken* token = new JsonStringToken(this->line, this->pos); __STP(token);
		token->addChar(ch);

		handleStringToken(token);
		retToken = __STP_MV(token);
	}
	else if(ch == BRACE_BEGIN_CHAR){
		JsonToken* token = new JsonToken(this->line, this->pos); __STP(token);
		token->addChar(ch);
		token->setType(TokenType::BRACE_BEGIN);

		retToken = __STP_MV(token);
	}
	else if(ch == BRACE_END_CHAR){
		JsonToken* token = new JsonToken(this->line, this->pos); __STP(token);
		token->addChar(ch);
		token->setType(TokenType::BRACE_END);

		retToken = __STP_MV(token);
	}
	else if(ch == BRACKET_BEGIN_CHAR){
		JsonToken* token = new JsonToken(this->line, this->pos); __STP(token);
		token->addChar(ch);
		token->setType(TokenType::BRACKET_BEGIN);

		retToken = __STP_MV(token);
	}
	else if(ch == BRACKET_END_CHAR){
		JsonToken* token = new JsonToken(this->line, this->pos); __STP(token);
		token->addChar(ch);
		token->setType(TokenType::BRACKET_END);

		retToken = __STP_MV(token);
	}
	else if(ch == COLON_CHAR){
		JsonToken* token = new JsonToken(this->line, this->pos); __STP(token);
		token->addChar(ch);
		token->setType(TokenType::COLON);

		retToken = __STP_MV(token);
	}
	else if(ch == COMMA_CHAR){
		JsonToken* token = new JsonToken(this->line, this->pos); __STP(token);
		token->addChar(ch);
		token->setType(TokenType::COMMA);

		retToken = __STP_MV(token);
	}
	else{
		JsonToken* token = new JsonToken(this->line, this->pos); __STP(token);
		token->addChar(ch);

		handleToken(token);
		retToken = __STP_MV(token);
	}

	return retToken;
}

AbstractJsonToken* JsonTokenizer::getToken() {
	return getToken(false);
}

bool JsonTokenizer::isSkippedToken(wchar_t ch) noexcept {
	bool ret = false;

	int pos = 0;
	while(SKIP_CHARS[pos] != 0){
		if(SKIP_CHARS[pos++] == ch){
			ret = true;
			break;
		}
	}

	return ret;
}

bool JsonTokenizer::isTokenChar(wchar_t ch) noexcept {
	bool ret = false;

	int pos = 0;
	while(TOKEN_CHARS[pos] != 0){
		if(TOKEN_CHARS[pos++] == ch){
			ret = true;
			break;
		}
	}

	return ret;
}

bool JsonTokenizer::isEndOfFile() const noexcept {
	return this->foreseeCount >= this->foreseeList->size() &&
			this->stream->isEndOfFile();
}

void JsonTokenizer::handleSkippedToken(SkippedToken* token) {
	bool bl = this->stream->isEndOfFile();

	if(bl == false){
		wchar_t ch = this->stream->lookNextChar();

		while((bl == false) && isSkippedToken(ch)){
			ch = getChar();
			token->addChar(ch);

			bl = this->stream->isEndOfFile();
			if(bl == false){
				ch = this->stream->lookNextChar();
			}
		}
	}
}

void JsonTokenizer::handleStringToken(JsonStringToken* token) {
	checkNotEof();
	wchar_t ch = this->stream->lookNextChar();

	while(ch != STRING_CHAR){
		ch = getChar();
		token->addChar(ch);

		checkNotEof();
		ch = this->stream->lookNextChar();
	}

	// final "
	ch = getChar();
	token->addChar(ch);
}

void JsonTokenizer::handleToken(JsonToken* token) {
	bool bl = this->stream->isEndOfFile();

	if(!bl){
		wchar_t ch = this->stream->lookNextChar();

		while(!this->stream->isEndOfFile() && !isSkippedToken(ch) && !isTokenChar(ch)){
			ch = getChar();
			token->addChar(ch);

			if(!this->stream->isEndOfFile()){
				ch = this->stream->lookNextChar();
			}
		}
	}

	// check type
	const UnicodeString* bodyStr = token->getBodyString();
	if(isBoolean(bodyStr)){
		token->setType(TokenType::BOOLEAN);
	}
	else {
		token->setType(TokenType::NUMERIC);
	}
}

bool JsonTokenizer::isBoolean(const UnicodeString *bodyStr) {
	return bodyStr->equals(TRUE) || bodyStr->equals(FALSE);
}

wchar_t JsonTokenizer::getChar() {
	wchar_t ch = this->stream->getChar();
	this->pos++;

	if(ch == NEWLINE_CHAR){
		this->pos = 0;
		this->line++;
	}

	if(ch == ESCAPED_CHAR){
		checkNotEof();
		ch = this->stream->getChar();
		this->pos++;
	}

	return ch;
}

void JsonTokenizer::checkNotEof() {
	if(this->stream->isEndOfFile()){
		UnicodeString msg(L"Token format error at line : ");
		msg.append((int64_t)this->line);
		msg.append(L" column : ");
		msg.append((int64_t)this->pos);

		throw new JsonTokenizeException(msg.towString(), __FILE__, __LINE__);
	}
}

const AbstractJsonToken* JsonTokenizer::foreseeToken() {
	// if foreseaed before
	int fsize = this->foreseeList->size();
	if(fsize > this->foreseeCount){
		const AbstractJsonToken* token = this->foreseeList->get(this->foreseeCount++);
		return token;
	}

	// get Token
	AbstractJsonToken* token = getToken(true);
	this->foreseeList->addElement(token);

	this->foreseeCount++;

	return token;
}


void JsonTokenizer::beginForesee() {
	this->stack->addElement(this->foreseeCount);
}

void JsonTokenizer::resetForesee() {
	int lastCount = this->stack->pop();

	this->foreseeCount = lastCount;
}


} /* namespace codablecash */
