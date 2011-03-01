#
SRC=src
HDR=src
LK_SRC=../../Source
LK_HDR=../../Header
BIN=bin/$(TARGET)

#
PROFILE		:= 
OPTIMIZE	:=-O2 -g
#OPTIMIZE	:=-O3 -funroll-all-loops
CONFIG_PPC2002	:=n
CONFIG_PPC2003	:=n
CONFIG_ALTAIR	:=n
CONFIG_PC	:=n
CONFIG_WINE	:=n
ALTAIR_PORTRAIT :=n
CONFIG_PNA	:=n
MINIMAL		:=n
XSCALE		:=n
GTARGET		:=$(TARGET)

ifeq ($(TARGET),PPC2002)
  CONFIG_PPC2002	:=y
else
  ifeq ($(TARGET),PPC2003)
    CONFIG_PPC2003	:=y
  else
    ifeq ($(TARGET),PPC2003X)
      CONFIG_PPC2003	:=y
      XSCALE :=y
      GTARGET := PPC2003
    else
      ifeq ($(TARGET),PC)
        CONFIG_PC	:=y
      else
        ifeq ($(TARGET),WINE)
          CONFIG_WINE :=y
        else
          ifeq ($(TARGET),ALTAIR)
            CONFIG_ALTAIR	:=y  
	    MINIMAL       :=y
	    XSCALE	:=y
          endif
          ifeq ($(TARGET),ALTAIRPORTRAIT)
            CONFIG_ALTAIR	:=y
	    ALTAIR_PORTRAIT :=y
	    MINIMAL       :=y
	    XSCALE	:=y
          endif
	  ifeq ($(TARGET),PNA)
	    CONFIG_PNA := y
	    CONFIG_PPC2003 := y
	    MINIMAL       :=n
	  endif
	endif
      endif
    endif
  endif
endif

############# build and CPU info

ifeq ($(CONFIG_PC),y)
TCPATH		:=i586-mingw32msvc-
CPU		:=i586
MCPU		:= -mcpu=$(CPU) 
else
ifeq ($(CONFIG_WINE),y)
TCPATH		:=wine
CPU		:=i586
MCPU		:= -mcpu=$(CPU) 
else
TCPATH		:=arm-mingw32ce-

ifeq ($(XSCALE),y)
CPU		:=xscale
MCPU		:= -mcpu=$(CPU) 
else
CPU		:=
MCPU		:=
endif

ifeq ($(TARGET),PNA)
CPU		:=arm1136j-s
MCPU		:=
endif
ifeq ($(CONFIG_PPC2002),y)
CPU		:=strongarm1110
MCPU		:= -mcpu=$(CPU)
endif

endif
endif

############# platform info

ifeq ($(CONFIG_PPC2002),y)
CE_MAJOR	:=3
CE_MINOR	:=00
CE_PLATFORM	:=310
TARGET		:=PPC2002
PCPU		:=ARM
endif
ifeq ($(CONFIG_PPC2003),y)
CE_MAJOR	:=4
CE_MINOR	:=00
CE_PLATFORM	:=400
PCPU		:=ARMV4
endif

# JMW this shouldn't be required VENTA FIX
#ifeq ($(CONFIG_PNA),y)
#CE_MAJOR	:=5
#CE_MINOR	:=00
#CE_PLATFORM	:=500
#endif

ifeq ($(CONFIG_ALTAIR),y)
# armv4i
CE_MAJOR	:=5
CE_MINOR	:=00
CE_PLATFORM	:=500
TARGET		:=ALTAIR
ifeq ($(ALTAIR_PORTRAIT),y)
TARGET          :=ALTAIRPORTRAIT
endif

endif

ifeq ($(CONFIG_PC),y)
# armv4i
CE_MAJOR	:=5
CE_MINOR	:=00
CE_PLATFORM	:=500
TARGET		:=PC
endif
ifeq ($(CONFIG_WINE),y)
# armv4i
CE_MAJOR	:=5
CE_MINOR	:=00
CE_PLATFORM	:=500
TARGET		:=WINE
CONFIG_PC	:=y
endif

######## output files

OUTPUTS 	:= TestContest-$(TARGET).exe

######## tools

EXE		:=$(findstring .exe,$(MAKE))
AR		:=$(TCPATH)ar$(EXE)
CXX		:=$(TCPATH)g++$(EXE)
CC		:=$(TCPATH)gcc$(EXE)
SIZE		:=$(TCPATH)size$(EXE)
STRIP		:=$(TCPATH)strip$(EXE)
WINDRES		:=$(TCPATH)windres$(EXE)
SYNCE_PCP	:=synce-pcp
SYNCE_PRM	:=synce-prm
CE_VERSION	:=0x0$(CE_MAJOR)$(CE_MINOR)
ARFLAGS		:=r
MKDIR           :=mkdir -p

######## windows definitions

ifeq ($(CONFIG_PC),y)
CE_DEFS		:=-D_WIN32_WINDOWS=$(CE_VERSION) -DWINVER=$(CE_VERSION) 
CE_DEFS		+=-D_WIN32_IE=$(CE_VERSION) -DWINDOWSPC=1 
CE_DEFS		+=-D_REALTHING_
else
CE_DEFS		:=-D_WIN32_WCE=$(CE_VERSION) -D_WIN32_IE=$(CE_VERSION)
CE_DEFS		+=-DWIN32_PLATFORM_PSPC=$(CE_PLATFORM)
endif

ifeq ($(CONFIG_PPC2002),y)
CE_DEFS		+=-DPPC2002=1 
endif
ifeq ($(CONFIG_PPC2003),y)
CE_DEFS		+=-DPPC2003=1 
endif


UNICODE		:= -DUNICODE -D_UNICODE

######## paths

ifeq ($(CONFIG_WINE),y)
INCLUDES	:= -I$(LK_HDR)/mingw32compat -I$(LK_HDR) -I$(LK_SRC) -I$(SRC) 
else
INCLUDES	:= -I$(LK_HDR)/mingw32compat -I$(LK_HDR) -I$(LK_SRC) -I$(SRC) 
endif

######## compiler flags

CPPFLAGS	:= $(INCLUDES) $(CE_DEFS) 
CPPFLAGS	+= -DNDEBUG -DFIVV
#CPPFLAGS	+= -DFLARM_AVERAGE  NOW INSIDE options.h
#CPPFLAGS	+= -Wchar-subscripts -Wformat -Winit-self -Wimplicit -Wmissing-braces -Wparentheses -Wreturn-type 
#CPPFLAGS	+= -Wunused-label -Wunused-variable -Wunused-value -Wuninitialized 

CPPFLAGS	+= -Wall -Wno-write-strings -Wno-char-subscripts -Wno-switch
#CPPFLAGS	+= -Wall -Wno-non-virtual-dtor 
#CPPFLAGS	+= -Wno-char-subscripts -Wno-switch

#CPPFLAGS	+= -Wshadow 
#CPPFLAGS	+= -Wsign-compare -Wsign-conversion 
ifeq ($(CONFIG_PNA),y)
#CPPFLAGS	+= -DBIGDISPLAY -DCECORE -DPNA -DNOLINETO 
CPPFLAGS	+= -DCECORE -DPNA 
endif

ifeq ($(CONFIG_PC),y)
CPPFLAGS	+= -D_WINDOWS -D_MBCS -DWIN32 -DCECORE -DUNDER_CE=300 $(UNICODE)
  ifeq ($(CONFIG_WINE),y)
CPPFLAGS	+= -D__MINGW32__ 
# -mno-cygwin 
  else
CPPFLAGS	+= $(UNICODE)
  endif
else
CPPFLAGS	+= -D_ARM_ $(UNICODE)
  ifeq ($(CONFIG_ALTAIR),y)
CPPFLAGS 	+=-IPPC2005 -DGNAV
    ifeq ($(ALTAIR_PORTRAIT),y)
CPPFLAGS	+= -DFORCEPORTRAIT
    endif
  endif
endif

CXXFLAGS	:=$(OPTIMIZE) -finput-charset=utf-8 $(PROFILE)
CFLAGS		:=$(OPTIMIZE) $(PROFILE)

####### linker configuration

LDFLAGS		:=-Wl,--major-subsystem-version=$(CE_MAJOR)
LDFLAGS		+=-Wl,--minor-subsystem-version=$(CE_MINOR)
ifeq ($(CONFIG_PC),y)
LDFLAGS		+=-Wl,-subsystem,console
endif
LDFLAGS		+=$(PROFILE)

ifeq ($(CONFIG_PC),y)
LDLIBS		:= -lmingw32 -lcomctl32 -lkernel32 -luser32 -lgdi32 -ladvapi32 -lwinmm -lmsimg32 -lstdc++
else
  LDLIBS		:= -lcommctrl -lstdc++ 
  ifeq ($(MINIMAL),n)
    LDLIBS		+= -laygshell 
    ifneq ($(TARGET),PNA)
      LDLIBS		+= -limgdecmp
    endif
  endif
endif

####### compiler target

ifeq ($(CONFIG_PC),y)
TARGET_ARCH	:=-mwindows -march=i586 -mms-bitfields
else

TARGET_ARCH	:=-mwin32 $(MCPU)
ifeq ($(TARGET),PNA)
TARGET_ARCH	:=-mwin32
endif

endif
WINDRESFLAGS	:=-I$(LK_HDR) -I$(LK_SRC) -I$(SRC) $(CE_DEFS) -D_MINGW32_
ifeq ($(CONFIG_ALTAIR),y)
WINDRESFLAGS	+=-DGNAV
endif
MAKEFLAGS	+=-r

####### build verbosity

# Internal - Control verbosity
#  make V=0 - quiet
#  make V=1 - terse (default)
#  make V=2 - show commands
ifeq ($(V),2)
Q		:=
NQ		:=\#
else
Q		:=@
ifeq ($(V),0)
NQ		:=\#
else
NQ		:=
endif
endif

ifeq ($(CONFIG_PC),n)
#CPPFLAGS_Common_Source_ :=-Werror
endif

####### sources

SRC_FILES :=\
	$(SRC)/main.cpp \
	$(SRC)/TestContest.cpp \
	$(SRC)/TestContestKMLWrapper.cpp \
	$(SRC)/ReplayLogger.cpp \
	$(SRC)/ContestMgr.cpp \
	$(SRC)/Trace.cpp \
	$(SRC)/TimeStamp.cpp \
	$(SRC)/Tools.cpp \
	$(SRC)/Utils.cpp \
#	$(SRC)/PointGPS.cpp \


####### compilation outputs

OBJS 	:=\
	$(patsubst $(SRC)%.cpp,$(BIN)%.o,$(SRC_FILES))
OBJS 	:=\
	$(patsubst $(LK_SRC)%.cpp,$(BIN)%.o,$(OBJS))

IGNORE	:= \( -name .git \) -prune -o


####### dependency handling

DEPFILE		=$(dir $@).$(notdir $@).d
DEPFLAGS	=-Wp,-MD,$(DEPFILE)
dirtarget	=$(subst \\,_,$(subst /,_,$(dir $@)))
cc-flags	=$(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_$(dirtarget)) $(TARGET_ARCH)
cxx-flags	=$(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(CPPFLAGS_$(dirtarget)) $(TARGET_ARCH)


####### targets

.PHONY: FORCE all clean cleani tags

all:	$(OUTPUTS)

clean: cleani
	rm -rf $(BIN)

cleani:
	find . $(IGNORE) \( -name '*.i' \) -type f -print | xargs -r $(RM)

tags:
	$(Q)etags --declarations --output=TAGS `find . -name *\\\.[ch] -or -name *\\\.[ch]pp`
	$(Q)ebrowse -s `find . -name *\\\.[ch] -or -name *\\\.[ch]pp`


#
# Useful debugging targets - make preprocessed versions of the source
#
%.i: %.cpp FORCE
	$(CXX) $(cxx-flags) -E $(OUTPUT_OPTION) $<

%.i: %.c FORCE
	$(CC) $(cc-flags) -E $(OUTPUT_OPTION) $<

%.s: %.cpp FORCE
	$(CXX) $(cxx-flags) -S $(OUTPUT_OPTION) $<



####### rules

TestContest-$(TARGET).exe: TestContest-$(TARGET)-ns.exe
	@$(NQ)echo "  STRIP   $@"
	$(Q)$(STRIP) $< -o $@
	$(Q)$(SIZE) $@
#	$(RM) TestContest-$(TARGET)-ns.exe

TestContest-$(TARGET)-ns.exe: $(OBJS)
	@$(NQ)echo "  LINK    $@"
	$(Q)$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(BIN)/%.o: $(SRC)/%.c
	@$(NQ)echo "  CC      $@"
	$(Q)$(MKDIR) $(dir $@)
	$(Q)$(CC) $(cc-flags) -c $(OUTPUT_OPTION) $<
	@sed -i '1s,^[^ :]*,$@,' $(DEPFILE)

$(BIN)/%.o: $(SRC)/%.cpp
	@$(NQ)echo "  CXX     $@"
	$(Q)$(MKDIR) $(dir $@)
	$(Q)$(CXX) $(cxx-flags) -c $(OUTPUT_OPTION) $<
	@sed -i '1s,^[^ :]*,$@,' $(DEPFILE)

$(BIN)/%.o: $(LK_SRC)/%.cpp
	@$(NQ)echo "  CXX     $@"
	$(Q)$(MKDIR) $(dir $@)
	$(Q)$(CXX) $(cxx-flags) -c $(OUTPUT_OPTION) $<
	@sed -i '1s,^[^ :]*,$@,' $(DEPFILE)


####### include depends files

ifneq ($(wildcard $(BIN)/.*.d),)
include $(wildcard $(BIN)/.*.d)
endif
ifneq ($(wildcard $(BIN)/*/.*.d),)
include $(wildcard $(BIN)/*/.*.d)
endif
