/*
Copyright_License {

  XCSoar Glide Computer - http://xcsoar.sourceforge.net/
  Copyright (C) 2000 - 2008  

  	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  $Id: externs.h,v 8.2 2009/06/24 10:06:04 root Exp root $
}
*/

#ifndef EXTERNS_H
#define EXTERNS_H

extern TCHAR XCSoar_Version[256];

#if !defined(AFX_EXTERNS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_EXTERNS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Sizes.h"
#include "XCSoar.h"
#include "Parser.h"
#include "Calculations.h"
#include "MapWindow.h"
#include "Task.h"
#include "Statistics.h"
#include "Dialogs.h"
#include "device.h" // 100210

#if (EXPERIMENTAL > 0)
//JMW#include "BlueSMS.h"
#endif


typedef enum {psInitInProgress=0, psInitDone=1, psFirstDrawDone=2, psNormalOp=3} StartupState_t;
// 0: not started at all
// 1: everything is alive
// 2: done first draw
// 3: normal operation


// instance of main program
extern HINSTANCE hInst;

extern StartupState_t ProgramStarted;


extern int UTCOffset;

#if defined(PNA) || defined(FIVV)  // VENTA2- ADD GlobalEllipse
extern int	GlobalModelType; 
extern TCHAR	GlobalModelName[];
extern float	GlobalEllipse;
extern TCHAR *	gmfpathname();
extern TCHAR *	gmfbasename();
extern int		GetGlobalModelName();
extern void		SmartGlobalModelType();
extern short		InstallSystem();
extern bool		CheckDataDir();
extern bool		CheckLanguageDir();
extern bool		CheckRegistryProfile();
extern void		ConvToUpper( TCHAR *);

#endif

#if LKSTARTUP
extern BYTE RUN_MODE;
#endif

// asset/registration data
extern TCHAR strAssetNumber[];
extern TCHAR strRegKey[];

// windows
extern HWND hWndMainWindow;           // HWND Main Window
extern HWND hWndMapWindow;            // HWND MapWindow
extern HWND hWndCB;		

// infoboxes
extern int  CurrentInfoType;          // Used for Popup Menu Select
extern int  InfoType[MAXINFOWINDOWS]; //
extern HWND hWndInfoWindow[MAXINFOWINDOWS];
extern int  InfoFocus;
extern bool DisplayLocked; // if infoboxes are locked
extern SCREEN_INFO Data_Options[];
extern const int NUMSELECTSTRINGS;
extern BOOL InfoBoxesHidden;
extern int numInfoWindows;

// waypoint data
extern int HomeWaypoint;
extern bool TakeOffWayPoint;
extern int AirfieldsHomeWaypoint; // VENTA3
extern int Alternate1; // VENTA3
extern int Alternate2;
extern int BestAlternate;
extern int ActiveAlternate;
extern bool  OnBestAlternate;
extern bool  OnAlternate1;
extern bool  OnAlternate2;

extern WAYPOINT *WayPointList;
extern WPCALC   *WayPointCalc; // VENTA3 additional calculated infos on WPs
extern unsigned int NumberOfWayPoints;
extern int WaypointsOutOfRange;
extern int  RangeLandableIndex[]; 
extern int  RangeAirportIndex[];
extern int  RangeTurnpointIndex[];

extern int  RangeLandableNumber;
extern int  RangeAirportNumber;
extern int  RangeTurnpointNumber;

extern int  SortedLandableIndex[];
extern int  SortedAirportIndex[];
extern int  SortedTurnpointIndex[];

extern int  SortedNumber;
extern int  SortedTurnpointNumber;
extern int  CommonIndex[];
extern int  CommonNumber;
extern int  RecentIndex[];
extern unsigned int RecentChecksum[];
extern int  RecentNumber;
// 0 Wpname 1 Distance 2 bearing, 3 reff, 4 altarr
extern short  SortedMode[];

extern bool  WarningHomeDir;
extern TCHAR LKLangSuffix[4];

// Specials
extern bool UseGeoidSeparation;
extern double GPSAltitudeOffset; 	// VENTA3
extern bool PressureHg;
//extern bool ShortcutIbox;
extern int  CustomKeyTime;
extern int  CustomKeyModeCenter;
extern int  CustomKeyModeLeft;
extern int  CustomKeyModeRight;
extern int  CustomKeyModeAircraftIcon;
extern int  CustomKeyModeLeftUpCorner;
extern int  CustomKeyModeRightUpCorner;
extern bool ResumeSession;
extern double QFEAltitudeOffset; // VENTA3
extern int OnAirSpace; // VENTA3 toggle DrawAirSpace
extern bool WasFlying; // used by auto QFE.. 
extern double LastFlipBoxTime; // used by XCSoar and Calculations
extern double LastRangeLandableTime;
#if defined(PNA) || defined(FIVV)
extern bool needclipping;
#endif
extern bool EnableAutoBacklight; // VENTA4
extern bool EnableAutoSoundVolume; // VENTA4
extern short AircraftCategory; // VENTA4
extern bool ExtendedVisualGlide;
extern short Look8000;
extern bool NewMap;
extern bool CheckSum;
extern bool HideUnits;
extern bool VirtualKeys;
extern short OutlinedTp;
extern int  OverColor;
extern COLORREF OverColorRef;
extern int TpFilter;
extern short MapBox;
extern short GlideBarMode; 
extern short ArrivalValue;
extern short NewMapDeclutter;
extern short AverEffTime;
extern bool MapLock;	// map locking status
extern bool UseMapLock;
extern bool ActiveMap; // 100318
extern bool DrawBottom;
extern short BottomMode; // Stripe number
extern short BottomSize; // Height of bottom stripe
extern short TopSize;
extern short SortBoxY;
extern short SortBoxX[];
extern short BottomGeom; // registry saved lk8000 navboxes geometry
extern short GlideBarOffset; // offset to the right for drawing LK8000 with GB active
extern bool  EngineeringMenu; // not saved in registry
extern short splitter;	
extern bool  iboxtoclick;
extern short DeclutterMode;

extern int PGClimbZoom;
extern int PGCruiseZoom;
extern int LKVarioBar;

extern short GestureSize;
extern bool IphoneGestures;
extern bool BlackScreen; // white on black.. 
extern bool LKTextBlack;
extern short BgMapColor;
extern bool BgMapColorTextBlack[];
extern int LKVarioSize;
extern bool PGZoomTrigger;
extern double  LastZoomTrigger;
extern double  LastDoTraffic;
extern double LastDoNearest;
extern double LastDoNearestTp;
extern double LastDoCommon;
extern double  CruiseMapScale;
extern double  ClimbMapScale;
extern int LKwdlgConfig;
extern double NmeaTime;
extern int NmeaHours, NmeaMinutes, NmeaSeconds;
extern bool PollingMode;

extern bool BestWarning;
extern bool ThermalBar;
extern bool McOverlay;
extern bool TrackBar;
extern double WindCalcSpeed;
extern int WindCalcTime;
extern bool RepeatWindCalc;
extern int LKTime_Real, LKTime_Ghost, LKTime_Zombie;
extern int WpFileType[3]; 
extern TCHAR WpHome_Name[NAME_SIZE+1];
extern double WpHome_Lat;
extern double WpHome_Lon;
extern double LKHearthBeats;
extern int PortMonitorMessages;
#ifdef NEWIBLSCALE
extern int LKIBLSCALE[MAXIBLSCALE+1];
#endif
extern int FlarmNetCount;
extern FLARM_TRAFFIC LKTraffic[FLARM_MAX_TRAFFIC+1];
extern int LKNumTraffic;
extern int LKSortedTraffic[FLARM_MAX_TRAFFIC+1];
extern int LKTargetIndex;
extern int LKTargetType;

extern int PGOpenTimeH;
extern int PGOpenTimeM;
extern int PGOpenTime;
extern int PGCloseTime;
extern int PGGateIntervalTime;
extern int PGNumberOfGates;
extern bool PGStartOut;
extern int ActiveGate;

#if LKTOPO
extern int LKTopo;
extern short LKWaterThreshold;
extern double LKTopoZoomCat05;
extern double LKTopoZoomCat10;
extern double LKTopoZoomCat20;
extern double LKTopoZoomCat30;
extern double LKTopoZoomCat40;
extern double LKTopoZoomCat50;
extern double LKTopoZoomCat60;
extern double LKTopoZoomCat70;
extern double LKTopoZoomCat80;
extern double LKTopoZoomCat90;
extern double LKTopoZoomCat100;
extern double LKTopoZoomCat110;
#endif
extern int LKMaxLabels;
#if OVERTARGET
extern short OvertargetMode;
#endif
// Simulator mode Turn rate, degrees per second (positive or negative)
extern double	SimTurn;
extern double ThLatitude;
extern double ThLongitude;
extern double ThermalRadius;
extern double SinkRadius;

// Append over here NEW

#if LK_CACHECALC

#if (LK_CACHECALC_MCA && LK_CACHECALC_MCA_STAT)
extern int  Cache_Calls_MCA;
extern int  Cache_Hits_MCA;
extern int  Cache_Fail_MCA;
extern int  Cache_False_MCA;
extern int  Cache_Incomplete_MCA;
#endif

#if (LK_CACHECALC_DBE && LK_CACHECALC_DBE_STAT)
extern int  Cache_Calls_DBE;
extern int  Cache_Hits_DBE;
extern int  Cache_Fail_DBE;
extern int  Cache_False_DBE;
#endif

#endif

// General index of all pages
extern short MapSpaceMode;
extern short SelectedRaw[];
extern short SelectedPage[];
extern short Numraws;
extern short Numpages;
extern short CommonNumraws;
extern short CommonNumpages;
extern short TurnpointNumraws;
extern short TurnpointNumpages;
extern short TrafficNumpages;
extern short ModeIndex;

// LK8000 sync flags
extern bool NearestDataReady;
extern bool CommonDataReady;
extern bool RecentDataReady;
extern bool NearestTurnpointDataReady;
extern bool LKForceDoNearest;
extern bool LKForceDoCommon;
extern bool LKForceDoRecent;
extern bool LKForceDoNearestTurnpoint;
extern short LKevent;
extern bool LKForceComPortReset;

extern ldrotary_s rotaryLD;
extern windrotary_s rotaryWind;
// airspace data
extern AIRSPACE_AREA *AirspaceArea;
extern AIRSPACE_POINT *AirspacePoint;
extern POINT *AirspaceScreenPoint;
extern AIRSPACE_CIRCLE *AirspaceCircle;
extern unsigned int NumberOfAirspacePoints;
extern unsigned int NumberOfAirspaceAreas;
extern unsigned int NumberOfAirspaceCircles;

extern short AltArrivMode;
extern bool GlobalRunning;

extern short ScreenSize;
extern int ScreenSizeX;
extern int ScreenSizeY;
extern RECT ScreenSizeR;
extern bool ScreenLandscape;

extern int time_in_flight;
extern int time_on_ground;
extern double TakeOffSpeedThreshold;

#ifdef COMDIAG
// Com port diagnostic - see Utils2.h
extern int ComPortStatus[NUMDEV+1];
extern long ComPortRx[NUMDEV+1];
extern long ComPortErrRx[NUMDEV+1];
extern long ComPortTx[NUMDEV+1];
extern long ComPortErrTx[NUMDEV+1];
extern long ComPortErrors[NUMDEV+1];
#endif
extern double ComPortHB[NUMDEV+1];

// Cpu stats
#ifdef CPUSTATS
extern int Cpu_Draw;
extern int Cpu_Calc;
extern int Cpu_Instrument;
extern int Cpu_Port;
extern int Cpu_Aver;
#endif

extern double	NearestAirspaceHDist;
extern double	NearestAirspaceVDist;
extern TCHAR	NearestAirspaceName[NAME_SIZE+1];

// Ready to use for experiments
extern double Experimental1;
extern double Experimental2;

// task data
extern START_POINT StartPoints[];
extern TASK_POINT Task[];
extern TASKSTATS_POINT TaskStats[];
extern int ActiveWayPoint;
extern bool TaskAborted;
extern int SelectedWaypoint;
extern int SectorType;
extern DWORD SectorRadius;

extern bool EnableMultipleStartPoints;
extern int StartLine;
extern DWORD StartRadius;
extern int FinishLine;
extern DWORD FinishRadius;
extern double AATTaskLength;
extern BOOL AATEnabled;
extern bool EnableFAIFinishHeight;
extern DWORD FinishMinHeight;
extern DWORD StartMaxHeight;
extern DWORD StartMaxHeightMargin;
extern DWORD StartMaxSpeed;
extern DWORD StartMaxSpeedMargin;
extern int StartHeightRef;
extern int OLCRules;
extern int Handicap;
extern bool EnableOLC;

// master flight data
extern NMEA_INFO GPS_INFO;
extern DERIVED_INFO CALCULATED_INFO;

// gps detection
extern BOOL GPSCONNECT;
extern BOOL VARIOCONNECT;

// units
extern double SPEEDMODIFY;
extern double LIFTMODIFY;
extern double DISTANCEMODIFY;
extern double ALTITUDEMODIFY; 
extern double TASKSPEEDMODIFY;

// polar info
extern double BUGS;
extern double BALLAST;
extern int POLARID;
extern double POLAR[POLARSIZE];
extern double WEIGHTS[POLARSIZE];
extern int BallastSecsToEmpty;
extern bool BallastTimerActive;

extern bool InfoWindowActive;

// snail trail
extern SNAIL_POINT SnailTrail[TRAILSIZE];
extern	int SnailNext;
extern int TrailLock;

// Logger
extern bool LoggerActive;
extern int LoggerTimeStepCruise;
extern int LoggerTimeStepCircling;

// user controls/parameters
extern double MACCREADY;
extern bool   AutoMacCready;
extern int  AutoMcMode;
extern double AccelerometerZero;
extern double SAFETYALTITUDEARRIVAL;
extern double SAFETYALTITUDEBREAKOFF;
extern double SAFETYALTITUDETERRAIN;
extern double SAFTEYSPEED;

extern int WindUpdateMode; // unused
extern double QNH;
extern int NettoSpeed;
extern bool EnableCalibration;
extern bool EnableAutoBlank;
extern bool EnableAuxiliaryInfo;
extern int debounceTimeout;
extern bool SetSystemTimeFromGPS;
extern bool ForceFinalGlide;
extern bool AutoForceFinalGlide;

// user interface options
extern bool bAirspaceBlackOutline;
extern int TrailActive;
extern int VisualGlide; // VENTA3
extern bool CircleZoom;
extern bool EnableTopology;
extern bool EnableTerrain;
extern int FinalGlideTerrain;
extern int AutoWindMode;
extern bool EnableNavBaroAltitude;
#if ORBITER
extern short Orbiter;
#endif
extern short Shading;
extern bool OverlayClock;
extern bool EnableSoundVario;
extern bool EnableSoundTask;
extern bool EnableSoundModes;
extern int SoundVolume;
extern int SoundDeadband;
extern int DisplayOrientation;
#if AUTORIENT
extern int OldDisplayOrientation;
extern int AutoOrientScale;
#endif
extern int DisplayTextType;
extern int AIRSPACEWARNINGS;
extern int WarningTime;
extern int AcknowledgementTime;
extern int AltitudeMode;
extern int SafetyAltitudeMode;
extern int ClipAltitude;
extern int AltWarningMargin;
extern bool EnableCDICruise;
extern bool EnableCDICircling;
extern bool EnableVarioGauge;
extern int AutoAdvance;
extern bool AdvanceArmed;
extern bool EnableBlockSTF; // block speed to fly instead of dolphin
extern int MenuTimeoutMax;
extern int EnableThermalLocator;
//

extern bool ExternalTriggerCruise;
extern bool ExternalTriggerCircling;
extern int EnableExternalTriggerCruise;

// statistics
extern Statistics flightstats;

// used in settings dialog 
extern BOOL COMPORTCHANGED;
extern BOOL AIRSPACEFILECHANGED;
extern BOOL WAYPOINTFILECHANGED;
extern BOOL TERRAINFILECHANGED;
extern BOOL AIRFIELDFILECHANGED;
extern BOOL TOPOLOGYFILECHANGED;
extern BOOL POLARFILECHANGED;
extern BOOL LANGUAGEFILECHANGED;
extern BOOL STATUSFILECHANGED;
extern BOOL INPUTFILECHANGED;
extern BOOL MAPFILECHANGED;
extern bool NEWWAYPOINTFILE;

bool Debounce();

// Team code
extern int TeamCodeRefWaypoint;
extern TCHAR TeammateCode[10];
extern double TeammateLatitude;
extern double TeammateLongitude;
extern bool TeammateCodeValid;
extern bool TeamFlarmTracking;
extern TCHAR TeamFlarmCNTarget[4]; // CN of the glider to track
extern int TeamFlarmIdTarget;    // FlarmId of the glider to track

extern bool DisableAutoLogger;

// Interface Globals
extern StatusMessageSTRUCT StatusMessageData[];
extern int StatusMessageData_Size;

extern bool RequestAirspaceWarningDialog;

extern int UserLevel;
extern int UseCustomFonts;
#if (EXPERIMENTAL > 0)
extern BlueDialupSMS bsms;
#endif

#if (WINDOWSPC>0) 
extern int SCREENWIDTH;
extern int SCREENHEIGHT;
#endif

#endif

#endif
