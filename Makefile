#
SRC=Common/Source
HDR=Common/Header
#
PROFILE		:= 
OPTIMIZE	:=-O2
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
TCPATH		:=i386-mingw32-
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
# FIX
#OUTPUTS 	:= LK8000SIM-$(TARGET).exe LK8000-$(TARGET).exe
OUTPUTS 	:= LK8000-$(TARGET).exe
#OUTPUTS 	:= LK8000SIM-$(TARGET).exe
ifeq ($(CONFIG_PNA),y)
#OUTPUTS 	:= LK8000SIM-$(TARGET).exe LK8000-$(TARGET).exe
#OUTPUTS 	:= LK8000SIM-$(TARGET).exe
OUTPUTS 	:= LK8000-$(TARGET).exe
endif
ifeq ($(CONFIG_PC),y)
#OUTPUTS 	:= LK8000SIM-$(TARGET).exe LK8000-$(TARGET).exe
#OUTPUTS 	:= LK8000SIM-$(TARGET).exe
OUTPUTS 	:= LK8000-$(TARGET).exe
endif
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
INCLUDES	:= -I$(HDR)/mingw32compat -I$(HDR) -I$(SRC) 
else
INCLUDES	:= -I$(HDR)/mingw32compat -I$(HDR) -I$(SRC) 
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

CXXFLAGS	:=$(OPTIMIZE) -fno-exceptions $(PROFILE)
CFLAGS		:=$(OPTIMIZE) $(PROFILE)

####### linker configuration

LDFLAGS		:=-Wl,--major-subsystem-version=$(CE_MAJOR)
LDFLAGS		+=-Wl,--minor-subsystem-version=$(CE_MINOR)
ifeq ($(CONFIG_PC),y)
LDFLAGS		+=-Wl,-subsystem,windows
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
WINDRESFLAGS	:=-I$(HDR) -I$(SRC) $(CE_DEFS) -D_MINGW32_
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

####### sources

ifeq ($(CONFIG_PC),n)
#CPPFLAGS_Common_Source_ :=-Werror
endif

DEVS	:=\
	$(SRC)/devBorgeltB50.o \
	$(SRC)/devCAI302.o \
	$(SRC)/devCaiGpsNav.o \
	$(SRC)/devCondor.o \
	$(SRC)/devEW.o \
	$(SRC)/devEWMicroRecorder.o \
	$(SRC)/devFlymasterF1.o \
	$(SRC)/devCompeo.o \
	$(SRC)/devDigifly.o \
	$(SRC)/devGeneric.o \
	$(SRC)/devLX.o \
	$(SRC)/devNmeaOut.o \
	$(SRC)/devPosiGraph.o \
	$(SRC)/devVolkslogger.o \
	$(SRC)/devXCOM760.o \
	$(SRC)/devZander.o \
	$(SRC)/devIlec.o

DLGS	:=\
	$(SRC)/dlgAirspace.o \
	$(SRC)/dlgAirspaceColours.o \
	$(SRC)/dlgAirspaceDetails.o \
	$(SRC)/dlgAirspacePatterns.o \
	$(SRC)/dlgAirspaceSelect.o \
	$(SRC)/dlgAirspaceWarning.o \
	$(SRC)/dlgBasicSettings.cpp \
	$(SRC)/dlgChecklist.o \
	$(SRC)/dlgConfiguration.o \
	$(SRC)/dlgConfiguration2.o \
	$(SRC)/dlgHelp.o \
	$(SRC)/dlgLoggerReplay.o \
	$(SRC)/dlgStartPoint.o \
	$(SRC)/dlgStartup.o \
	$(SRC)/dlgStatistics.o \
	$(SRC)/dlgStatus.o \
	$(SRC)/dlgStatusSystem.o \
	$(SRC)/dlgTarget.o \
	$(SRC)/dlgTaskCalculator.o \
	$(SRC)/dlgTaskOverview.o \
	$(SRC)/dlgTaskRules.o \
	$(SRC)/dlgTimeGates.o \
	$(SRC)/dlgTopology.o \
	$(SRC)/dlgTaskWaypoint.o \
	$(SRC)/dlgTeamCode.o \
	$(SRC)/dlgTextEntry.o \
	$(SRC)/dlgTextEntry_Keyboard.o \
	$(SRC)/dlgTools.o \
	$(SRC)/dlgWayPointDetails.o \
	$(SRC)/dlgWayQuick.o \
	$(SRC)/dlgWaypointEdit.o \
	$(SRC)/dlgWayPointSelect.o \
	$(SRC)/dlgWaypointOutOfTerrain.o \
	$(SRC)/dlgWeather.o \
	$(SRC)/dlgWindSettings.o \
	$(SRC)/dlgStartTask.o \
	$(SRC)/dlgFontEdit.o \
	$(SRC)/dlgLKTraffic.o \
	$(SRC)/dlgCustomKeys.o \
	$(SRC)/dlgProfiles.o \

VOLKS	:=\
	$(SRC)/Volkslogger/dbbconv.cpp \
	$(SRC)/Volkslogger/grecord.cpp \
	$(SRC)/Volkslogger/vlapi2.cpp \
	$(SRC)/Volkslogger/vlapihlp.cpp \
	$(SRC)/Volkslogger/vlapisys_win.cpp \
	$(SRC)/Volkslogger/vlconv.cpp \
	$(SRC)/Volkslogger/vlutils.cpp

#	$(SRC)/dlgConfigurationVario.o \
#	$(SRC)/dlgVoice.o \
#	$(SRC)/dlgVegaDemo.o \
#	$(SRC)/dlgFlarmTraffic.o \
#	$(SRC)/dlgSwitches.o \
#	$(SRC)/dlgBrightness.o \

OBJS	:=\
	$(SRC)/AATDistance.o 		$(SRC)/AirfieldDetails.o \
	$(SRC)/Airspace.o 		$(SRC)/AirspaceColourDlg.o \
	$(SRC)/AirspaceWarning.o 	$(SRC)/Atmosphere.o \
	$(SRC)/Calculations.o 		$(SRC)/Calculations2.o \
	$(SRC)/ClimbAverageCalculator.o $(SRC)/LKCalculations.o \
	$(SRC)/ConditionMonitor.o 	$(SRC)/device.o \
	$(SRC)/Dialogs.o 		$(SRC)/LKProcess.o \
	$(SRC)/FlarmIdFile.o 		$(SRC)/FlarmCalculations.o \
	$(SRC)/Geoid.o \
	$(SRC)/InfoBox.o 		$(SRC)/InfoBoxLayout.o \
	$(SRC)/InputEvents.o 		$(SRC)/leastsqs.o \
	$(SRC)/Logger.o 		$(SRC)/LKMapWindow.o \
	$(SRC)/LKDrawLook8000.o 	$(SRC)/LKDrawNearest.o\
	$(SRC)/LKDrawCommon.o 		\
	$(SRC)/LKDrawInfoPage.o		$(SRC)/LKDrawWaypoints.o\
	$(SRC)/LKDrawTraffic.o		$(SRC)/LKSimulator.o\
	$(SRC)/LKBestAlternate.o \
	$(SRC)/MapWindow.o 		$(SRC)/MapWindow2.o \
	$(SRC)/MapWindow3.o 		$(SRC)/Utils2.o \
	$(SRC)/McReady.o 		$(SRC)/Message.o \
	$(SRC)/NavFunctions.o		$(SRC)/OnLineContest.o \
	$(SRC)/Parser.o			$(SRC)/Port.o \
	$(SRC)/Process.o 		$(SRC)/dlgComboPicker.o \
	$(SRC)/RasterTerrain.o		$(SRC)/rscalc.o \
	$(SRC)/StdAfx.o			$(SRC)/STScreenBuffer.o \
	$(SRC)/Task.o			$(SRC)/TeamCodeCalculation.o \
	$(SRC)/Terrain.o		$(SRC)/ThermalLocator.o \
	$(SRC)/Topology.o		$(SRC)/units.o \
	$(SRC)/Utils.o			$(SRC)/Utm.o \
	$(SRC)/VOIMAGE.o		$(SRC)/LKObjects.o \
	$(SRC)/Waypointparser.o  	$(SRC)/LKUtils.o \
	$(SRC)/LKLanguage.o		$(SRC)/LKGeneralAviation.o \
	$(SRC)/windanalyser.o		$(SRC)/windmeasurementlist.o \
	$(SRC)/windstore.o 		$(SRC)/WindowControls.o \
	$(SRC)/WindZigZag.o 		$(SRC)/xmlParser.o \
	\
	$(SRC)/mapbits.o \
	$(SRC)/maperror.o 		$(SRC)/mapprimitive.o \
	$(SRC)/mapsearch.o		$(SRC)/mapshape.o \
	$(SRC)/maptree.o                $(SRC)/mapxbase.o \
	\
	$(SRC)/lk8000.o \
	$(DEVS) \
	$(DLGS:.cpp=.o) \
	$(VOLKS:.cpp=.o) \
	$(SRC)/lk8000.rsc \
	$(SRC)/jasper.a \
	$(SRC)/zzip.a \
	$(SRC)/compat.a


ZZIPSRC	:=$(SRC)/zzip
ZZIP	:=\
	$(ZZIPSRC)/adler32.c	 	$(ZZIPSRC)/compress.c \
	$(ZZIPSRC)/crc32.c 		$(ZZIPSRC)/deflate.c \
	$(ZZIPSRC)/err.c 		$(ZZIPSRC)/fetch.c \
	$(ZZIPSRC)/file.c 		\
	$(ZZIPSRC)/infback.c 		$(ZZIPSRC)/inffast.c \
	$(ZZIPSRC)/inflate.c 		$(ZZIPSRC)/info.c \
	$(ZZIPSRC)/inftrees.c 		$(ZZIPSRC)/plugin.c \
	$(ZZIPSRC)/trees.c 		$(ZZIPSRC)/uncompr.c \
	$(ZZIPSRC)/zip.c 		$(ZZIPSRC)/zstat.c \
	$(ZZIPSRC)/zutil.c

JASSRC	:=$(SRC)/jasper
JASPER	:=\
	$(JASSRC)/base/jas_cm.c 	$(JASSRC)/base/jas_debug.c \
	$(JASSRC)/base/jas_getopt.c	$(JASSRC)/base/jas_icc.c \
	$(JASSRC)/base/jas_iccdata.c 	$(JASSRC)/base/jas_image.c \
	$(JASSRC)/base/jas_init.c 	$(JASSRC)/base/jas_malloc.c \
	$(JASSRC)/base/jas_seq.c 	$(JASSRC)/base/jas_stream.c \
	$(JASSRC)/base/jas_string.c 	$(JASSRC)/base/jas_tvp.c \
	$(JASSRC)/base/jas_version.c	$(JASSRC)/jp2/jp2_cod.c \
	$(JASSRC)/jp2/jp2_dec.c 	$(JASSRC)/jpc/jpc_bs.c \
	$(JASSRC)/jpc/jpc_cs.c 		$(JASSRC)/jpc/jpc_dec.c \
	$(JASSRC)/jpc/jpc_math.c 	$(JASSRC)/jpc/jpc_mct.c \
	$(JASSRC)/jpc/jpc_mqdec.c       $(JASSRC)/jpc/jpc_mqcod.c \
	$(JASSRC)/jpc/jpc_qmfb.c 	$(JASSRC)/jpc/jpc_rtc.cpp \
	$(JASSRC)/jpc/jpc_t1dec.c 	$(JASSRC)/jpc/jpc_t1enc.c \
	$(JASSRC)/jpc/jpc_t1cod.c \
	$(JASSRC)/jpc/jpc_t2dec.c 	$(JASSRC)/jpc/jpc_t2cod.c \
	$(JASSRC)/jpc/jpc_tagtree.c	$(JASSRC)/jpc/jpc_tsfb.c \
	$(JASSRC)/jpc/jpc_util.c 	$(JASSRC)/jpc/RasterTile.cpp

COMPATSRC:=$(SRC)/wcecompat
COMPAT	:=\
	$(COMPATSRC)/errno.cpp 		$(COMPATSRC)/string_extras.c \
	$(COMPATSRC)/ts_string.cpp 	$(COMPATSRC)/wtoi.c

all:	$(OUTPUTS)

####### products

LK8000-$(TARGET).exe: LK8000-$(TARGET)-ns.exe
	@$(NQ)echo "  STRIP   $@"
	$(Q)$(STRIP) $< -o $@
	$(Q)$(SIZE) $@
	./buildnumber
	$(RM) LK8000-$(TARGET)-ns.exe

LK8000-$(TARGET)-ns.exe: $(OBJS)
	@$(NQ)echo "  LINK    $@"
	$(Q)$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@


#
# Create libraries for zzip, jasper and compatibility stuff
#
$(SRC)/zzip.a: $(patsubst %.cpp,%.o,$(ZZIP:.c=.o))
	@$(NQ)echo "  AR      $@"
	$(Q)$(AR) $(ARFLAGS) $@ $^

$(SRC)/jasper.a: $(patsubst %.cpp,%.o,$(JASPER:.c=.o))
	@$(NQ)echo "  AR      $@"
	$(Q)$(AR) $(ARFLAGS) $@ $^

$(SRC)/compat.a: $(patsubst %.cpp,%.o,$(COMPAT:.c=.o))
	@$(NQ)echo "  AR      $@"
	$(Q)$(AR) $(ARFLAGS) $@ $^

####### shared objects
#
# Tell make how to create a compiled resource object (rsc)
#
%.rsc: %.rc
	@sed -e 's,[Bb]itmaps\\\\,Bitmaps/,g' \
	    -e 's,lk8000.ICO,lk8000.ico,g' \
	    -e 's,\.\.\\\\Data\\\\Dialogs\\\\,../Data/Dialogs/,g' \
	    -e 's,small\.bmp,Small.bmp,g' \
		< $< > $<.tmp
	@$(NQ)echo "  WINDRES $@"
	$(Q)$(WINDRES) $(WINDRESFLAGS) $<.tmp $@
	@$(RM) $<.tmp

####### dependency handling

DEPFILE		=$(dir $@).$(notdir $@).d
DEPFLAGS	=-Wp,-MD,$(DEPFILE)
dirtarget	=$(subst \\,_,$(subst /,_,$(dir $@)))
cc-flags	=$(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_$(dirtarget)) $(TARGET_ARCH)
cxx-flags	=$(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(CPPFLAGS_$(dirtarget)) $(TARGET_ARCH)

#
# Useful debugging targets - make preprocessed versions of the source
#
%.i: %.cpp FORCE
	$(CXX) $(cxx-flags) -E $(OUTPUT_OPTION) $<

%.s: %.cpp FORCE
	$(CXX) $(cxx-flags) -S $(OUTPUT_OPTION) $<

%.i: %.c FORCE
	$(CC) $(cc-flags) -E $(OUTPUT_OPTION) $<

####### build rules

#
# Provide our own rules for building...
#
%.o: %.c
	@$(NQ)echo "  CC      $@"
	$(Q)$(CC) $(cc-flags) -c $(OUTPUT_OPTION) $<
	@sed -i '1s,^[^ :]*,$@,' $(DEPFILE)

%.o: %.cpp
	@$(NQ)echo "  CXX     $@"
	$(Q)$(CXX) $(cxx-flags) -c $(OUTPUT_OPTION) $<
	@sed -i '1s,^[^ :]*,$@,' $(DEPFILE)

%.os: %.c
	@$(NQ)echo "  CC      $@"
	$(Q)$(CC) $(cc-flags) -D_SIM_ -c $(OUTPUT_OPTION) $<
	@sed -i '1s,^[^ :]*,$@,' $(DEPFILE)

%.os: %.cpp
	@$(NQ)echo "  CXX     $@"
	$(Q)$(CXX) $(cxx-flags) -D_SIM_ -c $(OUTPUT_OPTION) $<
	@sed -i '1s,^[^ :]*,$@,' $(DEPFILE)

IGNORE	:= \( -name .svn -o -name CVS -o -name .git \) -prune -o

clean: cleani FORCE
	find . $(IGNORE) \( -name '*.[oa]' -o -name '*.rsc' -o -name '*.os' -o -name '.*.d' \) \
	-type f -print | xargs -r $(RM)
	$(RM) LK8000-$(TARGET)-ns.exe 

cleani: FORCE
	find . $(IGNORE) \( -name '*.i' \) \
		-type f -print | xargs -r $(RM)

.PHONY: FORCE

ifneq ($(wildcard $(SRC)/.*.d),)
include $(wildcard $(SRC)/.*.d)
endif
ifneq ($(wildcard $(SRC)/*/.*.d),)
include $(wildcard $(SRC)/*/.*.d)
endif

