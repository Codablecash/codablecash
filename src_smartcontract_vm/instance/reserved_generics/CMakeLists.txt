

set(__src
	AbstractGenericReservedMethodDeclare.cpp
	AbstractReservedGenericsClassDeclare.cpp
	ReservedGeneratedGenericsClassDeclare.cpp
)
handle_sub(codablecashlib "${__src}" smartcontract_vm instance/reserved_generics)

