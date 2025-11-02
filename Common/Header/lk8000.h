/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: lk8000.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef LK8000_LK8000_H
#define LK8000_LK8000_H

#include "Sizes.h"
#include <stdint.h>
#include "Thread/Mutex.hpp"
#include "Modeltype.h"

extern Mutex  CritSec_FlightData;
extern void UnlockFlightData();
extern void LockFlightData();

extern Mutex  CritSec_TaskData;
extern void UnlockTaskData();
extern void LockTaskData();

extern void UnlockTerrainDataGraphics();
extern void LockTerrainDataGraphics();

extern void TriggerGPSUpdate();
bool CheckLastCalculationRun(unsigned duration);

extern void TriggerVarioUpdate();

void FocusOnWindow(int i, bool selected);
void FullScreen();

extern void PopupWaypointDetails();
extern void PopupAnalysis();

#ifndef DEG_TO_RAD
#define DEG_TO_RAD  (PI / 180)
#define RAD_TO_DEG  (180 / PI)
#endif

#if defined(M_PI)
  #undef M_PI
#endif
#define M_PI PI
#define M_2PI (2*PI)

void UpdateBatteryInfos(void);
void SwitchToMapWindow(void);

#if 0
typedef struct {
        int     array[RASIZE]; // rotary array with a predefined max capacity
        short   start;          // pointer to current first item in rotarybuf if used
        short   size;           // real size of rotary buffer (0-size)
} ifilter_s;
#endif

// position independent data, for wind calculation
struct windrotary_s {
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
};

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
  BestCruiseTrack_t BestCruiseTrack;
  IndLandable_t IndLandable;	// landable icon style
  int UTF8Pictorials;
  bool InverseInfoBox;		// InfoBox black or white inverted, used also by LK styles
} Appearance_t;


bool ExpandMacros(const TCHAR *In, TCHAR *OutBuffer, size_t Size);

#endif
