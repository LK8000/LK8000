
include build/xcs_os.mk

ifeq ($(CONFIG_LINUX),y) 
# linux target
LK_OS := \
	$(SRC)/OS/Linux/CpuLoad.cpp \
	$(SRC)/OS/Linux/Memory.cpp \
	$(SRC)/OS/Linux/RotateScreen.cpp\
	$(SRC)/OS/Linux/lscpu/lscpu.cpp\
	$(SRC)/OS/Linux/lscpu/cpuset.cpp\
	
endif

ifeq ($(CONFIG_WIN32),y)
# Windows & WinCE Target
LK_OS := \
	$(SRC)/OS/Win/CpuLoad.cpp \
	$(SRC)/OS/Win/Memory.cpp \
	$(SRC)/OS/Win/RotateScreen.cpp\
	$(SRC)/OS/Win/lscpu.cpp\

endif

LK_OS += \
	$(XCS_OS) \
