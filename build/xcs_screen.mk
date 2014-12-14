# Event Library imported from xcsoar

XCS_SCREEN_MEMORY := \
	$(SRC)/Screen/Memory/Bitmap.cpp \
	$(SRC)/Screen/Memory/SubCanvas.cpp \
	$(SRC)/Screen/Memory/Canvas.cpp \
	$(SRC)/Screen/Memory/VirtualCanvas.cpp \

XCS_SCREEN_FB := \
	$(SRC)/Screen/FB/Window.cpp \
	$(SRC)/Screen/FB/SingleWindow.cpp \
	$(SRC)/Screen/FB/TopWindow.cpp \
	$(SRC)/Screen/FB/TopCanvas.cpp \

XCS_SCREEN_TTY := \
	$(SRC)/Screen/TTY/TopCanvas.cpp \

XCS_SCREEN_CUSTOM := \
	$(SRC)/Screen/Custom/Bitmap.cpp \
	$(SRC)/Screen/Custom/LibPNG.cpp \
	$(SRC)/Screen/Custom/LibJPEG.cpp \
	$(SRC)/Screen/Custom/Pen.cpp \
	$(SRC)/Screen/Custom/Files.cpp \
	$(SRC)/Screen/Custom/WList.cpp \
	$(SRC)/Screen/Custom/Window.cpp \
	$(SRC)/Screen/Custom/SingleWindow.cpp \
	$(SRC)/Screen/Custom/TopWindow.cpp \
	$(SRC)/Screen/Custom/ContainerWindow.cpp \
	$(SRC)/Screen/Custom/TextWindow.cpp \
	$(SRC)/Screen/Custom/Cache.cpp \
	$(SRC)/Screen/Custom/MoreCanvas.cpp \

XCS_SCREEN_FREETYPE := \
	$(SRC)/Screen/FreeType/Font.cpp \
	$(SRC)/Screen/FreeType/Init.cpp \

XCS_SCREEN_GDI := \
	$(SRC)/Screen/GDI/Brush.cpp \
	$(SRC)/Screen/GDI/Bitmap.cpp \
	$(SRC)/Screen/GDI/Pen.cpp \
	$(SRC)/Screen/GDI/Font.cpp \
	$(SRC)/Screen/GDI/Init.cpp \

XCS_SCREEN_SDL:= \
	$(SRC)/Screen/SDL/TopWindow.cpp \
	$(SRC)/Screen/SDL/TopCanvas.cpp \
	$(SRC)/Screen/SDL/SingleWindow.cpp \
	$(SRC)/Screen/SDL/Init.cpp \

XCS_SCREEN := \
	$(SRC)/Screen/Util.cpp \
	$(SRC)/Screen/Window.cpp \
	$(SRC)/Screen/SingleWindow.cpp \


XCS_UTILS := \
	$(SRC)/Util/UTF8.cpp\
	$(SRC)/Util/StaticString.cpp\
	$(SRC)/Util/StringUtil.cpp\

XCS_IO_ASYNC := \
	$(SRC)/IO/Async/DiscardFileEventHandler.cpp\
	$(SRC)/IO/Async/IOLoop.cpp\

XCS_HARDWARE := \
	$(SRC)/Hardware/CPU.cpp \


	
ifeq ($(CONFIG_LINUX),y) 
# linux target

XCS_SCREEN += \
	$(XCS_SCREEN_FREETYPE) \
	$(XCS_SCREEN_MEMORY) \
	$(XCS_SCREEN_TTY) \
	$(XCS_SCREEN_CUSTOM) \
	\
	$(XCS_HARDWARE) \
	$(XCS_IO_ASYNC) \
	$(XCS_UTILS) \
	$(XCS_IO_ASYNC) \
	$(XCS_HARDWARE) \



ifeq ($(USE_SDL),y)
# linux target with SDL Screen

XCS_SCREEN += \
	$(XCS_SCREEN_SDL) \

else

XCS_SCREEN += \
	$(XCS_SCREEN_FB) \
    


endif
	
else
#win32

XCS_SCREEN := \
    $(XCS_SCREEN_GDI) \

endif