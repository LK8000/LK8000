# OS Library imported from xcsoar

XCS_OS_LINUX := \
	$(SRC)/xcs/OS/EventPipe.cpp\
	$(SRC)/xcs/OS/FileDescriptor.cpp\
	$(SRC)/xcs/OS/Poll.cpp\
	$(SRC)/xcs/OS/FileUtil.cpp\
	$(SRC)/xcs/OS/Poll.cpp\
	$(SRC)/xcs/OS/Process.cpp\
	$(SRC)/xcs/OS/PathName.cpp\
	\
	$(SRC)/xcs/IO/FileSource.cpp\
	$(SRC)/xcs/IO/InflateLineReader.cpp \
	$(SRC)/xcs/IO/InflateSource.cpp \
	$(SRC)/xcs/IO/LineSplitter.cpp \
	\
	$(SRC)/xcs/IO/Async/GlobalIOThread.cpp \
	$(SRC)/xcs/IO/Async/IOThread.cpp \

XCS_UTIL := \
	$(SRC)/xcs/Util/tstring.cpp \
	$(SRC)/xcs/Util/ConvertString.cpp \
	$(SRC)/xcs/Util/TruncateString.cpp \

XCS_OS := \
	$(XCS_UTIL) \
	$(SRC)/xcs/OS/Clock.cpp\

ifeq ($(CONFIG_LINUX),y) 
# linux target

XCS_OS += \
    $(XCS_OS_LINUX) \

endif
