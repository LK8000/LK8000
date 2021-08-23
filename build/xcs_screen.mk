# Event Library imported from xcsoar

XCS_SCREEN_MEMORY := \
	$(SRC)/xcs/Screen/Memory/Bitmap.cpp \
	$(SRC)/xcs/Screen/Memory/SubCanvas.cpp \
	$(SRC)/xcs/Screen/Memory/Canvas.cpp \
	$(SRC)/xcs/Screen/Memory/VirtualCanvas.cpp \
	$(SRC)/xcs/Screen/Memory/RawBitmap.cpp \
	$(SRC)/xcs/Screen/Memory/Dither.cpp \
	$(SRC)/xcs/Screen/Memory/Export.cpp \
	
XCS_SCREEN_FB := \
	$(SRC)/xcs/Screen/FB/Window.cpp \
	$(SRC)/xcs/Screen/FB/SingleWindow.cpp \
	$(SRC)/xcs/Screen/FB/TopWindow.cpp \
	$(SRC)/xcs/Screen/FB/TopCanvas.cpp \
	$(SRC)/xcs/Screen/FB/Init.cpp \

XCS_SCREEN_TTY := \
	$(SRC)/xcs/Screen/TTY/TopCanvas.cpp \

XCS_SCREEN_CUSTOM := \
	$(SRC)/xcs/Screen/Custom/Bitmap.cpp \
	$(SRC)/xcs/Screen/Custom/LibPNG.cpp \
	$(SRC)/xcs/Screen/Custom/Pen.cpp \
	$(SRC)/xcs/Screen/Custom/Files.cpp \
	$(SRC)/xcs/Screen/Custom/WList.cpp \
	$(SRC)/xcs/Screen/Custom/Window.cpp \
	$(SRC)/xcs/Screen/Custom/SingleWindow.cpp \
	$(SRC)/xcs/Screen/Custom/TopWindow.cpp \
	$(SRC)/xcs/Screen/Custom/ContainerWindow.cpp \
	$(SRC)/xcs/Screen/Custom/Cache.cpp \
	$(SRC)/xcs/Screen/Custom/MoreCanvas.cpp \

XCS_SCREEN_FREETYPE := \
	$(SRC)/xcs/Screen/FreeType/Font.cpp \
	$(SRC)/xcs/Screen/FreeType/Init.cpp \

XCS_SCREEN_GDI := \
	$(SRC)/xcs/Screen/GDI/Brush.cpp \
	$(SRC)/xcs/Screen/GDI/Bitmap.cpp \
	$(SRC)/xcs/Screen/GDI/Pen.cpp \
	$(SRC)/xcs/Screen/GDI/Font.cpp \
	$(SRC)/xcs/Screen/GDI/Init.cpp \
	$(SRC)/xcs/Screen/GDI/RawBitmap.cpp \

XCS_SCREEN_SDL:= \
	$(SRC)/xcs/Screen/SDL/TopWindow.cpp \
	$(SRC)/xcs/Screen/SDL/TopCanvas.cpp \
	$(SRC)/xcs/Screen/SDL/SingleWindow.cpp \
	$(SRC)/xcs/Screen/SDL/Init.cpp \
	
XCS_SCREEN_OPENGL := \
	$(SRC)/xcs/Screen/OpenGL/Bitmap.cpp \
	$(SRC)/xcs/Screen/OpenGL/Buffer.cpp \
	$(SRC)/xcs/Screen/OpenGL/BufferCanvas.cpp \
	$(SRC)/xcs/Screen/OpenGL/Canvas.cpp \
	$(SRC)/xcs/Screen/OpenGL/TopCanvas.cpp \
	$(SRC)/xcs/Screen/OpenGL/Extension.cpp \
	$(SRC)/xcs/Screen/OpenGL/Globals.cpp \
	$(SRC)/xcs/Screen/OpenGL/Init.cpp \
	$(SRC)/xcs/Screen/OpenGL/RawBitmap.cpp \
	$(SRC)/xcs/Screen/OpenGL/Shapes.cpp \
	$(SRC)/xcs/Screen/OpenGL/SubCanvas.cpp \
	$(SRC)/xcs/Screen/OpenGL/Texture.cpp \
	$(SRC)/xcs/Screen/OpenGL/UncompressedImage.cpp \
	$(SRC)/xcs/Screen/OpenGL/Triangulate.cpp \
	$(SRC)/xcs/Screen/OpenGL/VertexArray.cpp \
	$(SRC)/xcs/Screen/OpenGL/Rotate.cpp \
	$(SRC)/xcs/Screen/OpenGL/FBO.cpp \
	$(SRC)/xcs/Screen/OpenGL/Dynamic.cpp \
	\
	$(SRC)/xcs/Math/Angle.cpp \

ifeq ($(GLES2),y)	
XCS_SCREEN_OPENGL += \
    $(SRC)/xcs/Screen/OpenGL/Shaders.cpp \

endif

XCS_SCREEN_EGL := \
	$(SRC)/xcs/Screen/EGL/Init.cpp \
	$(SRC)/xcs/Screen/EGL/TopCanvas.cpp \
	$(SRC)/xcs/Screen/FB/Window.cpp \
	$(SRC)/xcs/Screen/FB/TopWindow.cpp \
	$(SRC)/xcs/Screen/FB/SingleWindow.cpp	

XCS_SCREEN_X11 := \
	$(SRC)/xcs/Screen/X11/TopWindow.cpp \

XCS_SCREEN_WAYLAND += \
	$(SRC)/xcs/Screen/Wayland/TopWindow.cpp\

XCS_SCREEN := \
	$(SRC)/xcs/Screen/Util.cpp \
	$(SRC)/xcs/Screen/Window.cpp \
	$(SRC)/xcs/Screen/SingleWindow.cpp \
	$(SRC)/xcs/Screen/Color.cpp \
	$(SRC)/xcs/Screen/Debug.cpp \
	$(SRC)/xcs/Screen/BufferCanvas.cpp \
	$(SRC)/xcs/Thread/Debug.cpp \

XCS_UTILS := \
	$(SRC)/xcs/Util/UTF8.cpp\
	$(SRC)/xcs/Util/StaticString.cpp\
	$(SRC)/xcs/Util/StringUtil.cpp\

XCS_IO_ASYNC := \
	$(SRC)/xcs/IO/Async/DiscardFileEventHandler.cpp\
	$(SRC)/xcs/IO/Async/IOLoop.cpp\

XCS_HARDWARE := \
	$(SRC)/xcs/Hardware/CPU.cpp \
	$(SRC)/xcs/Hardware/RotateDisplay.cpp \


	
ifeq ($(CONFIG_LINUX),y) 
# linux target

XCS_SCREEN += \
	$(XCS_SCREEN_FREETYPE) \
	$(XCS_SCREEN_TTY) \
	$(XCS_SCREEN_CUSTOM) \
	\
	$(XCS_HARDWARE) \
	$(XCS_IO_ASYNC) \
	$(XCS_UTILS) \
	$(XCS_IO_ASYNC) \
	$(XCS_HARDWARE) \
	$(SRC)/xcs/Screen/Debug.cpp \
	$(SRC)/xcs/Thread/Debug.cpp \
	
    ifeq ($(USE_SDL),y)
    # linux target with SDL Screen
	XCS_SCREEN += \
	    $(XCS_SCREEN_SDL) \

    endif

    ifeq ($(OPENGL),y)
    # linux target with OpenGL
	XCS_SCREEN += \
	    $(XCS_SCREEN_OPENGL) \
    
    else
	# linux target with Memory canvas
	XCS_SCREEN += \
	    $(XCS_SCREEN_MEMORY) \

    endif

    ifeq ($(USE_SDL)$(OPENGL),nn)
	# linux target with Memory canvas & FrameBuffer
	XCS_SCREEN += \
	    $(XCS_SCREEN_FB) \
	
    endif

    ifeq ($(USE_EGL),y)
	# linux target with EGL
	XCS_SCREEN += $(XCS_SCREEN_EGL) 
    endif

    ifeq ($(USE_X11),y)
	# linux target with X11
	XCS_SCREEN += $(XCS_SCREEN_X11) 
    endif

    ifeq ($(USE_WAYLAND),y)
	# linux target with Wayland
	XCS_SCREEN += $(XCS_SCREEN_WAYLAND) 
    endif

else
#win32

XCS_SCREEN = \
    $(XCS_UTILS) \
    $(XCS_SCREEN_GDI) \
    $(SRC)/xcs/Screen/Debug.cpp \
    $(SRC)/xcs/Thread/Debug.cpp \

endif
