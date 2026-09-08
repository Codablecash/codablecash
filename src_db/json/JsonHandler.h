/*
 * JsonHandler.h
 *
 *  Created on: Sep 25, 2025
 *      Author: iizuka
 */

#ifndef JSON_JSONHANDLER_H_
#define JSON_JSONHANDLER_H_

namespace alinous {
class File;
class UnicodeString;
}
using namespace alinous;

namespace codablecash {

class JsonTokenizer;
class AbstractJsonObject;


class JsonHandler {
public:
	JsonHandler();
	virtual ~JsonHandler();

	void loadFile(const File* file, int buffSize);
	void loadString(const UnicodeString* str);

	AbstractJsonObject* parse();

	JsonTokenizer* getTokenizer() const noexcept {
		return this->tokenizer;
	}

private:
	JsonTokenizer* tokenizer;
};

} /* namespace codablecash */

#endif /* JSON_JSONHANDLER_H_ */
