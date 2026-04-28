/*
 * StackRelease.h
 *
 *  Created on: 2018/04/15
 *      Author: iizuka
 */

#ifndef BASE_STACKRELEASE_H_
#define BASE_STACKRELEASE_H_

#include "ArrayList.h"
#include <type_traits>

namespace alinous {

template <typename T>
class StackRelease {
public:
	StackRelease() : ptr(nullptr) {}
	explicit StackRelease(T* ptr) : ptr(ptr){
	}
	~StackRelease(){
		if(this->ptr != nullptr){
			delete ptr;
		}
	}
	void cancel() noexcept {
		this->ptr = nullptr;
	}
	void reset(T* ptr) noexcept {
		this->ptr = ptr;
	}
	T* move() noexcept {
		T* ret = this->ptr;
		cancel();

		return ret;
	}
private:
	T* ptr;
};

template <typename T>
class StackArrayRelease {
public:
	explicit StackArrayRelease(T* ptr) : ptr(ptr){
	}
	~StackArrayRelease(){
		if(this->ptr != nullptr){
			delete [] ptr;
		}
	}
private:
	T* ptr;
};

template <typename T>
class StackMultipleRelease {
public:
	StackMultipleRelease(){

	}
	~StackMultipleRelease(){
		int maxLoop = this->ptrlist.size();
		for(int i = 0; i < maxLoop; ++i){
			T* ptr = this->ptrlist.get(i);
			delete ptr;
		}
	}
	void add(T* ptr){
		ptrlist.addElement(ptr);
	}
private:
	ArrayList<T> ptrlist;
};

#define _ST(clazz, ref, getPtr) \
	clazz* ref = getPtr; \
	StackRelease<clazz> __r_##ref(ref);

#define __STP(obj) StackRelease<std::remove_pointer<decltype(obj)>::type> __st_##obj##__(obj)
#define __STP_MV(obj) __st_##obj##__.move()
#define __STP_RESET(obj, ptr) __st_##obj##__.reset(ptr)


} /* namespace alinous */

#endif /* BASE_STACKRELEASE_H_ */
