#
SRC=Common/Source
DEV=Common/Source/Devices
DLG=Common/Source/Dialogs
LIB=Common/Source/Library
DRW=Common/Source/Draw
MAP=Common/Source/MapDraw
TOP=Common/Source/Topology
SHP=Common/Source/Topology/shapelib
TER=Common/Source/Terrain
NTR=Common/Source/LKInterface
CLC=Common/Source/Calc
TSK=Common/Source/Calc/Task
HDR=Common/Header

BIN=Bin/$(TARGET)

# enable/disable heap checking (dmalloc.h libdmalloc.a must be in ../dmalloc)
DMALLOC=n

ifeq ($(DEBUG),y)
OPTIMIZE := -O0
OPTIMIZE += -g3 -gdwarf-2
else
OPTIMIZE := -O2
endif

PROFILE		:=
#OPTIMIZE	:=-O2
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
#
# LX MINIMAP CUSTOM VERSION
#
#CPPFLAGS	+= -DLXMINIMAP
#
#
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
CPPFLAGS	+= -D_WINDOWS -D_MBCS -DWIN32 -DCECORE $(UNICODE)
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
  LDLIBS := -Wl,-Bstatic -lstdc++  -lmingw32 -lcomctl32 -lkernel32 -luser32 -lgdi32 -ladvapi32 -lwinmm -lmsimg32 -lwsock32
else
  LDLIBS := -Wl,-Bstatic -lstdc++  -Wl,-Bdynamic -lcommctrl -lwinsock
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

LIBRARY	:=\
	$(LIB)/bsearch.cpp \
	$(LIB)/Crc.cpp\
	$(LIB)/DirectoryFunctions.cpp \
	$(LIB)/DrawFunctions.cpp \
	$(LIB)/leastsqs.cpp \
	$(LIB)/magfield.cpp \
	$(LIB)/MathFunctions.cpp	\
	$(LIB)/NavFunctions.cpp	\
	$(LIB)/PressureFunctions.cpp\
	$(LIB)/rscalc.cpp \
	$(LIB)/StringFunctions.cpp\
	$(LIB)/TimeFunctions.cpp\
	$(LIB)/Utm.cpp \
	$(LIB)/xmlParser.cpp \

LKINTER	:=\
	$(NTR)/LKCustomKeyHandler.cpp\
	$(NTR)/LKInit.cpp\
	$(NTR)/LKInitScreen.cpp\
	$(NTR)/LKInterface.cpp \
	$(NTR)/OverTargets.cpp\
	$(NTR)/VirtualKeys.cpp\

DRAW	:=\
	$(DRW)/CalculateScreen.cpp \
	$(DRW)/CalculateWaypointReachable.cpp \
	$(DRW)/DrawAircraft.cpp \
	$(DRW)/DrawAirSpaces.cpp \
	$(DRW)/DrawAirspaceLabels.cpp \
	$(DRW)/DrawBearing.cpp \
	$(DRW)/DrawBestCruiseTrack.cpp \
	$(DRW)/DrawCompass.cpp \
	$(DRW)/DrawCross.cpp \
	$(DRW)/DrawFinalGlideBar.cpp \
	$(DRW)/DrawFlarmRadar.cpp \
	$(DRW)/DrawFlightMode.cpp \
	$(DRW)/DrawGlideThroughTerrain.cpp \
	$(DRW)/DrawGPSStatus.cpp \
	$(DRW)/DrawGreatCircle.cpp \
	$(DRW)/DrawHeading.cpp \
	$(DRW)/DrawLKAlarms.cpp \
	$(DRW)/DrawMapScale.cpp \
	$(DRW)/DrawMultimap_Asp.cpp \
	$(DRW)/DrawMultimap_Radar.cpp \
	$(DRW)/DrawMultimap_Test.cpp \
	$(DRW)/DrawRunway.cpp \
	$(DRW)/DrawStartSector.cpp \
	$(DRW)/DrawTRI.cpp \
	$(DRW)/DrawTask.cpp \
	$(DRW)/DrawTaskAAT.cpp \
	$(DRW)/DrawTeamMate.cpp \
	$(DRW)/DrawTerrainAbove.cpp \
	$(DRW)/DrawThermalBand.cpp \
	$(DRW)/DrawThermalEstimate.cpp \
	$(DRW)/DrawVisualGlide.cpp \
	$(DRW)/DrawWind.cpp \
	$(DRW)/Draw_Primitives.cpp \
	$(DRW)/LKDrawAspNearest.cpp \
	$(DRW)/LKDrawCommon.cpp \
	$(DRW)/LKDrawCpuStatsDebug.cpp \
	$(DRW)/LKDrawFLARMTraffic.cpp \
	$(DRW)/LKDrawInfoPage.cpp \
	$(DRW)/LKDrawLook8000.cpp \
	$(DRW)/LKDrawMapSpace.cpp \
	$(DRW)/LKDrawNearest.cpp \
	$(DRW)/LKDrawTargetTraffic.cpp \
	$(DRW)/LKDrawThermalHistory.cpp \
	$(DRW)/LKDrawTrail.cpp \
	$(DRW)/LKDrawTraffic.cpp \
	$(DRW)/LKDrawVario.cpp \
	$(DRW)/LKDrawWaypoints.cpp \
	$(DRW)/LKDrawWelcome.cpp \
	$(DRW)/LKGeneralAviation.cpp \
	$(DRW)/LKMessages.cpp \
	$(DRW)/LKWriteText.cpp \
	$(DRW)/LoadSplash.cpp\
	$(DRW)/MapScale.cpp \
	$(DRW)/MapWindowA.cpp \
	$(DRW)/MapWindowMode.cpp \
	$(DRW)/MapWindowZoom.cpp \
	$(DRW)/MapWindow_Events.cpp \
	$(DRW)/MapWindow_Utils.cpp \
	$(DRW)/MapWndProc.cpp \
	$(DRW)/OrigAndOrient.cpp \
	$(DRW)/RenderAirspace.cpp\
	$(DRW)/RenderAirspaceTerrain.cpp\
	$(DRW)/RenderMapWindow.cpp \
	$(DRW)/RenderMapWindowBg.cpp \
	$(DRW)/RenderNearAirspace.cpp\
	$(DRW)/ScreenLatLon.cpp \
	$(DRW)/Sideview.cpp \
	$(DRW)/TextInBox.cpp \
	$(DRW)/UpdateAndRefresh.cpp \

CALC	:=\
	$(CLC)/AddSnailPoint.cpp 		\
	$(CLC)/AltitudeRequired.cpp \
	$(CLC)/Atmosphere.cpp 		\
	$(CLC)/AutoMC.cpp \
	$(CLC)/AutoQNH.cpp \
	$(CLC)/AverageClimbRate.cpp \
	$(CLC)/Azimuth.cpp \
	$(CLC)/BallastDump.cpp \
	$(CLC)/BestAlternate.cpp	\
	$(CLC)/Calculations2.cpp \
	$(CLC)/Calculations_Utils.cpp \
	$(CLC)/ClimbAverageCalculator.cpp\
	$(CLC)/ClimbStats.cpp\
	$(CLC)/ContestMgr.cpp\
	$(CLC)/DistanceToHome.cpp\
	$(CLC)/DistanceToNext.cpp\
	$(CLC)/DoAirspaces.cpp \
	$(CLC)/DoAlternates.cpp \
	$(CLC)/DoCalculations.cpp \
	$(CLC)/DoCalculationsSlow.cpp \
	$(CLC)/DoCalculationsVario.cpp \
	$(CLC)/DoCommon.cpp \
	$(CLC)/DoLogging.cpp \
	$(CLC)/DoNearest.cpp \
	$(CLC)/DoRangeWaypointList.cpp \
	$(CLC)/DoRecent.cpp \
	$(CLC)/DoTarget.cpp \
	$(CLC)/DoTraffic.cpp \
	$(CLC)/FarFinalGlideThroughTerrain.cpp\
	$(CLC)/FinalGlideThroughTerrain.cpp\
	$(CLC)/Flaps.cpp \
	$(CLC)/FlarmCalculations.cpp \
	$(CLC)/FlightTime.cpp\
	$(CLC)/FreeFlight.cpp \
	$(CLC)/GlideThroughTerrain.cpp \
	$(CLC)/Heading.cpp \
	$(CLC)/HeadWind.cpp \
	$(CLC)/InitCloseCalculations.cpp \
	$(CLC)/LastThermalStats.cpp\
	$(CLC)/LD.cpp\
	$(CLC)/LDRotaryBuffer.cpp\
	$(CLC)/MagneticVariation.cpp \
	$(CLC)/McReady.cpp\
	$(CLC)/NettoVario.cpp\
	$(CLC)/Orbiter.cpp \
	$(CLC)/Pirker.cpp \
	$(CLC)/PredictNextPosition.cpp \
	$(CLC)/ResetFlightStats.cpp\
	$(CLC)/SetWindEstimate.cpp \
	$(CLC)/SpeedToFly.cpp \
	$(CLC)/TakeoffLanding.cpp\
	$(CLC)/TeamCodeCalculation.cpp \
	$(CLC)/TerrainFootprint.cpp \
	$(CLC)/TerrainHeight.cpp \
	$(CLC)/ThermalBand.cpp \
	$(CLC)/ThermalHistory.cpp \
	$(CLC)/ThermalLocator.cpp \
	$(CLC)/TotalEnergy.cpp\
	$(CLC)/Trace.cpp \
	$(CLC)/Turning.cpp \
	$(CLC)/Valid.cpp\
	$(CLC)/Vario.cpp\
	$(CLC)/WaypointApproxDistance.cpp \
	$(CLC)/WaypointArrivalAltitude.cpp \
	$(CLC)/windanalyser.cpp\
	$(CLC)/windmeasurementlist.cpp \
	$(CLC)/windstore.cpp 	\
	$(CLC)/WindZigZag.cpp 	\

TASK	:=\
	$(TSK)/AATCalculateIsoLines.cpp \
	$(TSK)/AATDistance.cpp \
	$(TSK)/AATInTurnSector.cpp	\
	$(TSK)/AATStats.cpp 		\
	$(TSK)/AATtools.cpp 		\
	$(TSK)/AnnounceWPSwitch.cpp 	\
	$(TSK)/CheckFinalGlide.cpp \
	$(TSK)/CheckInSector.cpp \
	$(TSK)/CheckStartRestartFinish.cpp \
	$(TSK)/FAIFinishHeight.cpp \
	$(TSK)/FlyDirectTo.cpp \
	$(TSK)/InFinishSector.cpp \
	$(TSK)/InSector.cpp \
	$(TSK)/InStartSector.cpp \
	$(TSK)/InTurnSector.cpp \
	$(TSK)/InsideStartHeight.cpp\
	$(TSK)/OptimizedTargetPos.cpp \
	$(TSK)/ReadyToStartAdvance.cpp \
	$(TSK)/RefreshTaskStatistics.cpp \
	$(TSK)/SpeedHeight.cpp\
	$(TSK)/StartTask.cpp \
	$(TSK)/TaskAltitudeRequired.cpp\
	$(TSK)/TaskSpeed.cpp\
	$(TSK)/TaskStatistic.cpp\
	$(TSK)/TaskUtils.cpp\
	$(TSK)/TimeGates.cpp\
	$(TSK)/RefreshTask/CalculateAATTaskSectors.cpp\
	$(TSK)/RefreshTask/CalculateTaskSectors.cpp\
	$(TSK)/RefreshTask/RefreshTask.cpp\

TERRAIN	:=\
	$(TER)/Cache.cpp	\
	$(TER)/JP2.cpp	\
	$(TER)/OpenCreateClose.cpp	\
	$(TER)/RasterTerrain.cpp	\
	$(TER)/RAW.cpp	\
	$(TER)/STScreenBuffer.cpp \

TOPOL	:=\
	$(TOP)/Topology.cpp		\

MAPDRAW	:=\
	$(MAP)/DrawTerrain.cpp		\
	$(MAP)/DrawTopology.cpp		\
	$(MAP)/MarkLocation.cpp		\
	$(MAP)/OpenCloseTopology.cpp		\
	$(MAP)/SetTopologyBounds.cpp		\
	$(MAP)/TopoMarks.cpp		\
	$(MAP)/ZoomTopology.cpp		\

UTILS	:=\
	$(SRC)/utils/fileext.cpp \
	$(SRC)/utils/stringext.cpp \
	$(SRC)/utils/md5internal.cpp \
	$(SRC)/utils/md5.cpp


DEVS	:=\
	$(DEV)/devBase.cpp \
	$(DEV)/devBorgeltB50.cpp \
	$(DEV)/devCAI302.cpp \
	$(DEV)/devCaiGpsNav.cpp \
	$(DEV)/devCompeo.cpp \
	$(DEV)/devCondor.cpp \
	$(DEV)/devDigifly.cpp \
	$(DEV)/devDisabled.cpp \
	$(DEV)/devDSX.cpp \
	$(DEV)/devEW.cpp \
	$(DEV)/devEWMicroRecorder.cpp \
	$(DEV)/devFlymasterF1.cpp \
	$(DEV)/devFlytec.cpp \
	$(DEV)/devGeneric.cpp \
	$(DEV)/devIlec.cpp \
	$(DEV)/devIMI.cpp \
	$(DEV)/devNmeaOut.cpp \
	$(DEV)/devLKext1.cpp \
	$(DEV)/devLX.cpp \
	$(DEV)/devLX16xx.cpp \
	$(DEV)/devLXMiniMap.cpp \
	$(DEV)/devLXNano.cpp \
	$(DEV)/devLXV7.cpp \
	$(DEV)/devPosiGraph.cpp \
	$(DEV)/devVolkslogger.cpp \
	$(DEV)/devXCOM760.cpp \
	$(DEV)/devZander.cpp \
	$(DEV)/devWesterboer.cpp \
	$(DEV)/LKHolux.cpp \
	$(DEV)/LKRoyaltek3200.cpp	\
	$(DEV)/devFlyNet.cpp \
	$(DEV)/devCProbe.cpp 

VOLKS	:=\
	$(DEV)/Volkslogger/dbbconv.cpp \
	$(DEV)/Volkslogger/grecord.cpp \
	$(DEV)/Volkslogger/vlapi2.cpp \
	$(DEV)/Volkslogger/vlapihlp.cpp \
	$(DEV)/Volkslogger/vlapisys_win.cpp \
	$(DEV)/Volkslogger/vlconv.cpp \
	$(DEV)/Volkslogger/vlutils.cpp


DLGS	:=\
	$(DLG)/AddCustomKeyList.cpp \
	$(DLG)/dlgAirspace.cpp \
	$(DLG)/dlgAirspaceWarningParams.cpp \
	$(DLG)/dlgAirspaceColours.cpp \
	$(DLG)/dlgAirspaceDetails.cpp \
	$(DLG)/dlgAirspacePatterns.cpp \
	$(DLG)/dlgAirspaceSelect.cpp \
	$(DLG)/dlgBasicSettings.cpp \
	$(DLG)/dlgBottomBar.cpp \
	$(DLG)/dlgChecklist.cpp \
	$(DLG)/dlgComboPicker.cpp \
	$(DLG)/dlgConfiguration.cpp \
	$(DLG)/dlgConfiguration2.cpp \
	$(DLG)/dlgCustomKeys.cpp \
	$(DLG)/dlgCustomMenu.cpp \
	$(DLG)/dlgFontEdit.cpp \
	$(DLG)/dlgHelp.cpp \
	$(DLG)/dlgInfoPages.cpp \
	$(DLG)/dlgLKAirspaceWarning.cpp \
	$(DLG)/dlgLKTraffic.cpp \
	$(DLG)/dlgLoggerReplay.cpp \
	$(DLG)/dlgOracle.cpp \
	$(DLG)/dlgProfiles.cpp \
	$(DLG)/dlgStartPoint.cpp \
	$(DLG)/dlgStartTask.cpp \
	$(DLG)/dlgStartup.cpp \
	$(DLG)/dlgStatistics.cpp \
	$(DLG)/dlgStatus.cpp \
	$(DLG)/dlgTarget.cpp \
	$(DLG)/dlgTaskCalculator.cpp \
	$(DLG)/dlgTaskOverview.cpp \
	$(DLG)/dlgTaskRules.cpp \
	$(DLG)/dlgTimeGates.cpp \
	$(DLG)/dlgTopology.cpp \
	$(DLG)/dlgTaskWaypoint.cpp \
	$(DLG)/dlgTeamCode.cpp \
	$(DLG)/dlgTextEntry_Keyboard.cpp \
	$(DLG)/dlgThermalDetails.cpp \
	$(DLG)/dlgTools.cpp \
	$(DLG)/dlgWayPointDetails.cpp \
	$(DLG)/dlgWayQuick.cpp \
	$(DLG)/dlgWaypointEdit.cpp \
	$(DLG)/dlgWayPointSelect.cpp \
	$(DLG)/dlgWaypointOutOfTerrain.cpp \
	$(DLG)/dlgWindSettings.cpp \
	$(DLG)/Task/AdjustAATTargets.cpp\
	$(DLG)/Task/InsertWaypoint.cpp\
	$(DLG)/Task/LoadTaskWaypoints.cpp\
	$(DLG)/Task/RemoveTaskPoint.cpp\
	$(DLG)/Task/RemoveWaypoint.cpp\
	$(DLG)/Task/ReplaceWaypoint.cpp\
	$(DLG)/Task/RotateStartPoints.cpp\
	$(DLG)/Task/SwapWaypoint.cpp\

SRC_FILES :=\
	$(SRC)/AirfieldDetails.cpp \
	$(SRC)/Alarms.cpp\
	$(SRC)/Backlight.cpp 		\
	$(SRC)/Battery.cpp \
	$(SRC)/Bitmaps.cpp \
	$(SRC)/Buttons.cpp \
	$(SRC)/ChangeScreen.cpp\
	$(SRC)/CommandLine.cpp \
	$(SRC)/ConditionMonitor.cpp \
	$(SRC)/CpuLoad.cpp \
	$(SRC)/DataOptions.cpp \
	$(SRC)/device.cpp \
	$(SRC)/Dialogs.cpp\
	$(SRC)/DLL.cpp \
	$(SRC)/DoInits.cpp\
	$(SRC)/ExpandMacros.cpp	\
	$(SRC)/FlarmIdFile.cpp 		\
	$(SRC)/FlarmTools.cpp		\
	$(SRC)/Fonts.cpp \
	$(SRC)/Geoid.cpp \
	$(SRC)/Globals.cpp	\
	$(SRC)/InitFunctions.cpp\
	$(SRC)/InputEvents.cpp 		\
	$(SRC)/lk8000.cpp\
	$(SRC)/LiveTracker.cpp \
	$(SRC)/LKAirspace.cpp	\
	$(SRC)/LKFonts.cpp		\
	$(SRC)/LKInstall.cpp 		\
	$(SRC)/LKLanguage.cpp		\
	$(SRC)/LKObjects.cpp \
	$(SRC)/LKProcess.cpp \
	$(SRC)/LKProfileInitRuntime.cpp\
	$(SRC)/LKProfileLoad.cpp\
	$(SRC)/LKProfileResetDefault.cpp\
	$(SRC)/LKProfileSave.cpp\
	$(SRC)/LKSimulator.cpp\
	$(SRC)/LKUtils.cpp \
	$(SRC)/LocalPath.cpp\
	$(SRC)/Locking.cpp\
	$(SRC)/Logger/DoSignature.cpp 	\
	$(SRC)/Logger/FlightDataRec.cpp 	\
	$(SRC)/Logger/LogBook.cpp\
	$(SRC)/Logger/Logger.cpp \
	$(SRC)/Logger/StartStopLogger.cpp \
	$(SHP)/mapbits.cpp \
	$(SHP)/maperror.cpp 	\
	$(SHP)/mapprimitive.cpp \
	$(SHP)/mapsearch.cpp\
	$(SHP)/mapshape.cpp \
	$(SHP)/maptree.cpp\
	$(SHP)/mapxbase.cpp \
	$(SRC)/Memory.cpp \
	$(SRC)/Message.cpp \
	$(SRC)/MessageLog.cpp	\
	$(SRC)/Models.cpp\
	$(SRC)/Oracle.cpp\
	$(SRC)/Parser.cpp\
	$(SRC)/Polar.cpp		\
	$(SRC)/Port.cpp \
	$(SRC)/ProcessTimer.cpp \
	$(SRC)/Progress.cpp\
	$(SRC)/RotateScreen.cpp\
	$(SRC)/SaveLoadTask/ClearTask.cpp\
	$(SRC)/SaveLoadTask/DefaultTask.cpp\
	$(SRC)/SaveLoadTask/LoadNewTask.cpp\
	$(SRC)/SaveLoadTask/SaveDefaultTask.cpp\
	$(SRC)/SaveLoadTask/SaveTask.cpp\
	$(SRC)/Settings.cpp\
	$(SRC)/Sound.cpp \
	$(SRC)/StatusFile.cpp \
	$(SRC)/Thread_Calculation.cpp\
	$(SRC)/Thread_Draw.cpp	\
	$(SRC)/Thread_Port.cpp\
	$(SRC)/TrueWind.cpp		\
	$(SRC)/units.cpp \
	$(SRC)/Utils.cpp		\
	$(SRC)/Waypointparser.cpp \
	$(SRC)/WndProc.cpp\
	$(SRC)/WindowControls.cpp \
	\
	$(LKINTER) \
	$(LIBRARY) \
	$(DRAW) \
	$(CALC) \
	$(TASK) \
	$(TERRAIN) \
	$(TOPOL) \
	$(MAPDRAW) \
	$(UTILS) \
	$(DEVS) \
	$(DLGS) \
	$(VOLKS)


####### libraries

ZZIPSRC	:=$(LIB)/zzip
ZZIP	:=\
	$(ZZIPSRC)/adler32.c	 	\
	$(ZZIPSRC)/crc32.c 		\
	$(ZZIPSRC)/err.c 		$(ZZIPSRC)/fetch.c \
	$(ZZIPSRC)/file.c 		\
	$(ZZIPSRC)/infback.c 		$(ZZIPSRC)/inffast.c \
	$(ZZIPSRC)/inflate.c 		$(ZZIPSRC)/info.c \
	$(ZZIPSRC)/inftrees.c 		$(ZZIPSRC)/plugin.c \
	$(ZZIPSRC)/uncompr.c \
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
ifeq ($(DEBUG),y)
else
	$(RM) LK8000-$(TARGET)-ns.exe
endif
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
