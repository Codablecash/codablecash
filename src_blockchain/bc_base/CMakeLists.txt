

set(__src
	Abstract32BytesId.cpp
	AbstractAddress.cpp
	AddressCheckDigitException.cpp
	AddressDescriptor.cpp
	BalanceAddress.cpp
	BalanceUnit.cpp
	Base58.cpp
	Base64.cpp
	BinaryUtils.cpp
)
handle_sub(codablecashlib "${__src}" blockchain bc_base)
