

set(__src
	RemoteBalanceUtxo.cpp
	RemoteUtxoReference.cpp
)
handle_sub(codablecashlib "${__src}" blockchain bc_trx_icc)
