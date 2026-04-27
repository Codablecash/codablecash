/*
 * MultipleCommandProcessor.cpp
 *
 *  Created on: 2023/08/13
 *      Author: iizuka
 */

#include "procesor_multi/MultipleCommandProcessor.h"
#include "procesor_multi/CommandProcessUnit.h"

#include "procesor/ICommandParameter.h"

#include "bc/ISystemLogger.h"

#include "osenv/funcs.h"

#include "base/UnicodeString.h"
namespace codablecash {

MultipleCommandProcessor::MultipleCommandProcessor(ICommandParameter* param, int size, ISystemLogger* logger, UnicodeString* name) {
	this->list = new ArrayList<CommandProcessUnit>();
	this->logger = logger;
	this->param = param;

	UnicodeString prefix(name);

	for(int i = 0; i != size; ++i){
		UnicodeString name(prefix);
		name.append((int64_t)i);

		CommandProcessUnit* unit = new CommandProcessUnit(this, this->logger, &name);
		this->list->addElement(unit);
	}
}

MultipleCommandProcessor::~MultipleCommandProcessor() {
	shutdown();

	delete this->list;

	this->logger = nullptr;
	this->param = nullptr;
}

void MultipleCommandProcessor::start() {
	int maxLoop = size();
	for(int i = 0; i != maxLoop; ++i){
		CommandProcessUnit* unit = this->list->get(i);

		unit->setRunning(true);
		unit->start();
	}

	for(int i = 0; i != maxLoop; ++i){
		CommandProcessUnit* unit = this->list->get(i);

		while(unit->getStatus() != CommandProcessUnit::STATUS_WAITING) Os::usleep(1000);
	}
}

void MultipleCommandProcessor::shutdown() noexcept {
	int maxLoop = size();
	for(int i = 0; i != maxLoop; ++i){
		CommandProcessUnit* unit = this->list->get(i);

		if(unit->isRunning()){
			unit->setRunning(false);
		}
	}

	for(int i = 0; i != maxLoop; ++i){
		CommandProcessUnit* unit = this->list->get(i);

		unit->join();
	}

	this->list->deleteElements();
	this->list->reset();
}

void MultipleCommandProcessor::addCommandMessage(ICommandMessage *cmd, int i) noexcept {
	int pos = i % size();
	CommandProcessUnit* unit = this->list->get(pos);
	unit->addCommandMessage(cmd);
}

void MultipleCommandProcessor::addCommandMessageFirst(ICommandMessage *cmd, int i) noexcept {
	int pos = i % size();
	CommandProcessUnit* unit = this->list->get(pos);
	unit->insertCommandMessage(cmd, 0);
}

} /* namespace codablecash */
