cmake_minimum_required(VERSION 3.10)

project(LK8000_POCO)

include_directories (
	../../../Header
	../../../Source/xcs
)

add_library (
	${PROJECT_NAME}

	Debugger.cpp
	Bugcheck.cpp
	ErrorHandler.cpp
	Event.cpp
	NamedEvent.cpp
	Exception.cpp
	Mutex.cpp
	NamedMutex.cpp
	Runnable.cpp
	RWLock.cpp
	Thread.cpp
	ThreadLocal.cpp
	ThreadTarget.cpp
	Timestamp.cpp
	Timespan.cpp
	UnicodeConverter.cpp
	UTF8Encoding.cpp
	UTF16Encoding.cpp
	TextEncoding.cpp
	ASCIIEncoding.cpp
	Latin1Encoding.cpp
	Latin9Encoding.cpp
	Windows1252Encoding.cpp
	TextIterator.cpp
	TextConverter.cpp
	Ascii.cpp
	AtomicCounter.cpp
	RefCountedObject.cpp
	Condition.cpp
)
