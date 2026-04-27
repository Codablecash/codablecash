/*
 * ISystemLogger.h
 *
 *  Created on: 2023/01/13
 *      Author: iizuka
 */

#ifndef BC_ISYSTEMLOGGER_H_
#define BC_ISYSTEMLOGGER_H_

namespace alinous {
class Exception;
class UnicodeString;
}
using namespace alinous;

namespace codablecash {

class ISystemLogger {
public:
	static const constexpr int DEBUG_CHAIN_HEAD_DETECT{1};
	static const constexpr int DEBUG_NODE_TRANSFER_RESPONSE{2};

	static const constexpr int DEBUG_POW_CALC_THREAD{3};
	static const constexpr int DEBUG_UTXO_ID{4};


	static const constexpr int DEBUG_TMP_INFO{100};

	static const constexpr int DEBUG_DBSESSION_INFO{101};
	static const constexpr int DEBUG_POOL_TRANSFER_RESPONSE{102};

	ISystemLogger();
	virtual ~ISystemLogger();

	virtual void logException(const Exception* e) noexcept = 0;
	virtual void log(const UnicodeString* message) noexcept = 0;

	virtual void debugLog(int section, const UnicodeString* message, const char* srcfile, int srcline) noexcept = 0;

};

} /* namespace codablecash */

#endif /* BC_ISYSTEMLOGGER_H_ */
