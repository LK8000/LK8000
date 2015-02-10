
LK_OS_LINUX := \
	$(SRC)/OS/Linux/CpuLoad.cpp \
	$(SRC)/OS/Linux/Memory.cpp \
	$(SRC)/OS/Linux/RotateScreen.cpp\
	$(SRC)/OS/Linux/lscpu/lscpu.cpp\
	$(SRC)/OS/Linux/lscpu/cpuset.cpp\


ifeq ($(CONFIG_LINUX),y) 
# linux target

LK_OS += \
    $(LK_OS_LINUX) \

endif
