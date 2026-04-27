/*
 * VmStackManager.cpp
 *
 *  Created on: 2019/06/30
 *      Author: iizuka
 */

#include "vm/stack/VmStackManager.h"
#include "vm/stack/VmStack.h"
namespace alinous {

VmStackManager::VmStackManager() {
	this->currentMethod = nullptr;
	this->entryPoint = nullptr;
}

VmStackManager::~VmStackManager() {
	this->list.deleteElements();
}

void VmStackManager::addStack(VmStack* stack) noexcept {
	if(this->currentMethod != nullptr){
		stack->setCurrentMethod(this->currentMethod);
		this->currentMethod = nullptr;
	}

	if(this->entryPoint != nullptr){
		stack->setEntryPoint(this->entryPoint);
		this->entryPoint = nullptr;
	}

	this->list.addElement(stack);
}

void VmStackManager::popStack() noexcept {
	int pos = this->list.size() - 1;

	this->list.remove(pos);
}

VmStack* VmStackManager::top() const noexcept {
	int pos = this->list.size() - 1;
	return this->list.get(pos);
}

int VmStackManager::size() const noexcept {
	return this->list.size();
}

bool VmStackManager::isEmpty() const noexcept {
	return this->list.isEmpty();
}

VmStack* VmStackManager::get(int pos) const noexcept {
	return this->list.get(pos);
}

void VmStackManager::markStackbyMethod(MethodDeclare *method) {
	this->currentMethod = method;
}

void VmStackManager::markStackEntryPoint(AbstractExpression *exp) {
	this->entryPoint = exp;
}

} /* namespace alinous */
