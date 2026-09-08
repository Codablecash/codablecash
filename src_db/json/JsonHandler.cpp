/*
 * JsonHandler.cpp
 *
 *  Created on: Sep 25, 2025
 *      Author: iizuka
 */

#include "json/JsonHandler.h"
#include "json/JsonTokenizer.h"
#include "json/JsonStringStream.h"
#include "json/AbstractJsonToken.h"

#include "base_io_stream/FileInputStream.h"
#include "base_io_stream/InputStreamReader.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"

#include "json/JsonParser.h"


namespace codablecash {

JsonHandler::JsonHandler() {
	this->tokenizer = nullptr;
}

JsonHandler::~JsonHandler() {
	delete this->tokenizer;
}

void JsonHandler::loadFile(const File* file, int buffSize) {
	UnicodeString strJson(L"");

	FileInputStream stream(file);
	stream.open();

	InputStreamReader reader(&stream);
	wchar_t* buff = new wchar_t[buffSize]; StackArrayRelease<wchar_t> __st_buff(buff);

	int red = reader.read(buff, buffSize);
	while(red > 0){
		strJson.append(buff, red);

		red = reader.read(buff, buffSize);
	}

	// create tokenizer
	JsonStringStream* jsonStringStream = new JsonStringStream(&strJson);
	this->tokenizer = new JsonTokenizer(jsonStringStream);
}

void JsonHandler::loadString(const UnicodeString *str) {
	// create tokenizer
	JsonStringStream* jsonStringStream = new JsonStringStream(str);
	this->tokenizer = new JsonTokenizer(jsonStringStream);
}

AbstractJsonObject* JsonHandler::parse() {
	JsonParser parser(this->tokenizer);

	AbstractJsonObject* object = parser.parse();
	return object;
}


} /* namespace codablecash */
