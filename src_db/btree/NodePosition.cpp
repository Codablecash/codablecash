/*
 * NodePosition.cpp
 *
 *  Created on: 2018/12/29
 *      Author: iizuka
 */

#include "btree/NodePosition.h"
#include "btree/NodeHandle.h"
#include "btree/AbstractTreeNode.h"
#include "btree/AbstractBtreeKey.h"
#include "btree/BtreeStorage.h"
#include "btree/TreeNode.h"
#include "btree/DataNode.h"
#include "btree/exceptions.h"
#include "btree/AbstractBtreeDataFactory.h"

#include "btree_cache/NodeCacheRef.h"

#include "base/StackRelease.h"


namespace alinous {

NodePosition::NodePosition(NodeHandle* nodeHandle) {
	this->node = nodeHandle;
	this->pos = 0;
	this->innerNodes = nullptr;
	this->innerCount = 0;
}

NodePosition::~NodePosition() {
	delete this->node;

	if(this->innerNodes != nullptr){
		clearCache();
	}
}

void NodePosition::clearCache() {
	if(this->innerNodes != nullptr){
		this->innerNodes->deleteElements();
		delete this->innerNodes, this->innerNodes = nullptr;
	}
}

bool NodePosition::isEmpty() const noexcept {
	return this->innerCount == 0;
}

const AbstractBtreeKey* NodePosition::getKey() const noexcept {
	return this->node->getKey();
}

void NodePosition::setKey(const AbstractBtreeKey *key) noexcept {
	this->node->setKey(key);
}

uint64_t NodePosition::getFpos() const noexcept {
	return this->node->getFpos();
}

bool NodePosition::isLeaf() const {
	return this->node->isLeaf();
}

bool NodePosition::isRoot() const {
	return this->node->isRoot();
}

void NodePosition::setRoot(bool isroot) {
	this->node->setIsRoot(isroot);
}
/*
bool NodePosition::isData() const {
	return this->node->isData();
}
*/
NodeHandle* NodePosition::hasKey(const AbstractBtreeKey* key) const {
	int maxLoop = this->innerNodes->size();
	for(int i = 0; i != maxLoop; ++i){
		NodeHandle* nodeHandle = this->innerNodes->get(i);
		if(nodeHandle == nullptr){
			return nullptr;
		}
		AbstractBtreeKey* inkey = nodeHandle->getRef()->getNode()->getKey();
		if(key->compareTo(inkey) == 0){
			return nodeHandle;
		}
	}

	return nullptr;
}

void NodePosition::loadInnerNodes(BtreeStorage* store) {
	clearCache();

	RawArrayPrimitive<uint64_t>* fposList = this->node->getInnerNodeFpos();
	this->innerNodes = new ArrayList<NodeHandle>(fposList->size());

	this->innerCount = 0;

	int maxLoop = fposList->size();
	for(int i = 0; i != maxLoop; ++i){
		uint64_t fpos = fposList->get(i);
		if(fpos == 0){
			this->innerNodes->addElement(nullptr);
			continue;
		}

		NodeHandle* nodeHandle = store->loadNode(fpos);
		checkNoNull(nodeHandle, __FILE__, __LINE__);
		assert(nodeHandle->getFpos() != 0);

		this->innerNodes->addElement(nodeHandle);
		this->innerCount++;
	}
}

void NodePosition::setLastPos(){
	this->pos = this->innerCount - 1;
}

bool NodePosition::isFull(int nodeNumber) const noexcept {
	return this->innerCount >= nodeNumber;
}

void NodePosition::addNode(const AbstractBtreeKey* key, uint64_t fpos, int nodeNumber) {
	for(int i = 0; i != nodeNumber; ++i){
		NodeHandle* nh = this->innerNodes->get(i);

		if(nh == nullptr || nh->getKey()->compareTo(key) > 0){
			internalAddNode(i, fpos);
			break;
		}
	}
	clearCache();
}

uint64_t NodePosition::getNextChildPrevious(const AbstractBtreeKey* key) {
	uint64_t ret = 0;
	int maxLoop = this->innerCount;
	for(int i = maxLoop - 1; i >= 0; --i){
		NodeHandle* nh = this->innerNodes->get(i);

		if(key->compareTo(nh->getKey()) == 0){
			ret = nh->getFpos();
			this->pos = i - 1;
			break;
		}
		else if(key->compareTo(nh->getKey()) > 0){
			nh = this->innerNodes->get(i + 1);
			ret = nh->getFpos();
			this->pos = i;
			break;
		}
	}

	if(ret == 0){
		NodeHandle* nh = this->innerNodes->get(0);
		ret = nh->getFpos();
		this->pos = -1;
	}

	return ret;
}

uint64_t NodePosition::getNextChild(const AbstractBtreeKey* key) {
#ifdef __DEBUG__
	int count = 0;
#endif

	uint64_t ret = 0;
	int maxLoop = this->innerCount;
	for(int i = 0; i != maxLoop; ++i){
		NodeHandle* nh = this->innerNodes->get(i);
		this->pos++;

		AbstractBtreeKey* hskey = nh->getKey();
		if(key->compareTo(hskey) <= 0){
			ret = nh->getFpos();

#ifdef __DEBUG__
			count++;
#endif
			break;
		}
	}

#ifdef __DEBUG__
	assert(count > 0);
#endif

	return ret;
}

ArrayList<NodeHandle>* NodePosition::getInnerNodes() const noexcept {
	return this->innerNodes;
}

void NodePosition::internalAddNode(int index, uint64_t fpos) {
	TreeNode* treeNode = this->node->toTreeNode();
	RawArrayPrimitive<uint64_t>* list = treeNode->getInnerNodeFpos();

	int first = list->size() - 1;
	for(int i = first; i > index; --i){
		uint64_t f = list->get(i - 1);
		list->set(i ,f);
	}
	list->set(index ,fpos);

	this->innerCount++;
}

void NodePosition::updateInnerNodeFpos(const RawArrayPrimitive<uint64_t>* newlist) {
	TreeNode* treeNode = this->node->toTreeNode();
	treeNode->updateInnerNodeFpos(newlist);

	clearCache();
}

void NodePosition::save(BtreeStorage* store) {
	AbstractTreeNode* node = this->node->getRef()->getNode();

	store->updateNode(node);
}

void NodePosition::checkNoNull(NodeHandle* nodeHandle, const char* srcfile, int srcline) noexcept(false) {
	if(nodeHandle == nullptr){
		throw new NodeStructureException(srcfile, srcline);
	}
}

NodeHandle* NodePosition::getNodeHandle() const noexcept {
	return this->node;
}

uint64_t NodePosition::getChildFpos(int i) const noexcept {
	return this->innerNodes->get(i)->getFpos();
}

uint64_t NodePosition::previousData() {
	DataNode* dnode = this->node->toDataNode();

	//if(0 < this->pos){
	//	return 0;
	//}
	assert(this->pos == 0);

	this->pos--;
	return dnode->getDataFpos();
}

uint64_t NodePosition::previousNode(){
	TreeNode* treeNode = this->node->toTreeNode();
	RawArrayPrimitive<uint64_t>* list = treeNode->getInnerNodeFpos();

	if(this->pos < 0){
		return 0;
	}

	int cur = this->pos--;
	return list->get(cur);
}

bool NodePosition::hasPrevious() {
	return this->pos >= 0;
}

uint64_t NodePosition::nextData() {
	DataNode* dnode = this->node->toDataNode();

	//if(0 < this->pos){
	//	return 0;
	//}
	assert(this->pos == 0);

	this->pos++;
	return dnode->getDataFpos();
}

uint64_t NodePosition::nextNode() {
	TreeNode* treeNode = this->node->toTreeNode();
	RawArrayPrimitive<uint64_t>* list = treeNode->getInnerNodeFpos();

	if(this->innerCount - 1 < this->pos){
		return 0;
	}

	int cur = this->pos++;
	return list->get(cur);
}

bool NodePosition::hasNext() {
	return this->innerCount > this->pos;
}

bool NodePosition::removeChildNode(const AbstractBtreeKey* key, BtreeStorage* store) {
	int removePos = indexof(key);
	if(removePos < 0){
		return false;
	}

	if(isLeaf()){
		internalRemoveLeafChildNode(removePos, store, key);
	}
	else{
		internalRemoveChildNode(removePos, store);
	}

	return true;
}

void NodePosition::removeInnerNodeFpos(int index) noexcept {
	RawArrayPrimitive<uint64_t>* dnodesList = this->node->getInnerNodeFpos();
	int maxLoop = this->innerCount - 1;
	int i = index;
	for(; i != maxLoop; i++){
		uint64_t nextfpos = dnodesList->get(i + 1);
		dnodesList->set(i, nextfpos);
	}

	dnodesList->set(i, 0);
}

/**
 * remove node (not data node)
 */
void NodePosition::internalRemoveChildNode(int index, BtreeStorage* store) {
	NodeHandle* nh = this->innerNodes->get(index);
	uint64_t fpos = nh->getFpos();

	removeInnerNodeFpos(index); // remove fpos
	delete this->innerNodes->remove(index); // remove cached
	this->innerCount--;
	save(store);

	store->remove(fpos);
}

/**
 * remove data node
 */
void NodePosition::internalRemoveLeafChildNode(int index, BtreeStorage* store, const AbstractBtreeKey* key) {
	const AbstractBtreeDataFactory* dfactory = store->getDataFactory();

	// delete cache use count
	NodeHandle* nh = this->innerNodes->get(index);
	DataNode* dnode = nh->toDataNode();

	bool remove = dfactory->beforeRemove(dnode, store, key);
	if(!remove){
		return;
	}

	uint64_t dataFpos = dnode->getDataFpos();

	// remove data
	this->innerNodes->remove(index);
	delete nh;

	store->removeData(dataFpos);

	// remove child data node
	uint64_t nodeFpos = dnode->getFpos();
	store->remove(nodeFpos);

	// update self
	removeInnerNodeFpos(index);
	this->innerCount--;

	store->updateNode(this->node->getRef()->getNode());
}

int NodePosition::indexof(const AbstractBtreeKey* key) const {
	int maxLoop = this->innerCount;
	for(int i = 0; i != maxLoop; ++i){
		NodeHandle* nh = this->innerNodes->get(i);

		if(key->compareTo(nh->getKey()) == 0){
			return i;
		}
	}

	return -1;
}

const NodeHandle* NodePosition::gotoEqLessThanKey(const AbstractBtreeKey* key) {
	int maxLoop = this->innerCount;

	for(int i = maxLoop - 1; i >= 0; --i){
		NodeHandle* nh = this->innerNodes->get(i);
		if(key->compareTo(nh->getKey()) >= 0){
			this->pos = i - 1;
			return nh;
		}
	}

	return nullptr;
}


const NodeHandle* NodePosition::gotoEqMoreThanKey(const AbstractBtreeKey* key) {
	int maxLoop = this->innerCount;

	for(int i = 0; i != maxLoop; ++i){
		this->pos++;
		NodeHandle* nh = this->innerNodes->get(i);
		if(key->compareTo(nh->getKey()) <= 0){
			return nh;
		}
	}

	return nullptr;
}

const NodeHandle* NodePosition::gotoEqKey(const AbstractBtreeKey* key) {
	int maxLoop = this->innerCount;

	for(int i = 0; i != maxLoop; ++i){
		this->pos++;
		NodeHandle* nh = this->innerNodes->get(i);
		if(key->compareTo(nh->getKey()) == 0){
			return nh;
		}
	}

	return nullptr;
}

#ifdef __DEBUG__
void NodePosition::assertTopRightNode() {
	if(this->innerCount > 0 && !isLeaf()){
		int maxLoop = this->innerCount - 1;
		NodeHandle* nh = this->innerNodes->get(maxLoop);

		AbstractBtreeKey* key = nh->getKey();
		assert(key->compareTo(this->node->getKey()) == 0);
	}
}
#endif

} /* namespace alinous */


