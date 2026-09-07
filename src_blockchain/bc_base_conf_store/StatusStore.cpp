/*
 * StatusStore.cpp
 *
 *  Created on: 2022/05/14
 *      Author: iizuka
 */

#include "bc_base_conf_store/StatusStore.h"

#include "bc_base_conf_store/AbstractConfigStoreElement.h"
#include "bc_base_conf_store/LongValueConfigStoreValue.h"
#include "bc_base_conf_store/ShortValueConfigStoreValue.h"
#include "bc_base_conf_store/BinaryValueConfigStoreValue.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "base_io/File.h"
#include "base_io/ByteBuffer.h"

#include "base_thread/ConcurrentGate.h"

#include "random_access_file/DiskCacheManager.h"
#include "random_access_file/RandomAccessFile.h"


namespace codablecash {

StatusStore::StatusStore(const File* baseDir, const UnicodeString* name) {
	this->baseDir = new File(*baseDir);
	this->file = this->baseDir->get(name);
	this->store = nullptr;

	this->diskCacheManager = new DiskCacheManager();
	this->rwLock = new ConcurrentGate();
}

StatusStore::StatusStore(const File* baseDir, const wchar_t* name) {
	this->baseDir = new File(*baseDir);
	this->file = this->baseDir->get(name);
	this->store = nullptr;

	this->diskCacheManager = new DiskCacheManager();
	this->rwLock = new ConcurrentGate();
}


StatusStore::~StatusStore() {
	Iterator<UnicodeString>* it = this->map.keySet()->iterator(); __STP(it);
	while(it->hasNext()){
		const UnicodeString* key = it->next();
		AbstractConfigStoreElement* value = this->map.get(key);

		delete value;
	}

	delete this->baseDir;
	delete this->file;

	delete this->store;
	delete this->diskCacheManager;

	delete this->rwLock;
}

bool StatusStore::exists() const {
	return this->file->exists();
}

void StatusStore::open() {
	if(!this->baseDir->exists()){
		this->baseDir->mkdirs();
	}

	delete this->store;
	this->store = new RandomAccessFile(this->file, this->diskCacheManager);
	this->store->open(false);
}

void StatusStore::close() noexcept {
	if(this->store != nullptr){
		this->store->close();

		delete this->store;
		this->store = nullptr;
	}
}

void StatusStore::addLongValue(const wchar_t* key, int64_t value) {
	UnicodeString keystr(key);
	LongValueConfigStoreValue* v = new LongValueConfigStoreValue(value);

	addValue(&keystr, v);
}

void StatusStore::addShortValue(const UnicodeString *key, int16_t shortInt) {
	ShortValueConfigStoreValue* v = new ShortValueConfigStoreValue(shortInt);

	addValue(key, v);
}

void StatusStore::addShortValue(const wchar_t *key, int16_t shortInt) {
	UnicodeString str(key);
	addShortValue(&str, shortInt);
}


void StatusStore::addBinaryValue(const wchar_t *key, const uint8_t *data, int length) {
	BinaryValueConfigStoreValue* value = new BinaryValueConfigStoreValue();
	value->init(data, length);

	UnicodeString str(key);
	addValue(&str, value);
}

void StatusStore::addValue(const UnicodeString* key, AbstractConfigStoreElement* value) {
	StackWriteLock __lock(this->rwLock, __FILE__, __LINE__);

	__nlk_addValue(key, value);
}

void StatusStore::__nlk_addValue(const UnicodeString* key, AbstractConfigStoreElement* value) {
	AbstractConfigStoreElement* lastv = this->map.get(key);
	if(lastv != nullptr){
		delete lastv;
	}

	this->map.put(key, value);

	open();
	write();
	close();
}

int16_t StatusStore::getShortValue(const UnicodeString *key) const noexcept {
	return getShortValue(key->towString());
}

int16_t StatusStore::getShortValue(const wchar_t *key) const noexcept {
	const AbstractConfigStoreElement* v = getValue(key);
	const ShortValueConfigStoreValue* value = dynamic_cast<const ShortValueConfigStoreValue*>(v);

	return value != nullptr ? value->getValue() : 0;
}

ByteBuffer* StatusStore::getBinaryValue(const wchar_t *key) const noexcept {
	const AbstractConfigStoreElement* v = getValue(key);
	const BinaryValueConfigStoreValue* value = dynamic_cast<const BinaryValueConfigStoreValue*>(v);

	return value != nullptr ? ByteBuffer::wrapWithEndian(value->getData(), value->getLength(), true) : nullptr;
}

int64_t StatusStore::getLongValue(const wchar_t *key) const noexcept {
	const AbstractConfigStoreElement* v = getValue(key);
	const LongValueConfigStoreValue* value = dynamic_cast<const LongValueConfigStoreValue*>(v);

	return value != nullptr ? value->getValue() : 0;
}

const AbstractConfigStoreElement* StatusStore::getValue(const wchar_t* key) const noexcept {
	UnicodeString strkey(key);

	return getValue(&strkey);
}

const AbstractConfigStoreElement* StatusStore::getValue(const UnicodeString* key) const noexcept {
	StackReadLock __lock(this->rwLock, __FILE__, __LINE__);

	return this->map.get(key);
}

void StatusStore::load() {
	StackWriteLock __lock(this->rwLock, __FILE__, __LINE__);

	open();

	char totalSize[4];
	this->store->read(0, totalSize, 4);

	ByteBuffer* sb = ByteBuffer::wrapWithEndian((const uint8_t*)totalSize, 4, true); __STP(sb);
	int total = sb->getInt(0);

	char* bin = new char[total];
	StackArrayRelease<char> __st_bin(bin);

	this->store->read(sizeof(uint32_t), bin, total);
	ByteBuffer* buff = ByteBuffer::wrapWithEndian((const uint8_t*)bin, total, true); __STP(buff);

	int maxLoop = buff->getInt();
	for(int i = 0; i != maxLoop; ++i){
		UnicodeString* key = getString(buff); __STP(key);
		AbstractConfigStoreElement* value = AbstractConfigStoreElement::createFromBinary(buff);

		__nlk_addValue(key, value);
	}

	close();
}

void StatusStore::write() {
	int size = binarySize();

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(size, true); __STP(buff);

	int mapSize = this->map.size();
	buff->putInt(mapSize);

	Iterator<UnicodeString>* it = this->map.keySet()->iterator(); __STP(it);
	while(it->hasNext()){
		const UnicodeString* key = it->next();
		AbstractConfigStoreElement* value = this->map.get(key);

		putString(buff, key);
		value->toBinary(buff);
	}

	this->store->setLength(size + sizeof(uint32_t));

	ByteBuffer* sb = ByteBuffer::allocateWithEndian(sizeof(uint32_t), true); __STP(sb);
	sb->putInt(size);
	this->store->write(0, (const char*)sb->array(), sb->limit());

	this->store->write(sizeof(uint32_t), (const char*)buff->array(), buff->limit());
}

int StatusStore::binarySize() const {
	int total = 0;

	total += sizeof(uint32_t);

	Iterator<UnicodeString>* it = this->map.keySet()->iterator(); __STP(it);
	while(it->hasNext()){
		const UnicodeString* key = it->next();
		AbstractConfigStoreElement* value = this->map.get(key);

		total += stringSize(key);
		total += value->binarySize();
	}

	return total;
}

void StatusStore::putString(ByteBuffer* out, const UnicodeString* str) const noexcept {
	uint32_t maxLoop = str->length();
	out->putInt(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		wchar_t ch = str->get(i);
		out->putChar(ch);
	}
}

UnicodeString* StatusStore::getString(ByteBuffer* in) const noexcept {
	UnicodeString* ret = new UnicodeString(L"");
	uint32_t maxLoop = in->getInt();
	for(int i = 0; i != maxLoop; ++i){
		wchar_t ch = in->getChar();
		ret->append(ch);
	}

	return ret;
}

int StatusStore::stringSize(const UnicodeString* str) const noexcept {
	return sizeof(uint32_t) + str->length() * sizeof(uint16_t);
}

} /* namespace codablecash */
