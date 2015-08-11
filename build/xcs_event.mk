# Event Library imported from xcsoar

XCS_EVENT_CONSOLE := \
        $(SRC)/xcs/Event/Poll/Loop.cpp \
	$(SRC)/xcs/Event/Poll/Queue.cpp \
	$(SRC)/xcs/Event/Poll/InputQueue.cpp \

XCS_EVENT_LINUX := \
	$(SRC)/xcs/Event/Poll/Linux/AllInput.cpp \
	$(SRC)/xcs/Event/Poll/Linux/Input.cpp \
	$(SRC)/xcs/Event/Poll/Linux/MergeMouse.cpp \
	$(SRC)/xcs/Event/Poll/Linux/SignalListener.cpp \

unused = \
	$(SRC)/xcs/Event/Poll/Linux/TTYKeyboard.cpp \
	$(SRC)/xcs/Event/Poll/Linux/Mouse.cpp \

XCS_EVENT_SHARED := \
	$(SRC)/xcs/Event/Shared/TimerQueue.cpp \
	$(SRC)/xcs/Event/Shared/Timer.cpp \

XCS_EVENT_GDI := \
	$(SRC)/xcs/Event/GDI/Loop.cpp \
	$(SRC)/xcs/Event/GDI/Queue.cpp \
	$(SRC)/xcs/Event/GDI/Transcode.cpp \
	
XCS_EVENT_SDL := \
	$(SRC)/xcs/Event/SDL/Loop.cpp \
	$(SRC)/xcs/Event/SDL/Queue.cpp \
	

XCS_EVENT := \
	$(SRC)/xcs/Event/Globals.cpp \
	$(SRC)/xcs/Event/Idle.cpp \
	$(XCS_EVENT_SHARED) \


ifeq ($(CONFIG_LINUX),y) 
# linux target
XCS_EVENT += \
	

ifeq ($(USE_SDL),y)
# linux target with SDL Event
XCS_EVENT += \
	$(XCS_EVENT_SDL) \

else
# linux target with console event


XCS_EVENT += \
	$(XCS_EVENT_CONSOLE) \
	$(XCS_EVENT_LINUX) \
	
endif

else
# win32

XCS_EVENT += \
	$(XCS_EVENT_GDI) \
	
endif

