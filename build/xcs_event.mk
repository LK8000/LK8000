# Event Library imported from xcsoar

XCS_EVENT_CONSOLE := \
        $(SRC)/Event/Console/Loop.cpp \
	$(SRC)/Event/Console/Queue.cpp \

XCS_EVENT_LINUX := \
	$(SRC)/Event/Linux/TTYKeyboard.cpp \
	$(SRC)/Event/Linux/AllInput.cpp \
	$(SRC)/Event/Linux/Input.cpp \
	$(SRC)/Event/Linux/MergeMouse.cpp \
	$(SRC)/Event/Linux/Mouse.cpp \
	$(SRC)/Event/Linux/SignalListener.cpp \

XCS_EVENT_SHARED := \
	$(SRC)/Event/Shared/TimerQueue.cpp \
	$(SRC)/Event/Shared/Timer.cpp \

XCS_EVENT_GDI := \
	$(SRC)/Event/GDI/Loop.cpp \
	$(SRC)/Event/GDI/Queue.cpp \
	$(SRC)/Event/GDI/Transcode.cpp \
	
XCS_EVENT_SDL := \
	$(SRC)/Event/SDL/Loop.cpp \
	$(SRC)/Event/SDL/Queue.cpp \
	

XCS_EVENT := \
	$(SRC)/Event/Globals.cpp \
	$(SRC)/Event/Idle.cpp \
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

