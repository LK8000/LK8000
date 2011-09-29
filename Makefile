#
SRC=Common/Source
HDR=Common/Header
BIN=Bin/$(TARGET)

# enable/disable heap checking (dmalloc.h libdmalloc.a must be in ../dmalloc)
DMALLOC=n

PROFILE		:=
OPTIMIZE	:=-O2
#OPTIMIZE	:=-O3 -funroll-all-loops
CONFIG_PPC2002	:=n
CONFIG_PPC2003	:=n
CONFIG_PC	:=n
CONFIG_WINE	:=n
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

OUTPUTS 	:= LK8000-$(TARGET).exe

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
FIND            :=find
ETAGS           :=etags
EBROWSE         :=ebrowse

######## windows definitions

ifeq ($(CONFIG_PC),y)
CE_DEFS		:=-D_WIN32_WINDOWS=$(CE_VERSION) -DWINVER=$(CE_VERSION)
CE_DEFS		+=-D_WIN32_IE=$(CE_VERSION) -DWINDOWSPC=1
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
CPPFLAGS	+= -DNDEBUG 
#CPPFLAGS	+= -DFLARM_AVERAGE  NOW INSIDE options.h
#CPPFLAGS	+= -Wchar-subscripts -Wformat -Winit-self -Wimplicit -Wmissing-braces -Wparentheses -Wreturn-type
#CPPFLAGS	+= -Wunused-label -Wunused-variable -Wunused-value -Wuninitialized

CPPFLAGS	+= -Wall -Wno-write-strings -Wno-char-subscripts
#CPPFLAGS	+= -Wall -Wno-non-virtual-dtor
#CPPFLAGS	+= -Wno-char-subscripts -Wno-switch

#CPPFLAGS	+= -Wshadow
#CPPFLAGS	+= -Wsign-compare -Wsign-conversion
ifeq ($(CONFIG_PNA),y)
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
endif

ifeq ($(DMALLOC),y)
  CPPFLAGS += -DHC_DMALLOC
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
  LDLIBS := -Wl,-Bstatic -lstdc++  -lmingw32 -lcomctl32 -lkernel32 -luser32 -lgdi32 -ladvapi32 -lwinmm -lmsimg32
else
  LDLIBS := -Wl,-Bstatic -lstdc++  -Wl,-Bdynamic -lcommctrl
  ifeq ($(MINIMAL),n)
    LDLIBS		+= -laygshell 
    ifneq ($(TARGET),PNA)
      LDLIBS		+= -limgdecmp 
    endif
  endif
endif

ifeq ($(DMALLOC),y)
  LDLIBS += -L../dmalloc -ldmalloc
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

UTILS	:=\
	$(SRC)/utils/stringext.cpp
  
DEVS	:=\
	$(SRC)/devBase.cpp \
	$(SRC)/devBorgeltB50.cpp \
	$(SRC)/devCAI302.cpp \
	$(SRC)/devCaiGpsNav.cpp \
	$(SRC)/devCompeo.cpp \
	$(SRC)/devCondor.cpp \
	$(SRC)/devDigifly.cpp \
	$(SRC)/devDisabled.cpp \
	$(SRC)/devDSX.cpp \
	$(SRC)/devEW.cpp \
	$(SRC)/devEWMicroRecorder.cpp \
	$(SRC)/devFlymasterF1.cpp \
	$(SRC)/devFlytec.cpp \
	$(SRC)/devGeneric.cpp \
	$(SRC)/devIlec.cpp \
	$(SRC)/devIMI.cpp \
	$(SRC)/devNmeaOut.cpp \
	$(SRC)/devLKext1.cpp \
	$(SRC)/devLX.cpp \
	$(SRC)/devLXNano.cpp \
	$(SRC)/devPosiGraph.cpp \
	$(SRC)/devVolkslogger.cpp \
	$(SRC)/devXCOM760.cpp \
	$(SRC)/devZander.cpp \
	$(SRC)/devWesterboer.cpp \

DLGS	:=\
	$(SRC)/dlgAirspace.cpp \
	$(SRC)/dlgAirspaceWarningParams.cpp \
	$(SRC)/dlgAirspaceColours.cpp \
	$(SRC)/dlgAirspaceDetails.cpp \
	$(SRC)/dlgAirspacePatterns.cpp \
	$(SRC)/dlgAirspaceSelect.cpp \
	$(SRC)/dlgBasicSettings.cpp \
	$(SRC)/dlgBottomBar.cpp \
	$(SRC)/dlgChecklist.cpp \
	$(SRC)/dlgConfiguration.cpp \
	$(SRC)/dlgConfiguration2.cpp \
	$(SRC)/dlgCustomKeys.cpp \
	$(SRC)/dlgFontEdit.cpp \
	$(SRC)/dlgHelp.cpp \
	$(SRC)/dlgInfoPages.cpp \
	$(SRC)/dlgLKAirspaceWarning.cpp \
	$(SRC)/dlgLKTraffic.cpp \
	$(SRC)/dlgLoggerReplay.cpp \
	$(SRC)/dlgOracle.cpp \
	$(SRC)/dlgProfiles.cpp \
	$(SRC)/dlgStartPoint.cpp \
	$(SRC)/dlgStartTask.cpp \
	$(SRC)/dlgStartup.cpp \
	$(SRC)/dlgStatistics.cpp \
	$(SRC)/dlgStatus.cpp \
	$(SRC)/dlgStatusSystem.cpp \
	$(SRC)/dlgTarget.cpp \
	$(SRC)/dlgTaskCalculator.cpp \
	$(SRC)/dlgTaskOverview.cpp \
	$(SRC)/dlgTaskRules.cpp \
	$(SRC)/dlgTimeGates.cpp \
	$(SRC)/dlgTopology.cpp \
	$(SRC)/dlgTaskWaypoint.cpp \
	$(SRC)/dlgTeamCode.cpp \
	$(SRC)/dlgTextEntry_Keyboard.cpp \
	$(SRC)/dlgThermalDetails.cpp \
	$(SRC)/dlgTools.cpp \
	$(SRC)/dlgWayPointDetails.cpp \
	$(SRC)/dlgWayQuick.cpp \
	$(SRC)/dlgWaypointEdit.cpp \
	$(SRC)/dlgWayPointSelect.cpp \
	$(SRC)/dlgWaypointOutOfTerrain.cpp \
	$(SRC)/dlgWindSettings.cpp \

VOLKS	:=\
	$(SRC)/Volkslogger/dbbconv.cpp \
	$(SRC)/Volkslogger/grecord.cpp \
	$(SRC)/Volkslogger/vlapi2.cpp \
	$(SRC)/Volkslogger/vlapihlp.cpp \
	$(SRC)/Volkslogger/vlapisys_win.cpp \
	$(SRC)/Volkslogger/vlconv.cpp \
	$(SRC)/Volkslogger/vlutils.cpp

SRC_FILES :=\
	$(SRC)/Thread_Draw.cpp		$(SRC)/Thread_Port.cpp\
	$(SRC)/WndProc.cpp		$(SRC)/InitFunctions.cpp\
	$(SRC)/Settings.cpp		$(SRC)/Thread_Calculation.cpp\
	$(SRC)/lk8000.cpp		$(SRC)/ProcessTimer.cpp \
	$(SRC)/TimeFunctions.cpp	$(SRC)/StringFunctions.cpp\
	$(SRC)/Polar.cpp		$(SRC)/AssetId.cpp \
	$(SRC)/FlarmTools.cpp		$(SRC)/PressureFunctions.cpp\
	$(SRC)/MessageLog.cpp		$(SRC)/Registry.cpp\
	$(SRC)/Locking.cpp		$(SRC)/Fonts.cpp \
	$(SRC)/ExpandMacros.cpp		$(SRC)/Battery.cpp \
	$(SRC)/Globals.cpp		$(SRC)/DataOptions.cpp \
	$(SRC)/LKAirspace.cpp		$(SRC)/Bitmaps.cpp \
	$(SRC)/AATDistance.cpp 		$(SRC)/AirfieldDetails.cpp \
	$(SRC)/Airspace.cpp 		\
	$(SRC)/Atmosphere.cpp 		\
	$(SRC)/Calculations.cpp 	$(SRC)/Calculations2.cpp \
	$(SRC)/ClimbAverageCalculator.cpp $(SRC)/LKCalculations.cpp \
	$(SRC)/ConditionMonitor.cpp 	$(SRC)/device.cpp \
	$(SRC)/Dialogs.cpp 		$(SRC)/LKProcess.cpp \
	$(SRC)/FlarmIdFile.cpp 		$(SRC)/FlarmCalculations.cpp \
	$(SRC)/Geoid.cpp \
	$(SRC)/InfoBoxLayout.cpp \
	$(SRC)/InputEvents.cpp 		$(SRC)/leastsqs.cpp \
	$(SRC)/Logger.cpp 		$(SRC)/LKMapWindow.cpp \
	$(SRC)/LKDrawLook8000.cpp 	$(SRC)/LKDrawNearest.cpp\
	$(SRC)/LKDrawCommon.cpp 	$(SRC)/LKDrawAspNearest.cpp\
	$(SRC)/LKDrawInfoPage.cpp	$(SRC)/LKDrawWaypoints.cpp\
	$(SRC)/LKDrawTraffic.cpp	$(SRC)/LKSimulator.cpp\
	$(SRC)/LKDrawThermalHistory.cpp \
	$(SRC)/LKBestAlternate.cpp	$(SRC)/LKHolux.cpp \
	$(SRC)/LKRoyaltek3200.cpp	\
	$(SRC)/MapWindow.cpp 		$(SRC)/MapWindow2.cpp \
	$(SRC)/MapWindow3.cpp 		$(SRC)/MapWindowA.cpp \
	$(SRC)/MapWindowZoom.cpp        $(SRC)/MapWindowMode.cpp \
	$(SRC)/Utils2.cpp \
	$(SRC)/McReady.cpp 		$(SRC)/Message.cpp \
	$(SRC)/NavFunctions.cpp		\
	$(SRC)/Parser.cpp		$(SRC)/Port.cpp \
	$(SRC)/Process.cpp 		$(SRC)/dlgComboPicker.cpp \
	$(SRC)/RasterTerrain.cpp	$(SRC)/rscalc.cpp \
	$(SRC)/StdAfx.cpp		$(SRC)/STScreenBuffer.cpp \
	$(SRC)/Task.cpp			$(SRC)/TeamCodeCalculation.cpp \
	$(SRC)/Terrain.cpp		$(SRC)/ThermalLocator.cpp \
	$(SRC)/Topology.cpp		$(SRC)/units.cpp \
	$(SRC)/Utils.cpp		$(SRC)/Utm.cpp \
	$(SRC)/VOIMAGE.cpp		$(SRC)/LKObjects.cpp \
	$(SRC)/Waypointparser.cpp  	$(SRC)/LKUtils.cpp \
	$(SRC)/LKLanguage.cpp		$(SRC)/LKGeneralAviation.cpp \
	$(SRC)/windanalyser.cpp		$(SRC)/windmeasurementlist.cpp \
	$(SRC)/windstore.cpp 		$(SRC)/WindowControls.cpp \
	$(SRC)/WindZigZag.cpp 		$(SRC)/xmlParser.cpp \
	$(SRC)/ContestMgr.cpp 		$(SRC)/Trace.cpp \
	\
	$(SRC)/mapbits.cpp \
	$(SRC)/maperror.cpp 		$(SRC)/mapprimitive.cpp \
	$(SRC)/mapsearch.cpp		$(SRC)/mapshape.cpp \
	$(SRC)/maptree.cpp              $(SRC)/mapxbase.cpp \
	\
	$(UTILS) \
	$(DEVS) \
	$(DLGS) \
	$(VOLKS)


####### libraries

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
	$(COMPATSRC)/errno.cpp 		$(COMPATSRC)/string_extras.cpp \
	$(COMPATSRC)/ts_string.cpp 	$(COMPATSRC)/wtoi.c

#ifneq ($(CONFIG_PC),y)
#COMPAT	:=$(COMPAT) \
#   $(COMPATSRC)/redir.cpp
#endif


####### compilation outputs

# Add JP2 library for JP2000 unsupported raster maps
# (BIN)/jasper.a \

OBJS 	:=\
	$(patsubst $(SRC)%.cpp,$(BIN)%.o,$(SRC_FILES)) \
	$(BIN)/zzip.a \
	$(BIN)/compat.a \
	$(BIN)/lk8000.rsc

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
	@$(NQ)echo "  CLEAN   $(BIN)"
	$(Q)$(FIND) $(BIN) $(IGNORE) \( -name '*.[oa]' -o -name '*.rsc' -o -name '.*.d' \) -type f -print | xargs -r $(RM)
	$(Q)$(RM) LK8000-$(TARGET)-ns.exe

cleani:
	@$(NQ)echo "  CLEANI"
	$(Q)$(FIND) . $(IGNORE) \( -name '*.i' \) -type f -print | xargs -r $(RM)

tags:
	@$(NQ)echo "  TAGS"
	$(Q)$(ETAGS) --declarations --output=TAGS `find . -name *\\\.[ch] -or -name *\\\.cpp`
	$(Q)$(EBROWSE) -s `find . -name *\\\.[ch] -or -name *\\\.cpp`


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

LK8000-$(TARGET).exe: LK8000-$(TARGET)-ns.exe
	@$(NQ)echo "  STRIP   $@"
	$(Q)$(STRIP) $< -o $@
	$(Q)$(SIZE) $@
#	./buildnumber
	$(RM) LK8000-$(TARGET)-ns.exe

LK8000-$(TARGET)-ns.exe: $(OBJS)
	@$(NQ)echo "  LINK    $@"
	$(Q)$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(BIN)/zzip.a: $(patsubst $(SRC)%.cpp,$(BIN)%.o,$(ZZIP)) $(patsubst $(SRC)%.c,$(BIN)%.o,$(ZZIP))
	@$(NQ)echo "  AR      $@"
	$(Q)$(AR) $(ARFLAGS) $@ $^

$(BIN)/jasper.a: $(patsubst $(SRC)%.cpp,$(BIN)%.o,$(JASPER)) $(patsubst $(SRC)%.c,$(BIN)%.o,$(JASPER))
	@$(NQ)echo "  AR      $@"
	$(Q)$(AR) $(ARFLAGS) $@ $^

$(BIN)/compat.a: $(patsubst $(SRC)%.cpp,$(BIN)%.o,$(COMPAT)) $(patsubst $(SRC)%.c,$(BIN)%.o,$(COMPAT))
	@$(NQ)echo "  AR      $@"
	$(Q)$(AR) $(ARFLAGS) $@ $^

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

$(BIN)/%.rsc: $(SRC)/%.rc
	@echo "$@: $< " `sed -nr 's|^.*"\.\./(Data[^"]+)".*$$|Common/\1|gp' $<` > $(DEPFILE)
	@$(NQ)echo "  WINDRES $@"
	$(Q)$(WINDRES) $(WINDRESFLAGS) $< $@



####### include depends files

ifneq ($(wildcard $(BIN)/.*.d),)
include $(wildcard $(BIN)/.*.d)
endif
ifneq ($(wildcard $(BIN)/*/.*.d),)
include $(wildcard $(BIN)/*/.*.d)
endif
ifneq ($(wildcard $(BIN)/.*.rsc),)
include $(wildcard $(BIN)/.*.d)
endif
