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
CMM=Common/Source/Comm
WPT=Common/Source/Waypoints
RSC=Common/Source/Resources
HDR=Common/Header
SRC_SCREEN=$(SRC)/Screen
SRC_WINDOW=$(SRC)/Window


BIN=Bin/$(TARGET)

# enable/disable heap checking (dmalloc.h libdmalloc.a must be in ../dmalloc)
DMALLOC=n

OPTIMIZE    := -O2
PROFILE	    :=
REMOVE_NS   := y

ifeq ($(DEBUG),y)
    OPTIMIZE := -O0
    OPTIMIZE += -g3 -gdwarf-2
    REMOVE_NS :=
    BIN=Bin/$(TARGET)_debug
endif

ifeq ($(GPROF),y)
    PROFILE		:= -pg
    REMOVE_NS :=
endif

CONFIG_PPC2002	:=n
CONFIG_PPC2003	:=n
CONFIG_PC	:=n
CONFIG_WINE	:=n
CONFIG_PNA	:=n
CONFIG_LINUX	:=n
CONFIG_ANDROID	:=n
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
	  else
	    ifeq ($(TARGET),LINUX)
	      CONFIG_LINUX := y
	      CONFIG_ANDROID := n
	      MINIMAL       :=n
		else
		    ifeq ($(TARGET),PCX64)
			CONFIG_PC:=y
		    endif
        endif
	  endif
	endif
      endif
    endif
  endif
endif

############# build and CPU info

ifeq ($(CONFIG_PC),y)
ifeq ($(TARGET),PCX64)
TCPATH		:=x86_64-w64-mingw32-
CPU		:=
MCPU		:=
else
TCPATH		:=i686-w64-mingw32-
CPU		:=i586
MCPU		:= -mcpu=$(CPU)
endif
else
ifeq ($(CONFIG_WINE),y)
TCPATH		:=wine
CPU		:=i586
MCPU		:= -mcpu=$(CPU)
else
ifeq ($(CONFIG_LINUX),y)
TCPATH		:= 
else
TCPATH		:=arm-mingw32ce-
endif

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

-include local.mk

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

ifeq ($(DEBUG),y)
OUTPUTS 	:= LK8000-$(TARGET)_debug.exe
OUTPUTS_NS	:= LK8000-$(TARGET)_debug_ns.exe	
	
else
OUTPUTS 	:= LK8000-$(TARGET).exe
OUTPUTS_NS	:= LK8000-$(TARGET)_ns.exe	
endif

######## tools

EXE		:=$(findstring .exe,$(MAKE))
AR		:=$(TCPATH)ar$(EXE)
AS		:=$(TCPATH)as$(EXE)
CXX		:=$(TCPATH)g++$(EXE)
CC		:=$(TCPATH)gcc$(EXE)
SIZE		:=$(TCPATH)size$(EXE)
STRIP		:=$(TCPATH)strip$(EXE)
WINDRES		:=$(TCPATH)windres$(EXE)
LD		:=$(TCPATH)ld$(EXE)
OBJCOPY		:=$(TCPATH)objcopy$(EXE)
SYNCE_PCP	:=synce-pcp
SYNCE_PRM	:=synce-prm
CE_VERSION	:=0x0$(CE_MAJOR)$(CE_MINOR)
ARFLAGS		:=r
MKDIR           :=mkdir -p
FIND            :=find
ETAGS           :=etags
EBROWSE         :=ebrowse

GCCVERSION = $(shell $(CXX) --version | grep ^$(TCPATH) | sed 's/^.* //g')

######## windows definitions

ifeq ($(CONFIG_LINUX),y)
CE_DEFS		:=-D__linux__
else
ifeq ($(CONFIG_PC),y)
CE_DEFS		:=-D_WIN32_WINDOWS=$(CE_VERSION) -DWINVER=$(CE_VERSION)
CE_DEFS		+=-D_WIN32_IE=$(CE_VERSION) -DWINDOWSPC=1 -DMSOFT
else
CE_DEFS		:=-D_WIN32_WCE=$(CE_VERSION) -D_WIN32_IE=$(CE_VERSION)
CE_DEFS		+=-DWIN32_PLATFORM_PSPC=$(CE_PLATFORM) -DMSOFT
# UNIX like ressource work on all plarform, so no need.
#WIN32_RESSOURCE := y 
endif
endif

ifeq ($(WIN32_RESSOURCE), y)
CE_DEFS		+=-DWIN32_RESOURCE
endif

ifeq ($(CONFIG_PPC2002),y)
CE_DEFS		+=-DPPC2002=1
endif
ifeq ($(CONFIG_PPC2003),y)
CE_DEFS		+=-DPPC2003=1
endif

CE_DEFS		+= -DPOCO_NO_UNWINDOWS


######## paths

ifeq ($(CONFIG_LINUX),y)
INCLUDES	:= -I$(HDR)/linuxcompat -I$(HDR) -I$(SRC)
else
UNICODE		:= -DUNICODE -D_UNICODE
ifeq ($(CONFIG_WINE),y)
INCLUDES	:= -I$(HDR)/mingw32compat -I$(HDR) -I$(SRC)
else
INCLUDES	:= -I$(HDR)/mingw32compat -I$(HDR) -I$(SRC)
endif
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

CPPFLAGS	+= -Wall -Wno-char-subscripts
#CPPFLAGS	+= -Wall -Wno-char-subscripts -Wignored-qualifiers -Wunsafe-loop-optimizations 
#CPPFLAGS	+= -Winit-self -Wswitch -Wcast-qual -Wcast-align
#CPPFLAGS	+= -Wall -Wno-non-virtual-dtor
#CPPFLAGS	+= -Wno-char-subscripts -Wno-switch

#CPPFLAGS	+= -Wshadow
#CPPFLAGS	+= -Wsign-compare -Wsign-conversion
ifeq ($(CONFIG_PNA),y)
CPPFLAGS	+= -DCECORE -DPNA
endif

ifeq ($(CONFIG_LINUX),y)
CPPFLAGS	+= $(UNICODE)
else
ifeq ($(CONFIG_PC),y)
CPPFLAGS	+= -D_WINDOWS -DWIN32 -DCECORE $(UNICODE)

ifeq ($(GCCVERSION), 4.8.3)
    CPPFLAGS	+= -D_CRT_NON_CONFORMING_SWPRINTFS
endif

 ifeq ($(CONFIG_WINE),y)
CPPFLAGS	+= -D__MINGW32__
# -mno-cygwin
  else
CPPFLAGS	+= $(UNICODE)
  endif
else
CPPFLAGS	+= -D_ARM_ $(UNICODE)
endif
endif

ifeq ($(DMALLOC),y)
  CPPFLAGS += -DHC_DMALLOC
endif

CPPFLAGS += -DPOCO_STATIC

ifeq ($(INT_OVERFLOW), y)
	CPPFLAGS	+=-ftrapv -DINT_OVERFLOW
endif

CXXFLAGS	:= -std=gnu++0x $(OPTIMIZE) $(PROFILE)
CFLAGS		:= $(OPTIMIZE) $(PROFILE)

####### linker configuration

ifeq ($(CONFIG_LINUX),y)
LDFLAGS =
else
LDFLAGS		:=-Wl,--major-subsystem-version=$(CE_MAJOR)
LDFLAGS		+=-Wl,--minor-subsystem-version=$(CE_MINOR)
ifeq ($(CONFIG_PC),y)
LDFLAGS		+=-Wl,-subsystem,windows
endif
endif
LDFLAGS		+=$(PROFILE) -Wl,-Map=output.map

ifeq ($(CONFIG_LINUX),y)
  LDLIBS		+= -lstdc++ -lzzip -pthread -march=native
else
ifeq ($(CONFIG_PC),y)
  LDLIBS := -Wl,-Bstatic -lstdc++  -lmingw32 -lcomctl32 -lkernel32 -luser32 -lgdi32 -ladvapi32 -lwinmm -lmsimg32 -lwsock32 -lole32 -loleaut32 -luuid
else
  LDLIBS := -Wl,-Bstatic -lstdc++  -Wl,-Bdynamic -lcommctrl -lole32 -loleaut32 -luuid
  ifeq ($(CONFIG_PPC2002), y)
    LDLIBS		+= -lwinsock
  else
    LDLIBS		+= -lws2
  endif
  ifeq ($(MINIMAL),n)
    LDLIBS		+= -laygshell 
    ifneq ($(TARGET),PNA)
      LDLIBS		+= -limgdecmp 
    endif
  endif
endif
endif



ifeq ($(DMALLOC),y)
  LDLIBS += -L../dmalloc -ldmalloc
endif

####### compiler target

ifeq ($(CONFIG_PC),y)
ifeq ($(TARGET),PCX64)
    TARGET_ARCH := -m64
else
    TARGET_ARCH	:=-mwindows -march=i586 -mms-bitfields
endif
else
TARGET_ARCH	:=-mwin32 $(MCPU)
ifeq ($(TARGET),PNA)
TARGET_ARCH	:=-mwin32
endif
ifeq ($(TARGET),LINUX)
TARGET_ARCH	:=
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
WINDOW := \
	$(SRC_WINDOW)/WindowBase.cpp \
	$(SRC_WINDOW)/WndMain.cpp \
	$(SRC_WINDOW)/Win32/Window.cpp \
	$(SRC_WINDOW)/Win32/WndMainBase.cpp\
	$(SRC_WINDOW)/Win32/WndProc.cpp\

SCREEN := \
	$(SRC_SCREEN)/LKColor.cpp \
	$(SRC_SCREEN)/LKPen.cpp \
	$(SRC_SCREEN)/LKBitmap.cpp \
	$(SRC_SCREEN)/LKBrush.cpp \
	$(SRC_SCREEN)/LKFont.cpp \
	$(SRC_SCREEN)/LKSurface.cpp \
	$(SRC_SCREEN)/LKWindowSurface.cpp \
	$(SRC_SCREEN)/LKBitmapSurface.cpp \

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


WAYPT	:=\
	$(WPT)/AllocateWaypointList.cpp\
	$(WPT)/AltitudeFromTerrain.cpp\
	$(WPT)/CUPToLatLon.cpp\
	$(WPT)/Close.cpp\
	$(WPT)/FindMatchingWaypoint.cpp\
	$(WPT)/FindNearestFarVisible.cpp\
	$(WPT)/FindNearestWayPoint.cpp\
	$(WPT)/InTerrainRange.cpp\
	$(WPT)/InitWayPointCalc.cpp\
	$(WPT)/ParseCOMPE.cpp\
	$(WPT)/ParseCUP.cpp\
	$(WPT)/ParseDAT.cpp\
	$(WPT)/ParseOZI.cpp\
	$(WPT)/Read.cpp\
	$(WPT)/ReadAltitude.cpp\
	$(WPT)/ReadFile.cpp\
	$(WPT)/SetHome.cpp\
	$(WPT)/ToString.cpp\
	$(WPT)/Virtuals.cpp\
	$(WPT)/Write.cpp\


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
	$(DRW)/DoAirspaces.cpp \
	$(DRW)/DoTarget.cpp \
	$(DRW)/DoTraffic.cpp \
	$(DRW)/DrawAircraft.cpp \
	$(DRW)/DrawAirSpaces.cpp \
	$(DRW)/DrawAirSpacesBorders.cpp \
	$(DRW)/DrawAirspaceLabels.cpp \
	$(DRW)/DrawBearing.cpp \
	$(DRW)/DrawBestCruiseTrack.cpp \
	$(DRW)/DrawCompass.cpp \
	$(DRW)/DrawCross.cpp \
	$(DRW)/DrawFAIOpti.cpp \
	$(DRW)/DrawFinalGlideBar.cpp \
	$(DRW)/DrawFlarmRadar.cpp \
	$(DRW)/DrawFlightMode.cpp \
	$(DRW)/DrawFuturePos.cpp \
	$(DRW)/DrawGlideThroughTerrain.cpp \
	$(DRW)/DrawGPSStatus.cpp \
	$(DRW)/DrawGreatCircle.cpp \
	$(DRW)/DrawHeading.cpp \
	$(DRW)/DrawHSI.cpp \
	$(DRW)/DrawLKAlarms.cpp \
	$(DRW)/DrawMapScale.cpp \
	$(DRW)/DrawRunway.cpp \
	$(DRW)/DrawStartSector.cpp \
	$(DRW)/DrawTRI.cpp \
	$(DRW)/DrawTask.cpp \
	$(DRW)/DrawTaskAAT.cpp \
	$(DRW)/DrawTeamMate.cpp \
	$(DRW)/DrawTerrainAbove.cpp \
	$(DRW)/DrawThermalBand.cpp \
	$(DRW)/DrawThermalEstimate.cpp \
	$(DRW)/DrawWind.cpp \
	$(DRW)/Draw_Primitives.cpp \
	$(DRW)/LKDrawAspNearest.cpp \
	$(DRW)/LKDrawBottomBar.cpp \
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
	$(DRW)/LKProcess.cpp \
	$(DRW)/LKWriteText.cpp \
	$(DRW)/LoadSplash.cpp\
	$(DRW)/MapScale.cpp \
	$(DRW)/MapWindowA.cpp \
	$(DRW)/MapWindowMode.cpp \
	$(DRW)/MapWindowZoom.cpp \
	$(DRW)/MapWindow_Events.cpp \
	$(DRW)/MapWindow_Utils.cpp \
	$(DRW)/MapWndProc.cpp \
	$(DRW)/Multimaps/DrawMultimap.cpp \
	$(DRW)/Multimaps/DrawMultimap_Asp.cpp \
	$(DRW)/Multimaps/DrawMultimap_Radar.cpp \
	$(DRW)/Multimaps/DrawMultimap_Test.cpp \
	$(DRW)/Multimaps/GetVisualGlidePoints.cpp \
	$(DRW)/Multimaps/RenderAirspace.cpp\
	$(DRW)/Multimaps/RenderAirspaceTerrain.cpp\
	$(DRW)/Multimaps/RenderNearAirspace.cpp\
	$(DRW)/Multimaps/RenderPlane.cpp\
	$(DRW)/Multimaps/Sideview.cpp \
	$(DRW)/Multimaps/Sky.cpp \
	$(DRW)/Multimaps/TopView.cpp \
	$(DRW)/Multimaps/DrawVisualGlide.cpp \
	$(DRW)/OrigAndOrient.cpp \
	$(DRW)/RenderMapWindow.cpp \
	$(DRW)/RenderMapWindowBg.cpp \
	$(DRW)/ScreenLatLon.cpp \
	$(DRW)/Sonar.cpp \
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
	$(CLC)/DoAlternates.cpp \
	$(CLC)/DoCalculations.cpp \
	$(CLC)/DoCalculationsSlow.cpp \
	$(CLC)/DoCalculationsVario.cpp \
	$(CLC)/DoCommon.cpp \
	$(CLC)/DoLogging.cpp \
	$(CLC)/DoNearest.cpp \
	$(CLC)/DoRangeWaypointList.cpp \
	$(CLC)/DoRecent.cpp \
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
	$(CLC)/WindEKF.cpp 	\
	$(CLC)/WindKalman.cpp 	\


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
	$(TSK)/PGTask/PGTaskPt.cpp\
	$(TSK)/PGTask/PGCicrcleTaskPt.cpp\
	$(TSK)/PGTask/PGLineTaskPt.cpp\
	$(TSK)/PGTask/PGTaskMgr.cpp\
	$(TSK)/PGTask/PGSectorTaskPt.cpp\
	$(TSK)/PGTask/PGConeTaskPt.cpp\

TERRAIN	:=\
	$(TER)/Cache.cpp	\
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
	$(SRC)/utils/md5.cpp \
	$(SRC)/utils/filesystem.cpp \

COMMS	:=\
	$(CMM)/LKFlarm.cpp\
	$(CMM)/Parser.cpp\
	$(CMM)/ComPort.cpp\
	$(CMM)/GpsIdPort.cpp\
	$(CMM)/lkgpsapi.cpp\
	$(CMM)/SerialPort.cpp\
	$(CMM)/UpdateBaroSource.cpp \
	$(CMM)/UpdateMonitor.cpp \
	$(CMM)/UpdateQNH.cpp \
	$(CMM)/UtilsParser.cpp \
	$(CMM)/device.cpp \
	$(CMM)/Bluetooth/BtHandler.cpp \
	$(CMM)/Bluetooth/BtHandlerWince.cpp \
	$(CMM)/Bluetooth/BthPort.cpp \
	$(CMM)/Obex/CObexPush.cpp \


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
	$(DEV)/devEye.cpp \
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
	$(DEV)/devLXV7easy.cpp \
	$(DEV)/devLXV7_EXP.cpp \
	$(DEV)/devPosiGraph.cpp \
	$(DEV)/devVolkslogger.cpp \
	$(DEV)/devXCOM760.cpp \
	$(DEV)/devZander.cpp \
	$(DEV)/devWesterboer.cpp \
	$(DEV)/LKHolux.cpp \
	$(DEV)/LKRoyaltek3200.cpp	\
	$(DEV)/devFlyNet.cpp \
	$(DEV)/devCProbe.cpp \
	$(DEV)/devBlueFlyVario.cpp
		

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
	$(DLG)/dlgMultiSelectList.cpp \
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
	$(DLG)/dlgMultimaps.cpp\
	$(DLG)/dlgOracle.cpp \
	$(DLG)/dlgProfiles.cpp \
	$(DLG)/dlgStartPoint.cpp \
	$(DLG)/dlgStartTask.cpp \
	$(DLG)/dlgStartup.cpp \
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
	$(DLG)/Analysis/DrawOtherFunctions.cpp \
	$(DLG)/Analysis/DrawXYGrid.cpp \
	$(DLG)/Analysis/RenderBarograph.cpp \
	$(DLG)/Analysis/RenderClimb.cpp \
	$(DLG)/Analysis/RenderContest.cpp \
	$(DLG)/Analysis/RenderFAISector.cpp \
	$(DLG)/Analysis/RenderGlidePolar.cpp \
	$(DLG)/Analysis/RenderSpeed.cpp\
	$(DLG)/Analysis/RenderTask.cpp \
	$(DLG)/Analysis/RenderTemperature.cpp \
	$(DLG)/Analysis/RenderWind.cpp \
	$(DLG)/Analysis/ScaleFunctions.cpp \
	$(DLG)/Analysis/StyleLine.cpp \
	$(DLG)/Analysis/Update.cpp \
	$(DLG)/Analysis/dlgStatistics.cpp \
	$(DLG)/Task/AdjustAATTargets.cpp\
	$(DLG)/Task/InsertWaypoint.cpp\
	$(DLG)/Task/LoadTaskWaypoints.cpp\
	$(DLG)/Task/RemoveTaskPoint.cpp\
	$(DLG)/Task/RemoveWaypoint.cpp\
	$(DLG)/Task/ReplaceWaypoint.cpp\
	$(DLG)/Task/RotateStartPoints.cpp\
	$(DLG)/Task/SwapWaypoint.cpp\
	$(DLG)/dlgBluetooth.cpp\
	$(DLG)/dlgIgcFile.cpp\
	
SRC_FILES :=\
	$(WINDOW) \
	$(SCREEN) \
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
	$(SRC)/LKProfileInitRuntime.cpp\
	$(SRC)/LKProfileLoad.cpp\
	$(SRC)/LKProfileResetDefault.cpp\
	$(SRC)/LKProfileSave.cpp\
	$(SRC)/LKSimulator.cpp\
	$(SRC)/LKSimTraffic.cpp\
	$(SRC)/LKUtils.cpp \
	$(SRC)/LocalPath.cpp\
	$(SRC)/Locking.cpp\
	$(SRC)/Logger/DoSignature.cpp 	\
	$(SRC)/Logger/FlightDataRec.cpp 	\
	$(SRC)/Logger/LogBook.cpp\
	$(SRC)/Logger/Logger.cpp \
	$(SRC)/Logger/NMEAlogger.cpp\
	$(SRC)/Logger/ReplayLogger.cpp \
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
	$(SRC)/Multimap.cpp\
	$(SRC)/Oracle.cpp\
	$(SRC)/Polar.cpp		\
	$(SRC)/ProcessTimer.cpp \
	$(SRC)/Progress.cpp\
	$(SRC)/RotateScreen.cpp\
	$(SRC)/SaveLoadTask/ClearTask.cpp\
	$(SRC)/SaveLoadTask/DefaultTask.cpp\
	$(SRC)/SaveLoadTask/LoadNewTask.cpp\
	$(SRC)/SaveLoadTask/CTaskFileHelper.cpp\
	$(SRC)/SaveLoadTask/SaveDefaultTask.cpp\
	$(SRC)/SaveLoadTask/SaveTask.cpp\
	$(SRC)/SaveLoadTask/LoadCupTask.cpp\
	$(SRC)/SaveLoadTask/LoadGpxTask.cpp\
	$(SRC)/Settings.cpp\
	$(SRC)/Sound.cpp \
	$(SRC)/StatusFile.cpp \
	$(SRC)/Thread_Calculation.cpp\
	$(SRC)/Thread_Draw.cpp	\
	$(SRC)/TrueWind.cpp		\
	$(SRC)/units.cpp \
	$(SRC)/Utils.cpp		\
	$(SRC)/WindowControls.cpp \
	\
	$(LKINTER) \
	$(LIBRARY) \
	$(WAYPT) \
	$(DRAW) \
	$(CALC) \
	$(TASK) \
	$(TERRAIN) \
	$(TOPOL) \
	$(MAPDRAW) \
	$(UTILS) \
	$(COMMS) \
	$(DEVS) \
	$(DLGS) \
	$(VOLKS)


####### libraries
RSCSRC  := $(SRC)/Resource

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
	$(COMPATSRC)/errno.cpp 		$(COMPATSRC)/string_extras.cpp

POCOSRC:=$(LIB)/poco
POCO :=\
     $(POCOSRC)/Debugger.cpp \
     $(POCOSRC)/Bugcheck.cpp \
     $(POCOSRC)/ErrorHandler.cpp \
     $(POCOSRC)/Event.cpp \
     $(POCOSRC)/NamedEvent.cpp \
     $(POCOSRC)/Exception.cpp \
     $(POCOSRC)/Mutex.cpp \
     $(POCOSRC)/NamedMutex.cpp \
     $(POCOSRC)/Runnable.cpp \
     $(POCOSRC)/RWLock.cpp \
     $(POCOSRC)/Thread.cpp \
     $(POCOSRC)/ThreadLocal.cpp \
     $(POCOSRC)/ThreadTarget.cpp \
     $(POCOSRC)/Timestamp.cpp \
     $(POCOSRC)/Timespan.cpp \
     $(POCOSRC)/UnicodeConverter.cpp \
     $(POCOSRC)/UTF8Encoding.cpp \
     $(POCOSRC)/UTF16Encoding.cpp \
     $(POCOSRC)/TextEncoding.cpp \
     $(POCOSRC)/ASCIIEncoding.cpp \
     $(POCOSRC)/Latin1Encoding.cpp \
     $(POCOSRC)/Latin9Encoding.cpp \
     $(POCOSRC)/Windows1252Encoding.cpp \
     $(POCOSRC)/TextIterator.cpp \
     $(POCOSRC)/TextConverter.cpp \
     $(POCOSRC)/Ascii.cpp \
     $(POCOSRC)/AtomicCounter.cpp \
     $(POCOSRC)/RefCountedObject.cpp \


#ifneq ($(CONFIG_PC),y)
#COMPAT	:=$(COMPAT) \
#   $(COMPATSRC)/redir.cpp
#endif

DIALOG_XML = $(wildcard Common/Data/Dialogs/*.xml)

####### compilation outputs

# Add JP2 library for JP2000 unsupported raster maps
# (BIN)/jasper.a \

OBJS 	:=\
	$(patsubst $(SRC)%.cpp,$(BIN)%.o,$(SRC_FILES)) \
	$(BIN)/poco.a 
	
ifneq ($(WIN32_RESSOURCE), y)	
OBJS	+= $(BIN)/resource.a
endif

ifneq ($(CONFIG_LINUX),y)
OBJS	+= $(BIN)/zzip.a 
OBJS	+= $(BIN)/compat.a
OBJS	+= $(BIN)/lk8000.rsc
endif

IGNORE	:= \( -name .git \) -prune -o


####### dependency handling

DEPFILE		=$(dir $@).$(notdir $@).d
DEPFLAGS	=-Wp,-MD,$(DEPFILE)
dirtarget	=$(subst \\,_,$(subst /,_,$(dir $@)))
cc-flags	=$(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_$(dirtarget)) $(TARGET_ARCH)
cxx-flags	=$(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(CPPFLAGS_$(dirtarget)) $(TARGET_ARCH)


####### targets
.PHONY: FORCE all clean cleani tags rebuild cppcheck

all:	$(OUTPUTS)
	@$(NQ)echo "GCCVERSION : $(GCCVERSION)"
	
rebuild:
	@$(MAKE) clean
	@$(MAKE) all

clean: cleani
	@$(NQ)echo "  CLEAN   $(BIN)"
	$(Q)$(FIND) $(BIN) $(IGNORE) \( -name '*.[oa]' -o -name '*.rsc' -o -name '.*.d' -o -name '*.min.*' \) -type f -print | xargs -r $(RM)
	$(Q)$(RM) $(OUTPUTS_NS)
	$(Q)$(RM) $(OUTPUTS)

cleani:
	@$(NQ)echo "  CLEANI"
	$(Q)$(FIND) . $(IGNORE) \( -name '*.i' \) -type f -print | xargs -r $(RM)

tags:
	@$(NQ)echo "  TAGS"
	$(Q)$(ETAGS) --declarations --output=TAGS `find . -name *\\\.[ch] -or -name *\\\.cpp`
	$(Q)$(EBROWSE) -s `find . -name *\\\.[ch] -or -name *\\\.cpp`

cppcheck : 
	$(Q)cppcheck --force --enable=warning -q -j4 $(SRC_FILES)
#	$(Q)cppcheck --force --enable=warning -q -j4 $(ZZIPSRC)
#	$(Q)cppcheck --force --enable=warning -q -j4 $(COMPAT)
	

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

$(OUTPUTS) : $(OUTPUTS_NS)
	@$(NQ)echo "  STRIP   $@"
	$(Q)$(STRIP) $< -o $@
	$(Q)$(SIZE) $@

ifeq ($(REMOVE_NS),y)
	$(RM) $(OUTPUTS_NS)
endif

$(OUTPUTS_NS): $(OBJS)
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

$(BIN)/poco.a: $(patsubst $(SRC)%.cpp,$(BIN)%.o,$(POCO)) $(patsubst $(SRC)%.c,$(BIN)%.o,$(POCO))
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

$(BIN)/resource.a: $(BIN)/Resource/resource_data.o $(BIN)/Resource/resource_xml.o
	@$(NQ)echo "  AR      $@"
	$(Q)$(AR) $(ARFLAGS) $@ $^

$(BIN)/Resource/resource_xml.o:  $(BIN)/Resource/resource_xml.min.S
	@$(NQ)echo "  AS     $@"
	$(Q)$(MKDIR) $(dir $@)
	$(Q)$(AS) -I'$(dir $<)' $(OUTPUT_OPTION) $<

$(BIN)/Resource/resource_data.o:  $(RSCSRC)/resource_data.S
	@$(NQ)echo "  AS     $@"
	$(Q)$(MKDIR) $(dir $@)
	$(Q)$(AS) -I'$(dir $<)' $(OUTPUT_OPTION) $<

$(BIN)/Resource/resource_xml.min.S :  $(RSCSRC)/resource_xml.S $(patsubst Common/Data/Dialogs/%.xml,$(BIN)/Data/Dialogs/%.min.xml,$(DIALOG_XML))
	@$(NQ)echo "  update $@"
	@sed -r 's|(^.*")\.\./\.\./(Data/Dialogs[^"]+)(.xml".*)$$|\1\.\./\.\./\.\./$(BIN)/\2.min\3|g' $< > $@

$(BIN)/%.rsc: $(BIN)/%.min.rc 
	@$(NQ)echo "  WINDRES $@"
	$(Q)$(WINDRES) $(WINDRESFLAGS) $< $@

$(BIN)/%.min.rc: $(SRC)/%.rc $(patsubst Common/Data/Dialogs/%.xml,$(BIN)/Data/Dialogs/%.min.xml,$(DIALOG_XML))
	@echo "$@: $< " `sed -nr 's|^.*"\.\./(Data[^"]+)".*$$|Common/\1|gp' $<` > $(DEPFILE)
	@$(NQ)echo "  build $@"
	@sed -r 's|(^.*")\.\./(Data/Dialogs[^"]+)(.xml".*)$$|\1$(BIN)/\2.min\3|g' $< > $@

$(BIN)/Data/Dialogs/%.min.xml: Common/Data/Dialogs/%.xml
	@$(NQ)echo "  minimize $@"
	$(Q)xsltproc --output $@ build/dialogtemplate.xsl $<

.PRECIOUS: $(BIN)/Data/Dialogs/%.min.xml \
	$(BIN)/lk8000.min.rc

####### include depends files

ifneq ($(wildcard $(BIN)/.*.d),)
include $(wildcard $(BIN)/.*.d)
endif
ifneq ($(wildcard $(BIN)/*/.*.d),)
include $(wildcard $(BIN)/*/.*.d)
endif
ifneq ($(wildcard $(BIN)/*/*/.*.d),)
include $(wildcard $(BIN)/*/*/.*.d)
endif
ifneq ($(wildcard $(BIN)/*/*/*/.*.d),)
include $(wildcard $(BIN)/*/*/*/.*.d)
endif
