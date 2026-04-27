/*
 * ModuleDetectedMethodDeclare.cpp
 *
 *  Created on: Dec 7, 2025
 *      Author: iizuka
 */

#include "modular_interfaces/ModuleDetectedMethodDeclare.h"

#include "base/UnicodeString.h"

#include "lang/sc_declare/AccessControlDeclare.h"
#include "lang/sc_declare_types/VoidType.h"
#include "lang/sc_declare/ArgumentsListDeclare.h"
#include "lang/sc_declare/ArgumentDeclare.h"

#include "lang/sc_declare_types/AbstractType.h"
#include "lang/sc_declare_types/ObjectType.h"
#include "lang/sc_declare_types/StringType.h"

#include "engine/sc_analyze/AnalyzedType.h"

#include "lang/sc_declare/PackageNameDeclare.h"


using namespace alinous;

namespace codablecash {

const UnicodeString ModuleDetectedMethodDeclare::METHOD_NAME(L"moduleDetected");

ModuleDetectedMethodDeclare::ModuleDetectedMethodDeclare() : AbstractReservedMethodDeclare(METHOD_MODULE_PROXY_LISTNER_MODULE_DETECTED)  {
	AccessControlDeclare* access = new AccessControlDeclare();
	access->setCtrl(AccessControlDeclare::PUBLIC);
	setAccessControl(access);

	setType(new VoidType());
	setName(new UnicodeString(&METHOD_NAME));

	ArgumentsListDeclare* args = new ArgumentsListDeclare();
	setArguments(args);

	{
		ArgumentDeclare* module = new ArgumentDeclare();

		ObjectType* type = new ObjectType();
		UnicodeString lang(L"lang");

		PackageNameDeclare* pkg = PackageNameDeclare::makeFromString(&lang);
		type->setPackageName(pkg);
		type->setName(new UnicodeString(L"Object"));

		module->setType(type);
		module->setName(new UnicodeString(L"module"));

		args->addArgument(module);
	}

	{
		ArgumentDeclare* moduleName = new ArgumentDeclare();

		StringType* type = new StringType();
		moduleName->setType(type);

		moduleName->setName(new UnicodeString(L"name"));

		args->addArgument(moduleName);
	}
}

ModuleDetectedMethodDeclare::~ModuleDetectedMethodDeclare() {

}

} /* namespace codablecash */
