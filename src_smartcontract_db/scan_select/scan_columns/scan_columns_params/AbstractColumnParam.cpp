/*
 * AbstractColumnParam.cpp
 *
 *  Created on: 2020/08/20
 *      Author: iizuka
 */

#include "scan_select/scan_columns/scan_columns_params/AbstractColumnParam.h"

#include "base/UnicodeString.h"

namespace codablecash {

const UnicodeString AbstractColumnParam::PREFIX(L"_");

AbstractColumnParam::AbstractColumnParam() {

}

AbstractColumnParam::~AbstractColumnParam() {

}

void AbstractColumnParam::buildNameList(ArrayList<UnicodeString>* list, const ScanResultMetadata* metadata) const noexcept {
	if(this->asName != nullptr){
		return AbstractScanColumnsTarget::buildNameList(list, metadata);
	}

	int index = list->size();
	UnicodeString* nstr = new UnicodeString(PREFIX);
	nstr->append((int64_t)index);

	list->addElement(nstr);
}


} /* namespace codablecash */
