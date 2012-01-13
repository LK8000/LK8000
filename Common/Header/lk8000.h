/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: lk8000.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef LK8000_LK8000_H
#define LK8000_LK8000_H


typedef struct _DATAOPTIONS
{
  UnitGroup_t UnitGroup;
  TCHAR Description[DESCRIPTION_SIZE +1];
  TCHAR Title[TITLE_SIZE + 1];
} DATAOPTIONS;

extern void LockStartupStore();
extern void UnlockStartupStore();

extern void UnlockEventQueue();
extern void LockEventQueue();
extern void UnlockComm();
extern void LockComm();
extern void UnlockFlightData();
extern void CheckAndUnlockFlightData();
extern void LockFlightData();
extern void CheckAndLockFlightData();
extern void UnlockTaskData();
extern void LockTaskData();
extern void UnlockTerrainDataCalculations();
extern void LockTerrainDataCalculations();
extern void UnlockTerrainDataGraphics();
extern void LockTerrainDataGraphics();
extern void TriggerGPSUpdate();
extern void TriggerVarioUpdate();

void FocusOnWindow(int i, bool selected);
void FullScreen();

extern void PopupWaypointDetails();
extern void PopupAnalysis();
extern void RestartCommPorts();

#ifndef DEG_TO_RAD
#define DEG_TO_RAD                                      0.0174532925199432958
#define RAD_TO_DEG                                      57.2957795131
#endif

#if defined(M_PI)
  #undef M_PI
#endif
#define M_PI 3.14159265359
#define M_2PI 6.28318530718

extern "C" {
void DebugStore(const char *Str, ...);
}

void StartupStore(const TCHAR *Str, ...);
void FailStore(const TCHAR *Str, ...);

typedef struct
{
  BYTE acStatus;
  // 0 offline
  // 1 online
  // 255 unknown
  BYTE chargeStatus;
  // 1 high
  // 2 low
  // 4 critical
  // 8 charging
  // 128 no system battery
  // 255 unknown
  BYTE BatteryLifePercent;
  // 0-100 or 255 if unknown
  // VENTA-TEST BATTERY
  DWORD BatteryVoltage;
  DWORD BatteryCurrent;
  DWORD BatteryAverageCurrent;
  DWORD BatterymAHourConsumed;
  DWORD BatteryTemperature;
  DWORD BatteryLifeTime;
  DWORD BatteryFullLifeTime;
// END VENTA-TEST

} BATTERYINFO;


DWORD GetBatteryInfo(BATTERYINFO* pBatteryInfo);
void UpdateBatteryInfos(void);      
void SwitchToMapWindow(void);


#define MAPLABELS_ALLON		0
#define MAPLABELS_ONLYWPS	1
#define MAPLABELS_ONLYTOPO	2
#define MAPLABELS_ALLOFF	3



#if 0
typedef struct {
        int     array[RASIZE]; // rotary array with a predefined max capacity
        short   start;          // pointer to current first item in rotarybuf if used
        short   size;           // real size of rotary buffer (0-size)
} ifilter_s;
#endif

// position independent data, for wind calculation
typedef struct {
	// Ground Speed in m/s
	int speed[WCALC_ROTARYSIZE];
	// IAS if available
	int ias[WCALC_ROTARYSIZE];
	// Ground track
	int track[WCALC_ROTARYSIZE];
	// Digital compass heading, if available
	int compass[WCALC_ROTARYSIZE];
	// altitude in m
	int altitude[WCALC_ROTARYSIZE];
	// pointer to beginning of data in circular buffer
	int	start;
	// size of buffer in use: will reduce windrotarysize usage
	int	size;
} windrotary_s;

typedef struct {
        int     distance[MAXLDROTARYSIZE]; // rotary array with a predefined max capacity
        int     altitude[MAXLDROTARYSIZE]; 
        double     ias[MAXLDROTARYSIZE]; 
        double     totalias;
	int	totaldistance;
        short   start;          // pointer to current first item in rotarybuf if used
        short   size;           // real size of rotary buffer (0-size)
	bool	valid;
} ldrotary_s;

typedef struct {
	int triggervalue;	// example altitude limit
	int lastvalue;
	double lasttriggertime;	// last time trigger was issued
	short triggerscount;	// how many times alarm was issued
} lkalarms_s;

typedef struct {
  double Latitude;
  double Longitude;
  TCHAR  Name[MAXNEARESTTOPONAME+1];
  bool   Valid;
  double Distance;
  double Bearing;
} NearestTopoItem;

typedef struct{
  int DefaultMapWidth;
  BestCruiseTrack_t BestCruiseTrack;
  IndLandable_t IndLandable;	// landable icon style
  bool InverseInfoBox;		// InfoBox black or white inverted, used also by LK styles
  InfoBoxModelAppearance_t InfoBoxModel;
} Appearance_t;


bool ExpandMacros(const TCHAR *In, TCHAR *OutBuffer, size_t Size);

#endif 
