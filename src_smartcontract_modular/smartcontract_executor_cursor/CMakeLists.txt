

set(__src
	SessionContextCursor.cpp
	SessionContextCursorData.cpp
	SmartcontractCallSequence.cpp
	SmartcontractCallSequenceBuilder.cpp
	SmartcontractCallSequenceCursor.cpp
	SmartcontractContextMover.cpp
)
handle_sub(codablecashlib "${__src}" smartcontract_modular smartcontract_executor_cursor)

