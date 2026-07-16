/*
 * BigInteger.cpp
 *
 *  Created on: 2019/01/20
 *      Author: iizuka
 */


#include "numeric/BigInteger.h"
#include "numeric/exceptions.h"
#include "base_io/ByteBuffer.h"
#include "base/UnicodeString.h"

#include "base/Integer.h"

#include "base/StackRelease.h"
#include "base/ArrayList.h"

#include <stdlib.h>

#include "osenv/funcs.h"

#include "cassert"

#include "../../src_ext/mod_sqrt/bn_sqrt.h"
#include "crypto/Sha256.h"

using codablecash::Sha256;
namespace alinous {

const BigInteger BigInteger::ZERO((int64_t)0);
const BigInteger BigInteger::ONE((int64_t)1);
const BigInteger BigInteger::TWO((int64_t)2);
const BigInteger BigInteger::THOUSAND((int64_t)1000);


BigInteger& BigInteger::operator =(const BigInteger& inst) {
	if(this != &inst){
		mpz_set(this->value, inst.value);
	}
	return(*this);
}

BigInteger::BigInteger(const mpz_t mpvalue) {
	mpz_init_set(this->value, mpvalue);
}

BigInteger::BigInteger(const wchar_t* str, int radix) {
	UnicodeString ustr(str);

	const char* cstr = ustr.toCString();

	mpz_init_set_str(this->value, cstr, radix);

	delete [] cstr;
}

BigInteger::BigInteger(const BigInteger& inst) {
	mpz_init(this->value);
	mpz_set(this->value, inst.value);
}

BigInteger::BigInteger(int64_t value) {
	mpz_init(this->value);
	mpz_set_si(this->value, value);
}

BigInteger::BigInteger(const UnicodeString* val, int radix) {
	const char* str = val->toCString();

	mpz_init_set_str(this->value, str, radix);

	delete [] str;
}

BigInteger::BigInteger(const UnicodeString& val, int radix) {
	const char* str = val.toCString();

	mpz_init_set_str(this->value, str, radix);

	delete [] str;
}

BigInteger::BigInteger(const UnicodeString* val) {
	const char* str = val->toCString();

	mpz_init_set_str(this->value, str, 10);

	delete [] str;
}

BigInteger::BigInteger(const UnicodeString& val) {
	const char* str = val.toCString();

	mpz_init_set_str(this->value, str, 10);

	delete [] str;
}

BigInteger::~BigInteger() {
	mpz_clear(this->value);
}

int BigInteger::bitLength() const {
	return mpz_sizeinbase(this->value, 2);
}

int64_t BigInteger::longValue() const {
	return mpz_get_si(this->value);
}

BigInteger BigInteger::multiply(const BigInteger& val) const {
	mpz_t op;
	mpz_init(op);

	mpz_mul(op, this->value, val.value);

	BigInteger ret(op);
	mpz_clear(op);

	return *(&ret);
}

BigInteger& BigInteger::multiplySelf(const BigInteger& val) {
	mpz_mul(this->value, this->value, val.value);
	return *this;
}

BigInteger BigInteger::subtract(const BigInteger& val) const {
	mpz_t op;
	mpz_init(op);

	mpz_sub(op, this->value, val.value);

	BigInteger ret(op);
	mpz_clear(op);

	return *(&ret);
}

BigInteger& BigInteger::subtractSelf(const BigInteger& val) {
	mpz_sub(this->value, this->value, val.value);
	return *this;
}

BigInteger BigInteger::add(const BigInteger& val) const {
	mpz_t op;
	mpz_init(op);

	mpz_add(op, this->value, val.value);

	BigInteger ret(op);
	mpz_clear(op);

	return *(&ret);
}

BigInteger BigInteger::addSelf(const BigInteger &val) {
	mpz_add(this->value, this->value, val.value);
	return *this;
}

BigInteger BigInteger::divide(const BigInteger& divisor) const {
	mpz_t op;
	mpz_init(op);

	mpz_tdiv_q(op, this->value, divisor.value);

	BigInteger ret(op);
	mpz_clear(op);

	return *(&ret);
}

BigInteger BigInteger::pow(uint64_t exp) const {
	mpz_t op;
	mpz_init(op);

	mpz_pow_ui(op, this->value, exp);

	BigInteger *ret = new BigInteger(op); __STP(ret);
	mpz_clear(op);

	return *ret;
}

BigInteger BigInteger::modPow(const BigInteger& exponent, const BigInteger& m) const {
	mpz_t op;
	mpz_init(op);

	mpz_powm(op, this->value, exponent.value, m.value);

	BigInteger *ret = new BigInteger(op); __STP(ret);
	mpz_clear(op);

	return *ret;
}

BigInteger BigInteger::mod(const BigInteger& m) const {
	mpz_t op;
	mpz_init(op);

	mpz_mod(op, this->value, m.value);

	BigInteger *ret = new BigInteger(op); __STP(ret);
	mpz_clear(op);

	return *ret;
}

BigInteger BigInteger::modui(uint64_t m) const {
	mpz_t op;
	mpz_init(op);

	mpz_mod_ui(op, this->value, m);

	BigInteger *ret = new BigInteger(op); __STP(ret);
	mpz_clear(op);

	return *ret;
}

BigInteger BigInteger::modInverse(const BigInteger& val) {
	mpz_t op;
	mpz_init(op);

	mpz_invert(op, this->value, val.value);

	BigInteger *ret = new BigInteger(op); __STP(ret);
	mpz_clear(op);

	return *ret;
}

BigInteger& BigInteger::modSelf(const BigInteger& m) {
	mpz_mod(this->value, this->value, m.value);
	return *this;
}

BigInteger BigInteger::shiftLeft(int n) const {
    if (n == 0) {
        return *this;
    }
    if(n < 0){
    	return shiftRight(-n);
    }

	mpz_t op;
	mpz_init(op);
	mpz_mul_2exp(op, this->value, n);

	BigInteger ret(op);
	mpz_clear(op);

	return ret;
}

BigInteger BigInteger::shiftRight(int n) const {
    if (n == 0) {
        return *this;
    }
    if(n < 0){
    	return shiftLeft(-n);
    }

	mpz_t op;
	mpz_init(op);
	//mpz_tdiv_q_2exp(op, this->value, n);
	mpz_fdiv_q_2exp(op, this->value, n);

	BigInteger ret(op);
	mpz_clear(op);

	return ret;
}

BigInteger BigInteger::_and(const BigInteger &val) const noexcept {
	mpz_t op;
	mpz_init(op);

	//(mpz_t rop, const mpz_t op1, const mpz_t op2)
	mpz_and(op, this->value, val.value);

	BigInteger ret(op);
	mpz_clear(op);

	return ret;
}

bool BigInteger::equals(const BigInteger* x1) const {
	int cmp = mpz_cmp(this->value, x1->value);
	return cmp == 0;
}

int BigInteger::compareTo(const BigInteger& x) const {
	int cmp = mpz_cmp(this->value, x.value);
	return cmp;
}

bool BigInteger::testBit(int n) const {
	mp_bitcnt_t bit_index = n;

	int bit = mpz_tstbit(this->value, bit_index);
	return bit > 0;
}

UnicodeString BigInteger::toString(int radix) const {
	char *buff = mpz_get_str(nullptr, radix, this->value);

	UnicodeString str(buff);

	::free(buff);

	return str;
}

UnicodeString BigInteger::toString(const char* format) const {
	char buff[1024]{};

	gmp_snprintf(buff, sizeof(buff), format, this->value);

	UnicodeString str(buff);
	return str;
}

UnicodeString BigInteger::toString() const {
	return toString(10);
}

BigInteger BigInteger::abs() const {
	mpz_t op;
	mpz_init(op);

	mpz_abs(op, this->value);

	BigInteger* ret = new BigInteger(op); __STP(ret);
	mpz_clear(op);

	return *ret;
}

BigInteger BigInteger::negate() const {
	mpz_t op;
	mpz_init(op);

	mpz_neg(op, this->value);

	BigInteger* ret = new BigInteger(op); __STP(ret);
	mpz_clear(op);

	return *ret;
}

BigInteger BigInteger::valueOf(int64_t val) {
	return BigInteger(val);
}

bool BigInteger::isNegative() const {
	BigInteger ZERO((int64_t)0);

	int cmp = compareTo(ZERO);

	return cmp < 0;
}

int BigInteger::binarySize() const {
	size_t count;
	uint8_t* data =  (uint8_t*)mpz_export(NULL, &count, 1, 1, 1, 0, this->value);
	::free(data);

	return count;
}

ByteBuffer* BigInteger::toBinary() const {
	/**
	 * order = 1 :1 for most significant word first
	 * endian = 1 : 1 for most significant byte first(Big endian)
	 */

	size_t count;
	uint8_t* data =  (uint8_t*)mpz_export(NULL, &count, 1, 1, 1, 0, this->value);

	ByteBuffer* buff = ByteBuffer::wrapWithEndian(data, count, true);
	::free(data);

	return buff;
}

BigInteger* BigInteger::fromBinary(const char* buff, int length) {
	mpz_t mpvalue;
	mpz_init(mpvalue);
	mpz_import(mpvalue, length, 1, 1, 1, 0, buff); // big endian

	BigInteger* big  = new BigInteger(mpvalue);

	mpz_clear(mpvalue);

	return big;
}

BigInteger BigInteger::random() noexcept {
	static uint64_t solt = 1;

	mpz_t s;
	mpz_init(s);

	gmp_randstate_t state;
	gmp_randinit_default(state);


	{
		mpz_t seed;
		mpz_init(seed);

		for(int i = 0; i != 10; ++i){
			uint64_t cspr = Os::getOsCspring();

			mpz_mul_2exp(seed, seed, 32);
			mpz_add_ui(seed, seed, cspr);
		}

		// hash
		{
			size_t count;
			uint8_t* data =  (uint8_t*)mpz_export(NULL, &count, 1, 1, 1, 0, seed);

			ByteBuffer* buff = ByteBuffer::wrapWithEndian(data, count, true); __STP(buff);
			::free(data);

			ByteBuffer* shabuff = Sha256::sha256(buff, true); __STP(shabuff);

			mpz_import(seed, shabuff->capacity(), 1, 1, 1, 0, shabuff->array()); // big endian
		}


		gmp_randseed(state, seed);

		mpz_clear(seed);
	}

	mpz_urandomb(s, state, 256);

	BigInteger* big = new BigInteger(s); __STP(big);

	gmp_randclear(state);
	mpz_clear(s);

	return *big;
}

BigInteger BigInteger::random(const BigInteger &min, const BigInteger &max) noexcept {
	BigInteger val(0L);

	do{
		val = random();
		val = val.mod(max);
	}
	while(!between(val, min, max));

	assert(!val.isNegative());

//	debugval = val.longValue();

	return val;
}

bool BigInteger::between(const BigInteger &val, const BigInteger &min, const BigInteger &max) {
	return val.compareTo(min) > 0 && val.compareTo(max) < 0;
}

ByteBuffer* BigInteger::padBuffer(ByteBuffer* bin, int size) {
	bin->position(0);
	int len = bin->limit();

	int pad = size - len;
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(size, true);

	assert(size >= len);

	for(int i = 0; i != pad; ++i){
		buff->put((char)0);
	}

	buff->put(bin);

	return buff;
}



bool BigInteger::sqrtm(const BigInteger &p, BigInteger *result) {
	return sqrtm(*this, p, result);
}

bool BigInteger::sqrtm(const BigInteger &y, const BigInteger &p, BigInteger *result) {
	//    n %= p
	BigInteger n = y.mod(p);
	if(!isSqrt(y, p)){
		return false;
	}

	int retcode = BN_mod_sqrt(result->value, y.value, p.value);

	return (retcode == 0);
}

// if pow(n, (p-1)>>1, p) != 1:
//     return -1
bool BigInteger::isSqrt(const BigInteger &y, const BigInteger &p) {
	BigInteger p_1 = p.subtract(ONE).shiftRight(1);
	BigInteger res = y.modPow(p_1, p);

	return res.compareTo(ONE) == 0;
}

} /* namespace alinous */
