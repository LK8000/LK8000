# OS Library imported from xcsoar

XCS_OS_LINUX := \
	$(SRC)/OS/Linux/CpuLoad.cpp \
	$(SRC)/OS/Linux/Memory.cpp \
	$(SRC)/OS/Linux/RotateScreen.cpp\
	\
	$(SRC)/OS/EventPipe.cpp\
	$(SRC)/OS/FileDescriptor.cpp\
	$(SRC)/OS/Poll.cpp\

XCS_OS := \
	$(SRC)/OS/Clock.cpp\

ifeq ($(CONFIG_LINUX),y) 
# linux target

XCS_OS += \
    $(XCS_OS_LINUX) \

endif