cmake_minimum_required(VERSION 3.6.0)

project(assembler C CXX ASM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(
    -DBOOST_NO_IOSTREAM
    -DBOOST_NO_IOSFWD
    -DBOOST_NO_STRINGSTREAM
    -DBOOST_NO_WSTREAMBUF
    -D__linux__
    -DHAVE_POSIX
    -DANDROID
    -D__STDC_FORMAT_MACROS
    -DENABLE_OPENGL
    -DHAVE_GLES
    -DHAVE_GLES2
    -DUSE_GLSL
    -DGL_GLEXT_PROTOTYPES
    -DUSE_EGL
    -DPOCO_NO_UNWINDOWS
    -DPOCO_ANDROID
    -DGLM_FORCE_RADIANS
    -DGLM_FORCE_SIZE_T_LENGTH
    -DUSE_CURL
)

if("${ANDROID_ABI}" MATCHES "arm")
    add_definitions(
        -DGLM_FORCE_NEON
        -DGLM_FORCE_SILENT_WARNINGS
    )
endif()

list(APPEND	BUILD_FLAGS
    -Werror
    -Wall
    -fno-math-errno
    -fopenmp
    -fsigned-char
    -Wunused-label 
    -Wunused-variable 
    -Wunused-value 
    -Wuninitialized 
    -Wmissing-field-initializers
    -Wparentheses
    -Wredundant-decls
    -Wno-unknown-warning-option
)
string(REPLACE ";" " " BUILD_FLAGS "${BUILD_FLAGS}")

set(CMAKE_C_FLAGS             "${CMAKE_C_FLAGS} ${BUILD_FLAGS} ")
set(CMAKE_CXX_FLAGS           "${CMAKE_CXX_FLAGS} ${BUILD_FLAGS} ")


add_subdirectory( Common/Source/Library/poco lib_poco )
add_subdirectory( Common/Source/Library/glutess lib_glutess )
add_subdirectory(Common/Source/Tracking/Curl tracking_http)

add_library( LK8000 SHARED

    Common/Source/utils/unicode/unicode_to_ascii.cpp # put it first to speedup parallel build 
    Common/Source/utils/unicode/UTF16.cpp
    Common/Source/utils/unicode/CP1252.cpp

    Common/Source/xcs/Android/Assert.cpp
    Common/Source/xcs/Android/Bitmap.cpp
    Common/Source/xcs/Android/Context.cpp
    Common/Source/xcs/Android/NativeView.cpp
    Common/Source/xcs/Android/Product.cpp
    Common/Source/xcs/Android/TextUtil.cpp
    Common/Source/xcs/Android/EventBridge.cpp
    Common/Source/xcs/Android/InternalSensors.cpp
    Common/Source/xcs/Android/SoundUtil.cpp
    Common/Source/xcs/Android/BluetoothHelper.cpp
    Common/Source/xcs/Android/NativeLeScanCallback.cpp
    Common/Source/xcs/Android/PortBridge.cpp
    Common/Source/xcs/Android/NativeInputListener.cpp
    Common/Source/xcs/Android/NativePortListener.cpp

    Common/Source/xcs/Android/IOIOHelper.cpp

    Common/Source/xcs/Java/String.cxx
    Common/Source/xcs/Java/Global.cxx
    Common/Source/xcs/Java/File.cxx
    Common/Source/xcs/Java/Object.cxx
    Common/Source/xcs/Java/Exception.cxx

    Common/Source/Android/Main.cpp
    Common/Source/Android/InternalPort.cpp
    Common/Source/Android/AndroidPort.cpp
    Common/Source/Android/BluetoothPort.cpp
    Common/Source/Android/BluetoothLeScanner.cpp

    Common/Source/Android/IOIOUartPort.cpp

    Common/Source/Android/UsbSerialHelper.cpp
    Common/Source/Android/UsbSerialPort.cpp

    Common/Source/Android/NetUtil.cpp

    Common/Source/Android/LK8000Activity.cpp

    Common/Source/Android/Vario/OscillatorSquare.h
    Common/Source/Android/Vario/VarioPlayer.h
    Common/Source/Android/Vario/VarioPlayer.cpp

    Common/Source/Android/AndroidFileUtils.h
    Common/Source/Android/AndroidFileUtils.cpp

    Common/Source/Android/Air3/PowerManagerUtils.h
    Common/Source/Android/Air3/PowerManagerUtils.cpp

    Common/Source/Window/WndMain.cpp

    Common/Source/Form/WndButtonImage.cpp
    Common/Source/Form/Clipboard.cpp

    Common/Source/xcs/Event/Shared/Timer.cpp
    Common/Source/xcs/Event/Shared/TimerQueue.cpp

    Common/Source/xcs/Event/Globals.cpp
    Common/Source/xcs/Event/Idle.cpp

    Common/Source/xcs/Event/Android/Loop.cpp
    Common/Source/xcs/Event/Android/Queue.cpp

    Common/Source/xcs/Screen/Debug.cpp
    Common/Source/xcs/Screen/Util.cpp
    Common/Source/xcs/Screen/Color.cpp
    Common/Source/xcs/Screen/BufferCanvas.cpp
    Common/Source/xcs/Screen/Window.cpp
    Common/Source/xcs/Screen/SingleWindow.cpp

    Common/Source/xcs/Screen/Custom/Pen.cpp
    Common/Source/xcs/Screen/Custom/TextWindow.cpp
    Common/Source/xcs/Screen/Custom/Window.cpp
    Common/Source/xcs/Screen/Custom/WList.cpp
    Common/Source/xcs/Screen/Custom/ContainerWindow.cpp
    Common/Source/xcs/Screen/Custom/TopWindow.cpp
    Common/Source/xcs/Screen/Custom/SingleWindow.cpp
    Common/Source/xcs/Screen/Custom/MoreCanvas.cpp
    Common/Source/xcs/Screen/Custom/Cache.cpp

    Common/Source/xcs/Screen/Android/Window.cpp
    Common/Source/xcs/Screen/Android/TopWindow.cpp
    Common/Source/xcs/Screen/Android/SingleWindow.cpp
    Common/Source/xcs/Screen/Android/TopCanvas.cpp
    Common/Source/xcs/Screen/Android/Bitmap.cpp
    Common/Source/xcs/Screen/Android/Font.cpp


    Common/Source/xcs/Screen/OpenGL/EGL.cpp

    Common/Source/xcs/Screen/OpenGL/Init.cpp
    Common/Source/xcs/Screen/OpenGL/Dynamic.cpp
    Common/Source/xcs/Screen/OpenGL/Rotate.cpp
    Common/Source/xcs/Screen/OpenGL/Globals.cpp
    Common/Source/xcs/Screen/OpenGL/Extension.cpp
    Common/Source/xcs/Screen/OpenGL/FBO.cpp
    Common/Source/xcs/Screen/OpenGL/VertexArray.cpp
    Common/Source/xcs/Screen/OpenGL/Bitmap.cpp
    Common/Source/xcs/Screen/OpenGL/RawBitmap.cpp
    Common/Source/xcs/Screen/OpenGL/Canvas.cpp
    Common/Source/xcs/Screen/OpenGL/BufferCanvas.cpp
    Common/Source/xcs/Screen/OpenGL/TopCanvas.cpp
    Common/Source/xcs/Screen/OpenGL/SubCanvas.cpp
    Common/Source/xcs/Screen/OpenGL/Texture.cpp
    Common/Source/xcs/Screen/OpenGL/UncompressedImage.cpp
    Common/Source/xcs/Screen/OpenGL/Buffer.cpp
    Common/Source/xcs/Screen/OpenGL/Shapes.cpp
    Common/Source/xcs/Screen/OpenGL/Triangulate.cpp
    Common/Source/xcs/Screen/OpenGL/Shaders.cpp

    Common/Source/xcs/Util/tstring.cpp
    Common/Source/xcs/Util/ConvertString.cpp
    Common/Source/xcs/Util/TruncateString.cpp
    Common/Source/xcs/Util/StaticString.cpp
    Common/Source/xcs/Util/UTF8.cpp
    Common/Source/xcs/Util/StaticString.cpp
    Common/Source/xcs/Util/StringUtil.cpp

    Common/Source/xcs/OS/Clock.cpp
    Common/Source/xcs/OS/EventPipe.cpp
    Common/Source/xcs/OS/FileDescriptor.cpp
    Common/Source/xcs/OS/Poll.cpp
    Common/Source/xcs/OS/FileUtil.cpp
    Common/Source/xcs/OS/Poll.cpp
    Common/Source/xcs/OS/Process.cpp
    Common/Source/xcs/OS/PathName.cpp

    Common/Source/xcs/IO/FileSource.cpp
    Common/Source/xcs/IO/InflateLineReader.cpp
    Common/Source/xcs/IO/InflateSource.cpp
    Common/Source/xcs/IO/LineSplitter.cpp

    Common/Source/xcs/Thread/Debug.cpp

    Common/Source/xcs/IO/Async/DiscardFileEventHandler.cpp
    Common/Source/xcs/IO/Async/IOLoop.cpp

    Common/Source/xcs/Hardware/CPU.cpp
    Common/Source/xcs/Hardware/RotateDisplay.cpp

    Common/Source/xcs/Math/Angle.cpp
    Common/Source/xcs/Math/KalmanFilter1d.cpp
    Common/Source/xcs/Math/SelfTimingKalmanFilter1d.cpp

    Common/Source/OS/Linux/CpuLoad.cpp
    Common/Source/OS/Linux/Memory.cpp
    Common/Source/OS/Linux/RotateScreen.cpp
    Common/Source/OS/Linux/WindowResize.cpp

    Common/Source/OS/Win/lscpu.cpp

    Common/Source/Screen/LKColor.cpp
    Common/Source/Screen/LKPen.cpp
    Common/Source/Screen/LKBitmap.cpp
    Common/Source/Screen/LKBrush.cpp
    Common/Source/Screen/LKSurface.cpp
    Common/Source/Screen/LKWindowSurface.cpp
    Common/Source/Screen/LKBitmapSurface.cpp
    Common/Source/Screen/LKIcon.cpp
    Common/Source/Screen/PolygonRenderer.cpp

    Common/Source/Airspace/LKAirspace.cpp
    Common/Source/Airspace/Sonar.cpp

    Common/Source/UIGlobals.cpp
    Common/Source/AirfieldDetails.cpp
    Common/Source/Alarms.cpp
    Common/Source/Backlight.cpp
    Common/Source/Battery.cpp
    Common/Source/Bitmaps.cpp
    Common/Source/Buttons.cpp
    Common/Source/ChangeScreen.cpp
    Common/Source/CommandLine.cpp
    Common/Source/ConditionMonitor.cpp
    Common/Source/DataOptions.cpp
    Common/Source/Dialogs.cpp
    Common/Source/DoInits.cpp
    Common/Source/ExpandMacros.cpp
    Common/Source/FlarmIdFile.cpp
    Common/Source/FlarmTools.cpp
    Common/Source/Fonts1.cpp
    Common/Source/Fonts2.cpp
    Common/Source/Geoid.cpp
    Common/Source/Globals.cpp
    Common/Source/InitFunctions.cpp
    Common/Source/InputEvents.cpp
    Common/Source/InputEvents_Default.cpp
    Common/Source/lk8000.cpp
    Common/Source/LKInstall.cpp
    Common/Source/LKLanguage.cpp
    Common/Source/LKObjects.cpp
	Common/Source/LKProfiles.cpp
    Common/Source/LKProfileInitRuntime.cpp
    Common/Source/LKProfileLoad.cpp
    Common/Source/LKProfileResetDefault.cpp
    Common/Source/LKProfileSave.cpp
    Common/Source/LKSimulator.cpp
    Common/Source/LKSimTraffic.cpp
    Common/Source/LKUtils.cpp
    Common/Source/LocalPath.cpp
    Common/Source/Locking.cpp
    Common/Source/Message.cpp
    Common/Source/MessageLog.cpp
    Common/Source/Models.cpp
    Common/Source/Multimap.cpp
    Common/Source/Oracle.cpp
    Common/Source/Polar.cpp
    Common/Source/ProcessTimer.cpp
    Common/Source/Settings.cpp
    Common/Source/Thread_Calculation.cpp
    Common/Source/Thread_Draw.cpp
    Common/Source/TrueWind.cpp
    Common/Source/TunedParameter.cpp
    Common/Source/units.cpp
    Common/Source/Utils.cpp
    Common/Source/WindowControls.cpp
    Common/Source/Sysop.cpp
    Common/Source/LKSnailTrail.cpp


    Common/Source/Logger/igc_file_writer.cpp
    Common/Source/Logger/FlightDataRec.cpp
    Common/Source/Logger/LogBook.cpp
    Common/Source/Logger/Logger.cpp
    Common/Source/Logger/NMEAlogger.cpp
    Common/Source/Logger/ReplayLogger.cpp
    Common/Source/Logger/StartStopLogger.cpp

    Common/Source/Topology/shapelib/mapalloc.c
    Common/Source/Topology/shapelib/mapbits.c
    Common/Source/Topology/shapelib/mapprimitive.c
    Common/Source/Topology/shapelib/mapsearch.c
    Common/Source/Topology/shapelib/mapshape.c
    Common/Source/Topology/shapelib/mapstring.c
    Common/Source/Topology/shapelib/maptree.c
    Common/Source/Topology/shapelib/mapxbase.c

    Common/Source/SaveLoadTask/ClearTask.cpp
    Common/Source/SaveLoadTask/DefaultTask.cpp
    Common/Source/SaveLoadTask/CTaskFileHelper.cpp
    Common/Source/SaveLoadTask/SaveDefaultTask.cpp
    Common/Source/SaveLoadTask/SaveTask.cpp
    Common/Source/SaveLoadTask/LoadCupTask.cpp
    Common/Source/SaveLoadTask/LoadGpxTask.cpp
    Common/Source/SaveLoadTask/LoadXCTrackTask.cpp

    Common/Source/LKInterface/LKCustomKeyHandler.cpp
    Common/Source/LKInterface/LKInit.cpp
    Common/Source/LKInterface/LKInitScreen.cpp
    Common/Source/LKInterface/LKInterface.cpp
    Common/Source/LKInterface/OverTargets.cpp
    Common/Source/LKInterface/VirtualKeys.cpp
    Common/Source/LKInterface/CScreenOrientation.cpp

    Common/Source/Library/ColorFunctions.cpp
    Common/Source/Library/DirectoryFunctions.cpp
    Common/Source/Library/DrawFunctions.cpp
    Common/Source/Library/leastsqs.cpp
    Common/Source/Library/magfield.cpp
    Common/Source/Library/MathFunctions.cpp
    Common/Source/Library/NavFunctions.cpp
    Common/Source/Library/PressureFunctions.cpp
    Common/Source/Library/rscalc.cpp
    Common/Source/Library/StringFunctions.cpp
    Common/Source/Library/TimeFunctions.cpp
    Common/Source/Library/Utm.cpp

    Common/Source/Library/cpp-mmf/memory_mapped_file.cpp

    Common/Source/Waypoints/AllocateWaypointList.cpp
    Common/Source/Waypoints/AltitudeFromTerrain.cpp
    Common/Source/Waypoints/CUPToLatLon.cpp
    Common/Source/Waypoints/Close.cpp
    Common/Source/Waypoints/FindMatchingWaypoint.cpp
    Common/Source/Waypoints/FindNearestFarVisible.cpp
    Common/Source/Waypoints/FindNearestWayPoint.cpp
    Common/Source/Waypoints/InTerrainRange.cpp
    Common/Source/Waypoints/InitWayPointCalc.cpp
    Common/Source/Waypoints/ParseCOMPE.cpp
    Common/Source/Waypoints/ParseCUP.cpp
    Common/Source/Waypoints/ParseDAT.cpp
    Common/Source/Waypoints/ParseOZI.cpp
    Common/Source/Waypoints/ParseOpenAIP.cpp
    Common/Source/Waypoints/Read.cpp
    Common/Source/Waypoints/ReadAltitude.cpp
    Common/Source/Waypoints/ReadFile.cpp
    Common/Source/Waypoints/SetHome.cpp
    Common/Source/Waypoints/ToString.cpp
    Common/Source/Waypoints/Virtuals.cpp
    Common/Source/Waypoints/Write.cpp

    Common/Source/Draw/CalculateScreen.cpp
    Common/Source/Draw/CalculateWaypointReachable.cpp
    Common/Source/Draw/DoAirspaces.cpp
    Common/Source/Draw/DoTarget.cpp
    Common/Source/Draw/DoTraffic.cpp
    Common/Source/Draw/DrawAircraft.cpp
    Common/Source/Draw/DrawAirSpaces.cpp
    Common/Source/Draw/DrawAirSpacesBorders.cpp
    Common/Source/Draw/DrawAirspaceLabels.cpp
    Common/Source/Draw/DrawBearing.cpp
    Common/Source/Draw/DrawBestCruiseTrack.cpp
    Common/Source/Draw/DrawCompass.cpp
    Common/Source/Draw/DrawCross.cpp
    Common/Source/Draw/DrawFAIOpti.cpp
    Common/Source/Draw/DrawFinalGlideBar.cpp
    Common/Source/Draw/DrawFlarmRadar.cpp
    Common/Source/Draw/DrawFlightMode.cpp
    Common/Source/Draw/DrawFuturePos.cpp
    Common/Source/Draw/DrawGlideThroughTerrain.cpp
    Common/Source/Draw/DrawGPSStatus.cpp
    Common/Source/Draw/DrawGreatCircle.cpp
    Common/Source/Draw/DrawHeading.cpp
    Common/Source/Draw/DrawHSI.cpp
    Common/Source/Draw/DrawMapScale.cpp
    Common/Source/Draw/DrawRunway.cpp
    Common/Source/Draw/DrawTRI.cpp
    Common/Source/Draw/DrawTask.cpp
    Common/Source/Draw/DrawTaskAAT.cpp
    Common/Source/Draw/DrawTeamMate.cpp
    Common/Source/Draw/DrawTerrainAbove.cpp
    Common/Source/Draw/DrawThermalBand.cpp
    Common/Source/Draw/DrawThermalEstimate.cpp
    Common/Source/Draw/DrawWind.cpp
    Common/Source/Draw/Draw_Primitives.cpp
    Common/Source/Draw/LKDrawBottomBar.cpp
    Common/Source/Draw/LKDrawFLARMTraffic.cpp
    Common/Source/Draw/LKDrawFanetData.cpp
    Common/Source/Draw/LKDrawInfoPage.cpp
    Common/Source/Draw/LKDrawLook8000.cpp
    Common/Source/Draw/LKDrawMapSpace.cpp
    Common/Source/Draw/LKDrawNearest.cpp
    Common/Source/Draw/LKDrawTargetTraffic.cpp
    Common/Source/Draw/LKDrawTrail.cpp
    Common/Source/Draw/LKDrawLongTrail.cpp
    Common/Source/Draw/LKDrawVario.cpp
    Common/Source/Draw/LKDrawWaypoints.cpp
    Common/Source/Draw/LKDrawWelcome.cpp
    Common/Source/Draw/LKGeneralAviation.cpp
    Common/Source/Draw/LKMessages.cpp
    Common/Source/Draw/LKProcess.cpp
    Common/Source/Draw/LKWriteText.cpp
    Common/Source/Draw/LoadSplash.cpp
    Common/Source/Draw/MapScale.cpp
    Common/Source/Draw/MapWindowA.cpp
    Common/Source/Draw/MapWindowMode.cpp
    Common/Source/Draw/MapWindowZoom.cpp
    Common/Source/Draw/MapWindow_Events.cpp
    Common/Source/Draw/MapWindow_Utils.cpp
    Common/Source/Draw/MapWndProc.cpp
    Common/Source/Draw/OrigAndOrient.cpp
    Common/Source/Draw/RenderMapWindow.cpp
    Common/Source/Draw/RenderMapWindowBg.cpp
    Common/Source/Draw/ScreenProjection.cpp
    Common/Source/Draw/TextInBox.cpp
    Common/Source/Draw/UpdateAndRefresh.cpp

    Common/Source/Draw/Task/TaskRenderer.cpp
    Common/Source/Draw/Task/TaskRendererCircle.cpp
    Common/Source/Draw/Task/TaskRendererLine.cpp
    Common/Source/Draw/Task/TaskRendererDae.cpp
    Common/Source/Draw/Task/TaskRendererSector.cpp
    Common/Source/Draw/Task/TaskRendererStartSector.cpp
    Common/Source/Draw/Task/TaskRendererSGPStart.cpp
    Common/Source/Draw/Task/TaskRendererMgr.cpp

    Common/Source/Draw/Multimaps/DrawMultimap.cpp
    Common/Source/Draw/Multimaps/DrawMultimap_Asp.cpp
    Common/Source/Draw/Multimaps/DrawMultimap_Radar.cpp
    Common/Source/Draw/Multimaps/DrawMultimap_Test.cpp
    Common/Source/Draw/Multimaps/GetVisualGlidePoints.cpp
    Common/Source/Draw/Multimaps/RenderAirspace.cpp
    Common/Source/Draw/Multimaps/RenderAirspaceTerrain.cpp
    Common/Source/Draw/Multimaps/RenderNearAirspace.cpp
    Common/Source/Draw/Multimaps/RenderPlane.cpp
    Common/Source/Draw/Multimaps/Sideview.cpp
    Common/Source/Draw/Multimaps/Sky.cpp
    Common/Source/Draw/Multimaps/TopView.cpp
    Common/Source/Draw/Multimaps/DrawVisualGlide.cpp

    Common/Source/Draw/BottomBar/BottomBar.cpp

    Common/Source/Calc/AddSnailPoint.cpp
    Common/Source/Calc/AltitudeRequired.cpp
    Common/Source/Calc/Atmosphere.cpp
    Common/Source/Calc/AutoMC.cpp
    Common/Source/Calc/AutoQNH.cpp
    Common/Source/Calc/AverageClimbRate.cpp
    Common/Source/Calc/Azimuth.cpp
    Common/Source/Calc/BallastDump.cpp
    Common/Source/Calc/BestAlternate.cpp
    Common/Source/Calc/Calculations2.cpp
    Common/Source/Calc/Calculations_Utils.cpp
    Common/Source/Calc/ClimbStats.cpp
    Common/Source/Calc/ContestMgr.cpp
    Common/Source/Calc/DistanceToHome.cpp
    Common/Source/Calc/DistanceToNext.cpp
    Common/Source/Calc/DoAlternates.cpp
    Common/Source/Calc/DoCalculations.cpp
    Common/Source/Calc/DoCalculationsSlow.cpp
    Common/Source/Calc/DoCalculationsVario.cpp
    Common/Source/Calc/DoCommon.cpp
    Common/Source/Calc/DoLogging.cpp
    Common/Source/Calc/DoNearest.cpp
    Common/Source/Calc/DoRangeWaypointList.cpp
    Common/Source/Calc/DoRecent.cpp
    Common/Source/Calc/FarFinalGlideThroughTerrain.cpp
    Common/Source/Calc/FinalGlideThroughTerrain.cpp
    Common/Source/Calc/Flaps.cpp
    Common/Source/Calc/FlarmCalculations.cpp
    Common/Source/Calc/FlightTime.cpp
    Common/Source/Calc/FreeFlight.cpp
    Common/Source/Calc/GlideThroughTerrain.cpp
    Common/Source/Calc/Heading.cpp
    Common/Source/Calc/HeadWind.cpp
    Common/Source/Calc/InitCloseCalculations.cpp
    Common/Source/Calc/LastThermalStats.cpp
    Common/Source/Calc/LD.cpp
    Common/Source/Calc/LDRotaryBuffer.cpp
    Common/Source/Calc/MagneticVariation.cpp
    Common/Source/Calc/McReady.cpp
    Common/Source/Calc/NettoVario.cpp
    Common/Source/Calc/Orbiter.cpp
    Common/Source/Calc/Pirker.cpp
    Common/Source/Calc/PredictNextPosition.cpp
    Common/Source/Calc/ResetFlightStats.cpp
    Common/Source/Calc/SetWindEstimate.cpp
    Common/Source/Calc/SpeedToFly.cpp
    Common/Source/Calc/TakeoffLanding.cpp
    Common/Source/Calc/TeamCodeCalculation.cpp
    Common/Source/Calc/TerrainFootprint.cpp
    Common/Source/Calc/TerrainHeight.cpp
    Common/Source/Calc/ThermalBand.cpp
    Common/Source/Calc/ThermalHistory.cpp
    Common/Source/Calc/ThermalLocator.cpp
    Common/Source/Calc/TotalEnergy.cpp
    Common/Source/Calc/Trace.cpp
    Common/Source/Calc/Turning.cpp
    Common/Source/Calc/Valid.cpp
    Common/Source/Calc/Vario.cpp
    Common/Source/Calc/WaypointApproxDistance.cpp
    Common/Source/Calc/WaypointArrivalAltitude.cpp
    Common/Source/Calc/windanalyser.cpp
    Common/Source/Calc/windmeasurementlist.cpp
    Common/Source/Calc/windstore.cpp
    Common/Source/Calc/WindEKF.cpp
    Common/Source/Calc/WindKalman.cpp
    Common/Source/Calc/Radio.cpp

    Common/Source/Calc/Task/AATCalculateIsoLines.cpp
    Common/Source/Calc/Task/AATDistance.cpp
    Common/Source/Calc/Task/AATStats.cpp
    Common/Source/Calc/Task/AATtools.cpp
    Common/Source/Calc/Task/AnnounceWPSwitch.cpp
    Common/Source/Calc/Task/CheckFinalGlide.cpp
    Common/Source/Calc/Task/CheckInSector.cpp
    Common/Source/Calc/Task/CheckStartRestartFinish.cpp
    Common/Source/Calc/Task/FAIFinishHeight.cpp
    Common/Source/Calc/Task/FlyDirectTo.cpp
    Common/Source/Calc/Task/InFinishSector.cpp
    Common/Source/Calc/Task/InSector.cpp
    Common/Source/Calc/Task/InStartSector.cpp
    Common/Source/Calc/Task/InTurnSector.cpp
    Common/Source/Calc/Task/InsideStartHeight.cpp
    Common/Source/Calc/Task/OptimizedTargetPos.cpp
    Common/Source/Calc/Task/ReadyToStartAdvance.cpp
    Common/Source/Calc/Task/RefreshTaskStatistics.cpp
    Common/Source/Calc/Task/SpeedHeight.cpp
    Common/Source/Calc/Task/StartTask.cpp
    Common/Source/Calc/Task/TaskAltitudeRequired.cpp
    Common/Source/Calc/Task/TaskSpeed.cpp
    Common/Source/Calc/Task/TaskStatistic.cpp
    Common/Source/Calc/Task/TaskUtils.cpp
    Common/Source/Calc/Task/TimeGates.cpp

    Common/Source/Calc/Task/RefreshTask/CalculateAATTaskSectors.cpp
    Common/Source/Calc/Task/RefreshTask/CalculateTaskSectors.cpp
    Common/Source/Calc/Task/RefreshTask/RefreshTask.cpp

    Common/Source/Calc/Task/PGTask/PGCircleTaskPt.cpp
    Common/Source/Calc/Task/PGTask/PGLineTaskPt.cpp
    Common/Source/Calc/Task/PGTask/PGTaskMgr.cpp
    Common/Source/Calc/Task/PGTask/PGSectorTaskPt.cpp
    Common/Source/Calc/Task/PGTask/PGEssCircleTaskPt.cpp
    Common/Source/Calc/Task/task_zone.cpp

    Common/Source/Terrain/OpenCreateClose.cpp
    Common/Source/Terrain/RasterTerrain.cpp
    Common/Source/Terrain/RAW.cpp
    Common/Source/Terrain/STScreenBuffer.cpp
    Common/Source/Terrain/STHeightBuffer.cpp

    Common/Source/Topology/Topology.cpp
    Common/Source/Topology/ShapeSpecialRenderer.cpp
    Common/Source/Topology/ShapePolygonRenderer.cpp

    Common/Source/MapDraw/DrawTerrain.cpp
    Common/Source/MapDraw/DrawTopology.cpp
    Common/Source/MapDraw/MarkLocation.cpp
    Common/Source/MapDraw/OpenCloseTopology.cpp
    Common/Source/MapDraw/SetTopologyBounds.cpp
    Common/Source/MapDraw/ZoomTopology.cpp

    Common/Source/utils/fileext.cpp
    Common/Source/utils/stringext.cpp
    Common/Source/utils/md5internal.cpp
    Common/Source/utils/md5.cpp
    Common/Source/utils/filesystem.cpp
    Common/Source/utils/openzip.cpp
    Common/Source/utils/zzip_stream.cpp
    Common/Source/utils/TextWrapArray.cpp
    Common/Source/utils/hmac_sha2.cpp
    Common/Source/utils/base64.cpp
    Common/Source/utils/charset_helper.cpp
    Common/Source/utils/printf.cpp

    Common/Source/Comm/ExternalWind.cpp
    Common/Source/Comm/LKFlarm.cpp
    Common/Source/Comm/LKFanet.cpp
    Common/Source/Comm/Parser.cpp
    Common/Source/Comm/ComCheck.cpp
    Common/Source/Comm/ComPort.cpp
    Common/Source/Comm/GpsIdPort.cpp
    Common/Source/Comm/lkgpsapi.cpp
    Common/Source/Comm/SerialPort.cpp
    Common/Source/Comm/TTYPort.cpp
    Common/Source/Comm/SocketPort.cpp
    Common/Source/Comm/TCPPort.cpp
    Common/Source/Comm/UpdateBaroSource.cpp
    Common/Source/Comm/UpdateMonitor.cpp
    Common/Source/Comm/UpdateQNH.cpp
    Common/Source/Comm/UtilsParser.cpp
    Common/Source/Comm/device.cpp
    Common/Source/Comm/FilePort.cpp

    Common/Source/Comm/Bluetooth/BtHandler.cpp
    Common/Source/Comm/Bluetooth/BtHandlerWince.cpp
    Common/Source/Comm/Bluetooth/BthPort.cpp

    Common/Source/Comm/Obex/CObexPush.cpp

    Common/Source/Comm/GpsWeekNumberFix.h
    Common/Source/Comm/GpsWeekNumberFix.cpp
    Common/Source/Comm/wait_ack.cpp

    Common/Source/Devices/devBase.cpp
    Common/Source/Devices/devBorgeltB50.cpp
    Common/Source/Devices/devCAI302.cpp
    Common/Source/Devices/devCaiGpsNav.cpp
    Common/Source/Devices/devCompeo.cpp
    Common/Source/Devices/devCondor.cpp
    Common/Source/Devices/devDigifly.cpp
    Common/Source/Devices/devDisabled.cpp
    Common/Source/Devices/devEW.cpp
    Common/Source/Devices/devEWMicroRecorder.cpp
    Common/Source/Devices/devFlymasterF1.cpp
    Common/Source/Devices/devFlytec.cpp
    Common/Source/Devices/devGeneric.cpp
    Common/Source/Devices/devIlec.cpp
    Common/Source/Devices/devIMI.cpp
    Common/Source/Devices/devNmeaOut.cpp
    Common/Source/Devices/devLKext1.cpp
    Common/Source/Devices/devLX.cpp
    Common/Source/Devices/devLX16xx.cpp
    Common/Source/Devices/devLXMiniMap.cpp
    Common/Source/Devices/devLXNano.cpp
    Common/Source/Devices/devLXV7.cpp
    Common/Source/Devices/devLXV7easy.cpp
    Common/Source/Devices/devLXV7_EXP.cpp
    Common/Source/Devices/devPosiGraph.cpp
    Common/Source/Devices/devVolkslogger.cpp
    Common/Source/Devices/devXCOM760.cpp
    Common/Source/Devices/devZander.cpp
    Common/Source/Devices/devWesterboer.cpp
    Common/Source/Devices/LKHolux.cpp
    Common/Source/Devices/LKRoyaltek3200.cpp
    Common/Source/Devices/devFlyNet.cpp
    Common/Source/Devices/devCProbe.cpp
    Common/Source/Devices/devBlueFlyVario.cpp
    Common/Source/Devices/devPVCOM.cpp
    Common/Source/Devices/devKRT2.cpp
    Common/Source/Devices/devLXNano3.cpp
    Common/Source/Devices/devXCTracer.cpp
    Common/Source/Devices/devGPSBip.cpp
    Common/Source/Devices/devAR620x.cpp
    Common/Source/Devices/devATR833.cpp
    Common/Source/Devices/devVaulter.cpp
    Common/Source/Devices/devOpenVario.cpp
    Common/Source/Devices/devFlarm.cpp
    Common/Source/Devices/devFlarm.cpp
    Common/Source/Devices/devLX_EOS_ERA.cpp
    Common/Source/Devices/devFanet.cpp
    Common/Source/Devices/devRCFenix.cpp
    Common/Source/Devices/devXCVario.cpp
    Common/Source/Devices/devGenericAutopilot.cpp
    Common/Source/Devices/devAirControlDisplay.cpp
    Common/Source/Devices/DeviceRegister.cpp

    Common/Source/Dialogs/dlgSelectObject.cpp
    Common/Source/Dialogs/dlgSelectItem.cpp
    Common/Source/Dialogs/dlgSelectAirspace.cpp
    Common/Source/Dialogs/dlgSelectWaypoint.cpp
    Common/Source/Dialogs/dlgAirspace.cpp
    Common/Source/Dialogs/dlgAirspaceFiles.cpp
    Common/Source/Dialogs/dlgWaypointFiles.cpp
    Common/Source/Dialogs/dlgAirspaceWarningParams.cpp
    Common/Source/Dialogs/dlgAirspaceColours.cpp
    Common/Source/Dialogs/dlgMultiSelectList.cpp
    Common/Source/Dialogs/dlgAirspaceDetails.cpp
    Common/Source/Dialogs/dlgAirspacePatterns.cpp
    Common/Source/Dialogs/dlgBasicSettings.cpp
    Common/Source/Dialogs/dlgBottomBar.cpp
    Common/Source/Dialogs/dlgChecklist.cpp
    Common/Source/Dialogs/dlgComboPicker.cpp
    Common/Source/Dialogs/dlgConfiguration.cpp
    Common/Source/Dialogs/dlgConfiguration2.cpp
    Common/Source/Dialogs/dlgCustomKeys.cpp
    Common/Source/Dialogs/dlgCustomMenu.cpp
    Common/Source/Dialogs/dlgHelp.cpp
    Common/Source/Dialogs/dlgInfoPages.cpp
    Common/Source/Dialogs/dlgLKAirspaceWarning.cpp
    Common/Source/Dialogs/dlgLKTraffic.cpp
    Common/Source/Dialogs/dlgLoggerReplay.cpp
    Common/Source/Dialogs/dlgMultimaps.cpp
    Common/Source/Dialogs/dlgOracle.cpp
    Common/Source/Dialogs/dlgOverlays.cpp
    Common/Source/Dialogs/dlgProfiles.cpp
    Common/Source/Dialogs/dlgRadioSettings.cpp
    Common/Source/Dialogs/dlgStartPoint.cpp
    Common/Source/Dialogs/dlgStartTask.cpp
    Common/Source/Dialogs/dlgStartup.cpp
    Common/Source/Dialogs/dlgStatus.cpp
    Common/Source/Dialogs/dlgTarget.cpp
    Common/Source/Dialogs/dlgTaskCalculator.cpp
    Common/Source/Dialogs/dlgTaskOverview.cpp
    Common/Source/Dialogs/dlgTaskRules.cpp
    Common/Source/Dialogs/dlgTimeGates.cpp
    Common/Source/Dialogs/dlgTopology.cpp
    Common/Source/Dialogs/dlgTaskWaypoint.cpp
    Common/Source/Dialogs/dlgTeamCode.cpp
    Common/Source/Dialogs/dlgTerminal.cpp
    Common/Source/Dialogs/dlgTextEntry_Keyboard.cpp
    Common/Source/Dialogs/dlgThermalDetails.cpp
    Common/Source/Dialogs/dlgTools.cpp
    Common/Source/Dialogs/dlgWayPointDetails.cpp
    Common/Source/Dialogs/dlgWayQuick.cpp
    Common/Source/Dialogs/dlgWaypointEdit.cpp
    Common/Source/Dialogs/dlgWaypointOutOfTerrain.cpp
    Common/Source/Dialogs/dlgWindSettings.cpp
    Common/Source/Dialogs/dlgBluetooth.cpp
    Common/Source/Dialogs/dlgIgcFile.cpp
    Common/Source/Dialogs/dlgProgress.cpp
    Common/Source/Dialogs/dlgFlarmIGCDownload.cpp
    Common/Source/Dialogs/dlgIGCProgress.cpp
    Common/Source/Dialogs/dlgLXIGCDownload.cpp
    Common/Source/Dialogs/dlgWeatherStDetails.cpp
    Common/Source/Dialogs/dlgEOSIGCDownload.cpp
    Common/Source/Dialogs/dlgRadioPriSecSel.cpp
    Common/Source/Dialogs/dlgMacCready.cpp

    Common/Source/Dialogs/Analysis/DrawOtherFunctions.cpp
    Common/Source/Dialogs/Analysis/DrawXYGrid.cpp
    Common/Source/Dialogs/Analysis/RenderBarograph.cpp
    Common/Source/Dialogs/Analysis/RenderClimb.cpp
    Common/Source/Dialogs/Analysis/RenderContest.cpp
    Common/Source/Dialogs/Analysis/RenderGlidePolar.cpp
    Common/Source/Dialogs/Analysis/RenderSpeed.cpp
    Common/Source/Dialogs/Analysis/RenderTask.cpp
    Common/Source/Dialogs/Analysis/RenderTemperature.cpp
    Common/Source/Dialogs/Analysis/RenderWind.cpp
    Common/Source/Dialogs/Analysis/ScaleFunctions.cpp
    Common/Source/Dialogs/Analysis/StyleLine.cpp
    Common/Source/Dialogs/Analysis/Update.cpp
    Common/Source/Dialogs/Analysis/dlgStatistics.cpp
    Common/Source/Dialogs/dlgNMEAReplay.cpp

    Common/Source/Dialogs/Task/AdjustAATTargets.cpp
    Common/Source/Dialogs/Task/InsertWaypoint.cpp
    Common/Source/Dialogs/Task/LoadTaskWaypoints.cpp
    Common/Source/Dialogs/Task/RemoveTaskPoint.cpp
    Common/Source/Dialogs/Task/RemoveWaypoint.cpp
    Common/Source/Dialogs/Task/ReplaceWaypoint.cpp
    Common/Source/Dialogs/Task/RotateStartPoints.cpp
    Common/Source/Dialogs/Task/SwapWaypoint.cpp

    Common/Source/Devices/Volkslogger/dbbconv.cpp
    Common/Source/Devices/Volkslogger/grecord.cpp
    Common/Source/Devices/Volkslogger/vlapi2.cpp
    Common/Source/Devices/Volkslogger/vlapihlp.cpp
    Common/Source/Devices/Volkslogger/vlapisys_win.cpp
    Common/Source/Devices/Volkslogger/vlconv.cpp
    Common/Source/Devices/Volkslogger/vlutils.cpp

    Common/Source/Android/fdsan.h
    Common/Source/Android/fdsan.c

    Common/Source/Library/zzip/err.c
    Common/Source/Library/zzip/fetch.c
    Common/Source/Library/zzip/file.c
    Common/Source/Library/zzip/info.c
    Common/Source/Library/zzip/plugin.c
    Common/Source/Library/zzip/zip.c

    Common/Source/Sound/Android/Sound.cpp

    Common/Source/Geographic/GeoPoint.cpp
    Common/Source/Geographic/TransverseMercator.cpp

    Common/Source/Resource/resource_data.S
    Common/Source/Resource/resource_xml.S

	Common/Source/Tracking/LiveTrack24.cpp
	Common/Source/Tracking/Tracking.cpp
    Common/Source/Tracking/SkylinesGlue.cpp
    Common/Source/Tracking/FFVLTracking.cpp

    Common/Source/NMEA/Derived.h
    Common/Source/NMEA/Info.h
    Common/Source/NMEA/Info.cpp

    Common/Source/xcs/Util/CRC.cpp
    Common/Source/xcs/Util/CRC.hpp

	Common/Source/xcs/Tracking/SkyLines/Client.hpp
    Common/Source/xcs/Tracking/SkyLines/Client.cpp
    Common/Source/xcs/Tracking/SkyLines/Data.hpp
    Common/Source/xcs/Tracking/SkyLines/Export.hpp
    Common/Source/xcs/Tracking/SkyLines/Features.hpp
	Common/Source/xcs/Tracking/SkyLines/Glue.hpp
    Common/Source/xcs/Tracking/SkyLines/Glue.cpp
    Common/Source/xcs/Tracking/SkyLines/Handler.hpp
    Common/Source/xcs/Tracking/SkyLines/Import.hpp
	Common/Source/xcs/Tracking/SkyLines/Key.hpp
	Common/Source/xcs/Tracking/SkyLines/Key.cpp
    Common/Source/xcs/Tracking/SkyLines/Protocol.hpp
    Common/Source/xcs/Tracking/SkyLines/Queue.hpp
    Common/Source/xcs/Tracking/SkyLines/Settings.hpp
	Common/Source/xcs/Tracking/SkyLines/Assemble.hpp
	Common/Source/xcs/Tracking/SkyLines/Assemble.cpp

	Common/Source/xcs/Tracking/Features.hpp
	Common/Source/xcs/Tracking/TrackingGlue.hpp
	Common/Source/xcs/Tracking/TrackingGlue.cpp
	Common/Source/xcs/Tracking/TrackingSettings.hpp
	Common/Source/xcs/Tracking/TrackingSettings.cpp

	Common/Source/xcs/IO/Async/GlobalIOThread.hpp
	Common/Source/xcs/IO/Async/GlobalIOThread.cpp
	Common/Source/xcs/IO/Async/IOThread.hpp
	Common/Source/xcs/IO/Async/IOThread.cpp

	Common/Source/xcs/Time/GPSClock.hpp

	Common/Source/xcs/Net/AllocatedSocketAddress.cxx
	Common/Source/xcs/Net/AllocatedSocketAddress.hxx
	Common/Source/xcs/Net/Features.hxx
	Common/Source/xcs/Net/IPv4Address.cxx
	Common/Source/xcs/Net/IPv4Address.hxx
	Common/Source/xcs/Net/SocketAddress.cxx
	Common/Source/xcs/Net/SocketAddress.hxx
	Common/Source/xcs/Net/SocketDescriptor.cpp
	Common/Source/xcs/Net/SocketDescriptor.hpp
	Common/Source/xcs/Net/SocketError.hpp
	Common/Source/xcs/Net/State.cpp
	Common/Source/xcs/Net/State.hpp
	Common/Source/xcs/Net/StaticSocketAddress.cxx
	Common/Source/xcs/Net/StaticSocketAddress.hxx
)

# resource_xml.S dependencies
file(
    GLOB xml_files
    Common/Data/Dialogs/*.xml
)

set_source_files_properties (
    Common/Source/Resource/resource_xml.S
    PROPERTIES OBJECT_DEPENDS "${xml_files}"
)

# resource_data.S dependencies
file(
    GLOB data_files
    Common/Data/Bitmaps/egm96s.dem
)

set_source_files_properties (
    Common/Source/Resource/resource_data.S
    PROPERTIES OBJECT_DEPENDS "${data_files}"
)

find_package (geographiclib REQUIRED CONFIG)
find_package (oboe REQUIRED CONFIG)
find_package (boost_config REQUIRED CONFIG)
find_package (boost_type_traits REQUIRED CONFIG)
find_package (boost_preprocessor REQUIRED CONFIG)
find_package (boost_core REQUIRED CONFIG)
find_package (boost_assert REQUIRED CONFIG)
find_package (boost_tuple REQUIRED CONFIG)
find_package (boost_container_hash REQUIRED CONFIG)
find_package (boost_utility REQUIRED CONFIG)
find_package (boost_detail REQUIRED CONFIG)
find_package (boost_static_assert REQUIRED CONFIG)
find_package (boost_move REQUIRED CONFIG)
find_package (boost_predef REQUIRED CONFIG)
find_package (boost_mp11 REQUIRED CONFIG)
find_package (boost_describe REQUIRED CONFIG)
find_package (boost_intrusive REQUIRED CONFIG)
find_package (boost_container REQUIRED)

target_include_directories( LK8000 PRIVATE
    Common/Header/linuxcompat
    Common/Source/Library
    Common/Header
    Common/Source
    Common/Source/xcs
    ./
    lib/doctest
    lib/glm
)

target_link_libraries(LK8000 PRIVATE
    LK8000_POCO
    LK8000_GLUTESS
    tracking_http

	geographiclib::geographiclib
	oboe::oboe
	boost_config::boost_config
	boost_type_traits::boost_type_traits
	boost_preprocessor::boost_preprocessor
	boost_core::boost_core
	boost_assert::boost_assert
	boost_tuple::boost_tuple
	boost_container_hash::boost_container_hash
	boost_utility::boost_utility
	boost_detail::boost_detail
	boost_static_assert::boost_static_assert
	boost_move::boost_move
	boost_predef::boost_predef
    boost_mp11::boost_mp11
    boost_describe::boost_describe
	boost_intrusive::boost_intrusive
	boost_container::boost_container
    android
    log
    z
    EGL
    GLESv2
    -static-openmp
)
