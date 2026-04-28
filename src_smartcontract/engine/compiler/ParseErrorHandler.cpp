/*
 * ParseErrorHandler.cpp
 *
 *  Created on: 2019/01/16
 *      Author: iizuka
 */

#include "engine/compiler/ParseErrorHandler.h"
#include "engine/compiler/CompileError.h"
#include "base/UnicodeString.h"

#include "alinous_lang/ParseException.h"

namespace alinous {

ParseErrorHandler::ParseErrorHandler() : ErrorHandler() {
	this->devMode = false;
}

ParseErrorHandler::~ParseErrorHandler() {
	this->list.deleteElements();
}

/**
 * ErrorHandler::handleUnexpectedToken(expectedKind, expectedToken, actual, parser); // default
 * @param expectedKind
 * @param expectedToken
 * @param actual
 * @param parser
 */
void ParseErrorHandler::handleUnexpectedToken(int expectedKind,	const JJString& expectedToken, Token* actual, AlinousLang* parser) {
	CompileError* error = new CompileError(new UnicodeString(expectedToken.c_str()),
			new UnicodeString(actual->image.c_str()), actual->beginLine, actual->beginColumn);
	list.addElement(error);

	this->error_count++;

	ParseException* p = new ParseException();
	throw p;
}

void ParseErrorHandler::handleParseError(Token *last, Token *unexpected, const JJSimpleString &production, AlinousLang *parser) {
	this->error_count++;

	ParseException* p = new ParseException();
	throw p;
}

bool ParseErrorHandler::hasError() const noexcept {
	return !this->list.isEmpty() || this->error_count != 0;
}

} /* namespace alinous */
