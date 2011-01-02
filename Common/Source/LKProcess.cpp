/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKProcess.cpp,v 1.8 2010/12/11 19:32:14 root Exp root $
*/

#include "StdAfx.h"
#include "options.h"
#include "Cpustats.h"
#include "XCSoar.h"
#include "Utils2.h"
#include "compatibility.h"
#include "MapWindow.h"
#include "Units.h"
#include "McReady.h"
#include "externs.h"
#include "InputEvents.h"
#include <windows.h>
#include <math.h>
#include <tchar.h>
#include "InfoBoxLayout.h"
#include "Logger.h"
#include "Process.h"
#include "Task.h"

// #define NULLSHORT	"--" 
#define NULLMEDIUM	"---"
#define NULLLONG	"---"
#define NULLTIME	"--:--"
#define INFINVAL	"oo"

extern int PDABatteryPercent;
extern int PDABatteryFlag;
extern int PDABatteryStatus;

// below this value, altitude differences are useless and not returned
//#define	ALTDIFFLIMIT	-2000


// Returns true if value is valid, false if not
// lktitle is shorter and limited to 6 or 7 chars, good for navboxes
// Units are empty by default, and valid is false by default
bool MapWindow::LKFormatValue(const short lkindex, const bool lktitle, TCHAR *BufferValue, TCHAR *BufferUnit, TCHAR *BufferTitle) {

  static int	index=-1;
  static double value;
  static int	ivalue;
  static char	text[LKSIZETEXT];
  static bool   doinit=true;
  static TCHAR	varformat[10];

  // By default, invalid return value. Set it to true after assigning value in cases
  bool		valid=false;

  if (doinit) {

	if (LIFTMODIFY==TOFEETPERMINUTE)
		_stprintf(varformat,TEXT("%%+.0f"));
	else
		_stprintf(varformat,TEXT("%%+0.1f"));

	doinit=false;
  }


	_tcscpy(BufferUnit,_T(""));

	switch(lkindex) {

		// B135
		case LK_TIME_LOCALSEC:
			Units::TimeToTextS(BufferValue, (int)DetectCurrentTime());
			valid=true;
			if (lktitle)
				_stprintf(BufferTitle,TEXT("Time"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			break;
		// B39
		case LK_TIME_LOCAL:
			Units::TimeToText(BufferValue, (int)DetectCurrentTime());
			valid=true;
			if (lktitle)
				_stprintf(BufferTitle,TEXT("Time"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			break;

		// B40
		case LK_TIME_UTC:
			Units::TimeToText(BufferValue,(int) GPS_INFO.Time);
			valid=true;
			if (lktitle)
				_stprintf(BufferTitle,TEXT("UTC"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			break;

		// B03
		case LK_BRG:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("Brg"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					// This value in AAT is not the waypoint bearing!
					value = WayPointCalc[index].Bearing;
					valid=true;
#ifndef __MINGW32__
					if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0f°"), value);
					else if (value < -1)
						_stprintf(BufferValue, TEXT("%2.0f°"), -value);
						else
							_tcscpy(BufferValue, TEXT("0°"));
#else
					if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0fÂ°"), value);
					else if (value < -1)
						_stprintf(BufferValue, TEXT("%2.0fÂ°"), -value);
						else
							_tcscpy(BufferValue, TEXT("0Â°"));
#endif
				}
			}
			break;

		// B78
		case LK_HOMERADIAL:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("Radl"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if ( ValidWayPoint(HomeWaypoint) != false ) {
				if (CALCULATED_INFO.HomeDistance >10.0) {
					// homeradial == 0, ok?
					value = CALCULATED_INFO.HomeRadial;
					valid=true;
#ifndef __MINGW32__
					if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0f°"), value);
					else if (value < -1)
						_stprintf(BufferValue, TEXT("%2.0f°"), -value);
						else
							_tcscpy(BufferValue, TEXT("0°"));
#else
					if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0fÂ°"), value);
					else if (value < -1)
						_stprintf(BufferValue, TEXT("%2.0fÂ°"), -value);
						else
							_tcscpy(BufferValue, TEXT("0Â°"));
#endif
				}
			}
			break;

		// B47
		case LK_BRGDIFF:
			wsprintf(BufferValue,_T(NULLMEDIUM)); // 091221
			if (lktitle)
				_stprintf(BufferTitle, TEXT("To"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					if (DisplayMode != dmCircling)
					{
						value = WayPointCalc[index].Bearing -  GPS_INFO.TrackBearing;
						valid=true;
						if (value < -180.0)
							value += 360.0;
						else
							if (value > 180.0)
								value -= 360.0;
#ifndef __MINGW32__
						if (value > 1)
							_stprintf(BufferValue, TEXT("%2.0f°»"), value);
						else if (value < -1)
							_stprintf(BufferValue, TEXT("«%2.0f°"), -value);
							else
								_tcscpy(BufferValue, TEXT("«»"));
#else
						if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0fÂ°Â»"), value);
						else if (value < -1)
						_stprintf(BufferValue, TEXT("Â«%2.0fÂ°"), -value);
						else
							_tcscpy(BufferValue, TEXT("Â«Â»"));
					}
#endif
				}
			}
			break;
#if 0
		// B151 UNUSED
		case LK_ALT1_BRGDIFF:
			wsprintf(BufferValue,_T(NULLMEDIUM)); 
			_stprintf(BufferTitle, TEXT("To"));
			if ( ValidWayPoint(Alternate1) != false ) {
				index = Alternate1;
				if (index>=0) {
					if (DisplayMode != dmCircling)
					{
						value = WayPointCalc[index].Bearing -  GPS_INFO.TrackBearing;
						valid=true;
						if (value < -180.0)
							value += 360.0;
						else
							if (value > 180.0)
								value -= 360.0;
#ifndef __MINGW32__
						if (value > 1)
							_stprintf(BufferValue, TEXT("%2.0f°»"), value);
						else if (value < -1)
							_stprintf(BufferValue, TEXT("«%2.0f°"), -value);
							else
								_tcscpy(BufferValue, TEXT("«»"));
#else
						if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0fÂ°Â»"), value);
						else if (value < -1)
						_stprintf(BufferValue, TEXT("Â«%2.0fÂ°"), -value);
						else
							_tcscpy(BufferValue, TEXT("Â«Â»"));
					}
#endif
				}
			}
			break;

		// B152 UNUSED
		case LK_ALT2_BRGDIFF:
			wsprintf(BufferValue,_T(NULLMEDIUM)); 
			_stprintf(BufferTitle, TEXT("To"));
			if ( ValidWayPoint(Alternate2) != false ) {
				index = Alternate2;
				if (index>=0) {
					if (DisplayMode != dmCircling)
					{
						value = WayPointCalc[index].Bearing -  GPS_INFO.TrackBearing;
						valid=true;
						if (value < -180.0)
							value += 360.0;
						else
							if (value > 180.0)
								value -= 360.0;
#ifndef __MINGW32__
						if (value > 1)
							_stprintf(BufferValue, TEXT("%2.0f°»"), value);
						else if (value < -1)
							_stprintf(BufferValue, TEXT("«%2.0f°"), -value);
							else
								_tcscpy(BufferValue, TEXT("«»"));
#else
						if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0fÂ°Â»"), value);
						else if (value < -1)
						_stprintf(BufferValue, TEXT("Â«%2.0fÂ°"), -value);
						else
							_tcscpy(BufferValue, TEXT("Â«Â»"));
					}
#endif
				}
			}
			break;

		// B153 UNUSED
		case LK_BALT_BRGDIFF:
			wsprintf(BufferValue,_T(NULLMEDIUM)); 
			_stprintf(BufferTitle, TEXT("To"));
			if ( ValidWayPoint(BestAlternate) != false ) {
				index = BestAlternate;
				if (index>=0) {
					if (DisplayMode != dmCircling)
					{
						value = WayPointCalc[index].Bearing -  GPS_INFO.TrackBearing;
						valid=true;
						if (value < -180.0)
							value += 360.0;
						else
							if (value > 180.0)
								value -= 360.0;
#ifndef __MINGW32__
						if (value > 1)
							_stprintf(BufferValue, TEXT("%2.0f°»"), value);
						else if (value < -1)
							_stprintf(BufferValue, TEXT("«%2.0f°"), -value);
							else
								_tcscpy(BufferValue, TEXT("«»"));
#else
						if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0fÂ°Â»"), value);
						else if (value < -1)
						_stprintf(BufferValue, TEXT("Â«%2.0fÂ°"), -value);
						else
							_tcscpy(BufferValue, TEXT("Â«Â»"));
					}
#endif
				}
			}
			break;

		// B155 UNUSED
		case LK_LASTTHERMAL_BRGDIFF:
			wsprintf(BufferValue,_T(NULLMEDIUM)); 
			_stprintf(BufferTitle, TEXT("To"));
			if ( ValidResWayPoint(RESWP_LASTTHERMAL) != false ) {
				index = RESWP_LASTTHERMAL;
				if (index>=0) {
					if (DisplayMode != dmCircling)
					{
						value = WayPointCalc[index].Bearing -  GPS_INFO.TrackBearing;
						valid=true;
						if (value < -180.0)
							value += 360.0;
						else
							if (value > 180.0)
								value -= 360.0;
#ifndef __MINGW32__
						if (value > 1)
							_stprintf(BufferValue, TEXT("%2.0f°»"), value);
						else if (value < -1)
							_stprintf(BufferValue, TEXT("«%2.0f°"), -value);
							else
								_tcscpy(BufferValue, TEXT("«»"));
#else
						if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0fÂ°Â»"), value);
						else if (value < -1)
						_stprintf(BufferValue, TEXT("Â«%2.0fÂ°"), -value);
						else
							_tcscpy(BufferValue, TEXT("Â«Â»"));
					}
#endif
				}
			}
			break;

#endif
		// B11
		case LK_NEXT_DIST:
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value=CALCULATED_INFO.WaypointDistance*DISTANCEMODIFY;
					valid=true;
					if (value>99)
						sprintf(text,"%.0f",value);
					else
						sprintf(text,"%.1f",value);
				} else {
					strcpy(text,NULLMEDIUM); // 091221
				}
			} else {
				strcpy(text,NULLMEDIUM); // 091221
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
				_tcscpy(BufferTitle, _T("Dis"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;
#if 0
		// B148  UNUSED
		case LK_ALT1_DIST:
			if(ValidWayPoint(Alternate1)) {
				index = Alternate1;
				if (index>=0) {
					value=WayPointCalc[index].Distance*DISTANCEMODIFY;
					valid=true;
					if (value>99)
						sprintf(text,"%.0f",value);
					else
						sprintf(text,"%.1f",value);
				} else {
					strcpy(text,NULLMEDIUM);
				}
			} else {
				strcpy(text,NULLMEDIUM);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			break;

		// B149  UNUSED
		case LK_ALT2_DIST:
			if(ValidWayPoint(Alternate2)) {
				index = Alternate2;
				if (index>=0) {
					value=WayPointCalc[index].Distance*DISTANCEMODIFY;
					valid=true;
					if (value>99)
						sprintf(text,"%.0f",value);
					else
						sprintf(text,"%.1f",value);
				} else {
					strcpy(text,NULLMEDIUM);
				}
			} else {
				strcpy(text,NULLMEDIUM);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			break;

		// B150  100919 UNUSED
		case LK_BALT_DIST:
			if(ValidWayPoint(BestAlternate)) {
				index = BestAlternate;
				if (index>=0) {
					value=WayPointCalc[index].Distance*DISTANCEMODIFY;
					valid=true;
					if (value>99)
						sprintf(text,"%.0f",value);
					else
						sprintf(text,"%.1f",value);
				} else {
					strcpy(text,NULLMEDIUM);
				}
			} else {
				strcpy(text,NULLMEDIUM);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			break;

		// B154  100919 UNUSED
		case LK_LASTTHERMAL_DIST:
			if(ValidResWayPoint(RESWP_LASTTHERMAL)) {
				index = RESWP_LASTTHERMAL;
				if (index>=0) {
					value=WayPointCalc[index].Distance*DISTANCEMODIFY;
					valid=true;
					if (value>99)
						sprintf(text,"%.0f",value);
					else
						sprintf(text,"%.1f",value);
				} else {
					strcpy(text,NULLMEDIUM);
				}
			} else {
				strcpy(text,NULLMEDIUM);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			break;
#endif

		// B147 Distance from the start sector, always available also after start
		case LK_START_DIST:
			if ( ValidTaskPoint(0) && ValidTaskPoint(1) ) { // if real task
				index = Task[0].Index;
				if (index>=0) {
					value=(CALCULATED_INFO.WaypointDistance-StartRadius)*DISTANCEMODIFY;
					if (value<0) value*=-1; // 101112 BUGFIX
					valid=true;
					if (value>99)
						sprintf(text,"%.0f",value);
					else {
						if (value>10) {
							sprintf(text,"%.1f",value);
						} else 
							sprintf(text,"%.3f",value);
					}
				} else {
					strcpy(text,NULLMEDIUM); // 091221
				}
			} else {
				strcpy(text,NULLMEDIUM); // 091221
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			_tcscpy(BufferTitle, _T("StDis"));
			break;


		// B60
		case LK_HOME_DIST:
			if (HomeWaypoint>=0) {
				if ( ValidWayPoint(HomeWaypoint) != false ) {
					value=CALCULATED_INFO.HomeDistance*DISTANCEMODIFY;
					valid=true;
					if (value>99)
						sprintf(text,"%.0f",value);
					else
						sprintf(text,"%.1f",value);
				} else {
					strcpy(text,NULLMEDIUM); // 091221
				}
			} else {
				strcpy(text,NULLMEDIUM); // 091221
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
				wsprintf(BufferTitle, TEXT("Home"),text);
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B83
		case LK_ODOMETER:
			if (CALCULATED_INFO.Odometer>0) {
				value=CALCULATED_INFO.Odometer*DISTANCEMODIFY;
				valid=true;
				if (value>99)
					sprintf(text,"%.0f",value);
				else
					sprintf(text,"%.1f",value);
			} else {
				strcpy(text,NULLMEDIUM); // 091221
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
				wsprintf(BufferTitle, TEXT("Odomet"),text);
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B28
		case LK_AA_DISTMAX:
		// B29
		case LK_AA_DISTMIN:
			if ( (ValidTaskPoint(ActiveWayPoint) != false) && AATEnabled ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					if ( lkindex == LK_AA_DISTMAX )
						value = DISTANCEMODIFY*CALCULATED_INFO.AATMaxDistance ;
					else
						value = DISTANCEMODIFY*CALCULATED_INFO.AATMinDistance ;
					valid=true;
					if (value>99)
						sprintf(text,"%.0f",value);
					else
						sprintf(text,"%.1f",value);
				} else {
					strcpy(text,NULLMEDIUM); // 091221
				}
			} else {
				strcpy(text,NULLMEDIUM); // 091221
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B51
		case LK_AA_TARG_DIST:
			if ( (ValidTaskPoint(ActiveWayPoint) != false) && AATEnabled ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value = DISTANCEMODIFY*CALCULATED_INFO.AATTargetDistance ;
					valid=true;
					if (value>99)
						sprintf(text,"%.0f",value);
					else
						sprintf(text,"%.1f",value);
				} else {
					strcpy(text,NULLMEDIUM); // 091221
				}
			} else {
				strcpy(text,NULLMEDIUM); // 091221
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B30
		case LK_AA_SPEEDMAX:
		// B31
		case LK_AA_SPEEDMIN:
			if ( (ValidTaskPoint(ActiveWayPoint) != false) && AATEnabled && CALCULATED_INFO.AATTimeToGo>=1 ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					if ( lkindex == LK_AA_SPEEDMAX )
						value = TASKSPEEDMODIFY*CALCULATED_INFO.AATMaxSpeed;
					else
						value = TASKSPEEDMODIFY*CALCULATED_INFO.AATMinSpeed;

					valid=true;
					sprintf(text,"%.0f",value);
				} else {
					strcpy(text,NULLMEDIUM);
				}
			} else {
				strcpy(text,NULLMEDIUM);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetTaskSpeedName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B52
		case LK_AA_TARG_SPEED:
			if ( (ValidTaskPoint(ActiveWayPoint) != false) && AATEnabled && CALCULATED_INFO.AATTimeToGo>=1 ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value = TASKSPEEDMODIFY*CALCULATED_INFO.AATTargetSpeed;
					valid=true;
					sprintf(text,"%.0f",value);
				} else {
					strcpy(text,NULLMEDIUM);
				}
			} else {
				strcpy(text,NULLMEDIUM);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetTaskSpeedName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B72	WP REQ EFF
		case LK_NEXT_GR:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("Req.E"));
			else
				_stprintf(BufferTitle, TEXT("Req.E"));
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value=WayPointCalc[index].GR;
					if (value <1 || value >=ALTERNATE_MAXVALIDGR )
						strcpy(text,NULLMEDIUM);
					else {
						if (value >= 100) sprintf(text,"%.0lf",value);
							else sprintf(text,"%.1lf",value);
						valid=true;
					}
					wsprintf(BufferValue, TEXT("%S"),text);
				}
			}
			break;

		// B71	LD AVR 
		case LK_LD_AVR:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("E.Avg"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if (DisplayMode != dmCircling) {
				value=CALCULATED_INFO.AverageLD;
				if (value <1 ||  value >=ALTERNATE_MAXVALIDGR ) {
					strcpy(text,INFINVAL); 
					valid=true;
				} else
					if (value==0)
						sprintf(text,NULLMEDIUM);
					else {
						if (value<100)
							sprintf(text,"%.1f",value);
						else
							sprintf(text,"%2.0f",value);
						valid=true;
					}
				wsprintf(BufferValue, TEXT("%S"),text);
			}
			break;

		// B12
		// Arrival altitude using current MC  and total energy. Does not use safetymc.
		// total energy is disabled!
		case LK_NEXT_ALTDIFF:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("NxtArr"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					// don't use current MC...
					value=ALTITUDEMODIFY*WayPointCalc[index].AltArriv[AltArrivMode];
					if ( value > ALTDIFFLIMIT ) {
						valid=true;
						_stprintf(BufferValue,TEXT("%+1.0f"), value);
					}
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B13
		// Using MC! 
		case LK_NEXT_ALTREQ:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("NexAltR"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value=ALTITUDEMODIFY*WayPointCalc[index].AltReqd[AltArrivMode];
					if (value<10000 && value >-10000) {
						_stprintf(BufferValue,TEXT("%1.0f"), value);
						valid=true;
					} 
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B05
		case LK_LD_CRUISE:
			if (lktitle)
                       		_stprintf(BufferTitle, TEXT("E.Th"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			value=DerivedDrawInfo.CruiseLD;
			if (value <-99 ||  value >=ALTERNATE_MAXVALIDGR ) {
				strcpy(text,INFINVAL); 
				valid=true;
			} else
			if (value==0) sprintf(text,NULLMEDIUM);
			else {
				sprintf(text,"%.0f",value);
				valid=true;
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			break;

		// B04
		case LK_LD_INST:
                        wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
                        	_stprintf(BufferTitle, TEXT("E.20\""));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			value=CALCULATED_INFO.LD;
			if (value <-99 ||  value >=ALTERNATE_MAXVALIDGR ) {
				strcpy(text,INFINVAL);
				valid=true;
			} else
				if (value==0) sprintf(text,NULLMEDIUM);
				else {
					sprintf(text,"%.0f",value);
					valid=true;
				}
			wsprintf(BufferValue, TEXT("%S"),text);
			break;

		// B00
		case LK_HNAV:
			if (lktitle)
                        	_stprintf(BufferTitle, TEXT("Alt"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			value=ALTITUDEMODIFY*DerivedDrawInfo.NavAltitude;
			valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B01		AltAgl HAGL 100318
		case LK_HAGL:
			if (lktitle)
                        	_stprintf(BufferTitle, TEXT("HAGL"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if (!CALCULATED_INFO.TerrainValid) { 
				wsprintf(BufferValue, TEXT(NULLLONG));
				wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
				valid=false;
				break;
			}
			value=ALTITUDEMODIFY*DerivedDrawInfo.AltitudeAGL;
			valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B33
		case LK_HBARO:
			if (GPS_INFO.BaroAltitudeAvailable) {
				value=ALTITUDEMODIFY*DrawInfo.BaroAltitude;
				valid=true;
				sprintf(text,"%d",(int)value);
				wsprintf(BufferValue, TEXT("%S"),text);
			} else
				wsprintf(BufferValue, TEXT(NULLLONG));
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B86
		case LK_HGPS:
			if (lktitle)
                        	_stprintf(BufferTitle, TEXT("HGPS"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if (GPS_INFO.NAVWarning || (GPS_INFO.SatellitesUsed == 0)) {
				wsprintf(BufferValue, TEXT(NULLLONG));
				valid=false;
			} else {
				value=ALTITUDEMODIFY*GPS_INFO.Altitude;
				valid=true;
				sprintf(text,"%d",(int)value);
				wsprintf(BufferValue, TEXT("%S"),text);
				wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			}
			break;

		// B70
		case LK_QFE:
			value=ALTITUDEMODIFY*DerivedDrawInfo.NavAltitude-QFEAltitudeOffset;;
			valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B20
		case LK_HGND:
                        wsprintf(BufferValue,_T(NULLLONG));
			if (DerivedDrawInfo.TerrainValid) {
				value=ALTITUDEMODIFY*DerivedDrawInfo.TerrainAlt;
				valid=true;
				sprintf(text,"%d",(int)value);
				wsprintf(BufferValue, TEXT("%S"),text);
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			if (lktitle)
				wsprintf(BufferTitle, TEXT("Gnd"),text);
			else 
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B23
		case LK_TRACK:
			wsprintf(BufferValue,_T(NULLLONG));
			//_stprintf(BufferUnit,TEXT(""));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("Trk"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			value = GPS_INFO.TrackBearing;
			valid=true;
#ifndef __MINGW32__
			if (value > 1)
				_stprintf(BufferValue, TEXT("%2.0f°"), value);
			else if (value < -1)
				_stprintf(BufferValue, TEXT("%2.0f°"), -value);
				else
					_tcscpy(BufferValue, TEXT("0°"));
#else
			if (value > 1)
				_stprintf(BufferValue, TEXT("%2.0fÂ°"), value);
			else if (value < -1)
				_stprintf(BufferValue, TEXT("%2.0fÂ°"), -value);
				else
					_tcscpy(BufferValue, TEXT("0Â°"));
#endif
			break;


		// B06
		case LK_GNDSPEED:
			value=SPEEDMODIFY*DrawInfo.Speed;
			valid=true;
			if (value<0||value>9999) value=0; else valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			if (lktitle)
				wsprintf(BufferTitle, TEXT("GS"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;


		// B32
		case LK_IAS:
			if (GPS_INFO.AirspeedAvailable) {
				if (lktitle)
					wsprintf(BufferTitle, TEXT("IAS"));
				else
					_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
				value=SPEEDMODIFY*DrawInfo.IndicatedAirspeed;
				if (value<0||value>999) value=0; else valid=true;
				sprintf(text,"%d",(int)value);
				wsprintf(BufferValue, TEXT("%S"),text);
			} else {
				wsprintf(BufferTitle, TEXT("eIAS"));
				value=SPEEDMODIFY*DerivedDrawInfo.IndicatedAirspeedEstimated;
				if (value<0||value>999) value=0; else valid=true;
				sprintf(text,"%d",(int)value);
				wsprintf(BufferValue, TEXT("%S"),text);
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			break;
				
		// B43 AKA STF
		case LK_SPEED_DOLPHIN:
			// if (GPS_INFO.AirspeedAvailable) {
				value=SPEEDMODIFY*DerivedDrawInfo.VOpt;
				if (value<0||value>999) value=0; else valid=true;
				sprintf(text,"%d",(int)value);
				wsprintf(BufferValue, TEXT("%S"),text);
			// } else
			//	wsprintf(BufferValue, TEXT(NULLMEDIUM));
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B87  100908
		case LK_EQMC:
			wsprintf(BufferTitle, TEXT("eqMC"));
			if ( CALCULATED_INFO.Circling == TRUE || CALCULATED_INFO.EqMc<0 || CALCULATED_INFO.OnGround == TRUE) {
				wsprintf(BufferValue, TEXT(NULLMEDIUM));
			} else {
				value = iround(LIFTMODIFY*CALCULATED_INFO.EqMc*10)/10.0;
				valid=true;
				sprintf(text,"%2.1lf",value);
				wsprintf(BufferValue, TEXT("%S"),text);
			}
			break;
				
		// B44
		case LK_NETTO:
			value=LIFTMODIFY*DerivedDrawInfo.NettoVario;
			if (value<-100||value>100) value=0; else valid=true;
			_stprintf(BufferValue,varformat,value);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetVerticalSpeedName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;


		// B54 091221
		case LK_TAS:
			if (GPS_INFO.AirspeedAvailable) {
				if (lktitle)
					wsprintf(BufferTitle, TEXT("TAS"));
				else
					_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
				value=SPEEDMODIFY*DrawInfo.TrueAirspeed;
				if (value<0||value>999) {
					sprintf(text,"%s",NULLMEDIUM);
				} else {
					valid=true;
					sprintf(text,"%d",(int)value);
				}
				wsprintf(BufferValue, TEXT("%S"),text);
			} else {
				wsprintf(BufferTitle, TEXT("eTAS"));
				value=SPEEDMODIFY*DerivedDrawInfo.TrueAirspeedEstimated;
				if (value<0||value>999) {
					sprintf(text,"%s",NULLMEDIUM);
				} else {
					valid=true;
					sprintf(text,"%d",(int)value);
				}
				wsprintf(BufferValue, TEXT("%S"),text);
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			break;

		// B55  Team Code 091216
		case LK_TEAM_CODE:
			if(ValidWayPoint(TeamCodeRefWaypoint)) {
				_tcsncpy(BufferValue,CALCULATED_INFO.OwnTeamCode,5);
				BufferValue[5] = '\0';
				valid=true; // 091221
			} else
				wsprintf(BufferValue,_T("----"));
			if (lktitle)
				wsprintf(BufferTitle, TEXT("TmCode"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B56  Team Code 091216
		case LK_TEAM_BRG:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("TmBrg"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if(ValidWayPoint(TeamCodeRefWaypoint) && TeammateCodeValid) {
				value=CALCULATED_INFO.TeammateBearing;
				valid=true;
				if (value > 1)
					_stprintf(BufferValue, TEXT("%2.0fÂ°"), value);
				else if (value < -1)
					_stprintf(BufferValue, TEXT("%2.0fÂ°"), -value);
				else
					_tcscpy(BufferValue, TEXT("0Â°"));
			}
			break;

		// B57 Team Bearing Difference 091216
		case LK_TEAM_BRGDIFF:
			wsprintf(BufferValue,_T(NULLLONG));

			if (lktitle)
				_stprintf(BufferTitle, TEXT("TmBd"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if (ValidWayPoint(TeamCodeRefWaypoint) && TeammateCodeValid) {
				value = CALCULATED_INFO.TeammateBearing -  GPS_INFO.TrackBearing;
				valid=true; // 091221
				if (value < -180.0)
					value += 360.0;
				else
					if (value > 180.0) value -= 360.0;

				if (value > 1)
					_stprintf(BufferValue, TEXT("%2.0fÂ°Â»"), value);
				else if (value < -1)
					_stprintf(BufferValue, TEXT("Â«%2.0fÂ°"), -value);
					else
						_tcscpy(BufferValue, TEXT("Â«Â»"));
			}
			break;

		// B58 091216 Team Range Distance
		case LK_TEAM_DIST:
			if ( TeammateCodeValid ) {
				value=DISTANCEMODIFY*CALCULATED_INFO.TeammateRange;
				valid=true;
				if (value>99)
					sprintf(text,"%.0f",value);
				else
					sprintf(text,"%.1f",value);
			} else {
				strcpy(text,NULLLONG);
			}

			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
				_tcscpy(BufferTitle, _T("TmDis"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B34
		case LK_SPEED_MC:
			value=SPEEDMODIFY*DerivedDrawInfo.VMacCready;
			valid=true;
			if (value<=0||value>999) value=0; else valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			if (lktitle)
				wsprintf(BufferTitle, TEXT("SpMC"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;


		// B35
		case LK_PRCCLIMB:
			value=DerivedDrawInfo.PercentCircling;
			valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%%"));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;


		// B73
		case LK_FL:
			if (lktitle)
				_stprintf(BufferTitle, TEXT("FL"));
			else
				_stprintf(BufferTitle, TEXT("FL"));
			value=(TOFEET*DerivedDrawInfo.NavAltitude)/100.0;
			valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			//_stprintf(BufferUnit,TEXT(""));
			break;

		// B131
		case LK_WIND:
			_stprintf(BufferTitle, TEXT("Wind"));
			if (DerivedDrawInfo.WindSpeed*SPEEDMODIFY>=1) {
				value = DerivedDrawInfo.WindBearing;
				valid=true;
				if (value==360) value=0;
				if (HideUnits)
					_stprintf(BufferValue,TEXT("%1.0f/%1.0f"), 
						value, SPEEDMODIFY*DerivedDrawInfo.WindSpeed );
				else
					_stprintf(BufferValue,TEXT("%1.0f")_T(DEG)_T("/%1.0f"), 
						value, SPEEDMODIFY*DerivedDrawInfo.WindSpeed );
			} else {
				_stprintf(BufferValue,TEXT("--/--"));
			}
			break;

		// B25
		case LK_WIND_SPEED:
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			wsprintf(BufferUnit, TEXT("%s"),Units::GetHorizontalSpeedName());
			
			value=DerivedDrawInfo.WindSpeed*SPEEDMODIFY;
			if (value>=1 ) {
				_stprintf(BufferValue,TEXT("%1.0f"), value );
				valid=true;
			} else {
				_stprintf(BufferValue,TEXT(NULLMEDIUM)); // 091221
			}
			break;

		// B26
		case LK_WIND_BRG:
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if (DerivedDrawInfo.WindSpeed*SPEEDMODIFY>=1) {
				value = DerivedDrawInfo.WindBearing;
				valid=true;
				if (value==360) value=0;
				_stprintf(BufferValue,TEXT("%1.0f")_T(DEG), value );
			} else {
				_stprintf(BufferValue,TEXT(NULLMEDIUM));
			}
			
			break;

		// B07  091221
		case LK_TL_AVG:
			value= LIFTMODIFY*CALCULATED_INFO.LastThermalAverage;
			if (value==0)
				sprintf(text,NULLMEDIUM);
			else { 
				valid=true;
				if (value<20) sprintf(text,"%+.1lf",value);
					else sprintf(text,"%+.0lf",value);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
			if (lktitle)
				wsprintf(BufferTitle, TEXT("TL.Avg"),text);
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B08 091216 091221
		case LK_TL_GAIN:
			value=ALTITUDEMODIFY*DerivedDrawInfo.LastThermalGain;
			if (value==0)
				sprintf(text,NULLMEDIUM);
			else { 
				valid=true;
				sprintf(text,"%+d",(int)value);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			if (lktitle)
				wsprintf(BufferTitle, TEXT("TL.Gain"),text);
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B21 091221
		case LK_TC_AVG:
			value= LIFTMODIFY*CALCULATED_INFO.AverageThermal;
			if (value==0)
				sprintf(text,NULLMEDIUM);
			else { 
				if (value<20) sprintf(text,"%+.1lf",value);
				else sprintf(text,"%+.0lf",value);
				valid=true; 
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
			if (lktitle)
				wsprintf(BufferTitle, TEXT("TC.Avg"),text);
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B10
		case LK_MC:
			value = iround(LIFTMODIFY*MACCREADY*10)/10.0;
			valid=true;
			//sprintf(text,"%.1lf",value);
			sprintf(text,"%2.1lf",value);
			wsprintf(BufferValue, TEXT("%S"),text);
			//if (!ValidTaskPoint(ActiveWayPoint) && ((AutoMcMode==0) || (AutoMcMode==2))) {
			if (!CALCULATED_INFO.AutoMacCready) {
				if (lktitle)
					wsprintf(BufferTitle, TEXT("ManMC"));
				else
					_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			} else {
				if (lktitle)
					wsprintf(BufferTitle, TEXT("AutMC"));
				else
					_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			}

			break;

		// B2 091221
		case LK_TC_30S:
			value=LIFTMODIFY*DerivedDrawInfo.Average30s;
			if (value==0)
				sprintf(text,NULLMEDIUM);
			else { 
				valid=true;
				if (value<20) sprintf(text,"%+.1lf",value);
					else sprintf(text,"%+.0lf",value);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
			if (lktitle)
				wsprintf(BufferTitle, TEXT("TC.30\""));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B22 091221
		case LK_TC_GAIN:
			value=ALTITUDEMODIFY*DerivedDrawInfo.ThermalGain;
			if (value==0)
				sprintf(text,NULLMEDIUM);
			else { 
				valid=true;
				sprintf(text,"%+d",(int)value);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			if (lktitle)
				wsprintf(BufferTitle, TEXT("TC.Gain"),text);
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B63 091221
		case LK_TC_ALL:
			if (CALCULATED_INFO.timeCircling <=0)
				//value=0.0;
				sprintf(text,NULLMEDIUM);
			else {
				value = LIFTMODIFY*CALCULATED_INFO.TotalHeightClimb /CALCULATED_INFO.timeCircling;
				if (value<20)
					sprintf(text,"%+.1lf",value);
				else
					sprintf(text,"%+.0lf",value);
				valid=true;
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
			if (lktitle)
				wsprintf(BufferTitle, TEXT("Th.All"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
                	break;

		// B24
		case LK_VARIO:
			if (GPS_INFO.VarioAvailable) {
				value = LIFTMODIFY*GPS_INFO.Vario;
			} else {
				value = LIFTMODIFY*CALCULATED_INFO.Vario;
			}
			valid=true;
			_stprintf(BufferValue,varformat,value);
			wsprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
			if (lktitle)
				wsprintf(BufferTitle, TEXT("Vario"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B15 Arrival altitude , no more total energy
		case LK_FIN_ALTDIFF:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("TskArr"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value=ALTITUDEMODIFY*DerivedDrawInfo.TaskAltitudeDifference;
					if ( value > ALTDIFFLIMIT ) {
						valid=true;
						_stprintf(BufferValue,TEXT("%+1.0f"), value);
					}
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B16
		case LK_FIN_ALTREQ:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("TskAltR"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value=ALTITUDEMODIFY*DerivedDrawInfo.TaskAltitudeRequired;
					if (value<10000 && value >-10000) {
						_stprintf(BufferValue,TEXT("%1.0f"), value);
						valid=true;
					} 
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B18
		case LK_FIN_DIST:
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					if (CALCULATED_INFO.ValidFinish) {
						value = DISTANCEMODIFY*CALCULATED_INFO.WaypointDistance;
					} else {
						value = DISTANCEMODIFY*CALCULATED_INFO.TaskDistanceToGo;
					}
					valid=true;
					if (value>99)
						sprintf(text,"%.0f",value);
					else
						sprintf(text,"%.1f",value);
				} else {
					strcpy(text,NULLLONG);
				}
			} else {
				strcpy(text,NULLLONG);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
				_tcscpy(BufferTitle, _T("TskDis"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B74
		case LK_TASK_DISTCOV:
			if ( (ActiveWayPoint >=1) && ( ValidTaskPoint(ActiveWayPoint) )) {
				value = DISTANCEMODIFY*CALCULATED_INFO.TaskDistanceCovered;
				valid=true;
				sprintf(text,"%.0f",value); // l o f?? TODO CHECK
			} else {
				strcpy(text,NULLLONG);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
				wsprintf(BufferTitle, TEXT("TskCov"),text);
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B79
		case LK_AIRSPACEDIST:
			if (lktitle)
				wsprintf(BufferTitle, TEXT("Arspace"),text);
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if (NearestAirspaceHDist >0) {
				value = DISTANCEMODIFY*NearestAirspaceHDist;
				sprintf(text,"%1.1f",value);
				wsprintf(BufferValue, TEXT("%S"),text);
				valid = true;
			} else {
				valid=false;
				wsprintf(BufferValue, TEXT(NULLMEDIUM),text);
				value = -1;
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			break;

		// B66
		case LK_FIN_GR:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("TskReqE"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					// the ValidFinish() seem to return FALSE when is actually valid.
					// In any case we do not use it for the vanilla GR
					value = CALCULATED_INFO.GRFinish;
					if (value <1 || value >=ALTERNATE_MAXVALIDGR )
						strcpy(text,NULLMEDIUM);
					else {
						if (value >= 100) sprintf(text,"%.0lf",value);
							else sprintf(text,"%.1lf",value);
						valid=true;
					}
					wsprintf(BufferValue, TEXT("%S"),text);
				}
			}
			break;


		// B19
		case LK_FIN_LD:
			wsprintf(BufferValue,_T(NULLLONG));
			//_stprintf(BufferUnit,TEXT(""));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("OLDfLD"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					if (CALCULATED_INFO.ValidFinish) {
						value = 0;
					} else {
						value = CALCULATED_INFO.LDFinish;
					}
					if (value <1 || value >=ALTERNATE_MAXVALIDGR )
						strcpy(text,NULLMEDIUM);
					else {
						valid=true;
						if (value >= 100) sprintf(text,"%.0lf",value);
							else sprintf(text,"%.1lf",value);
					}
					wsprintf(BufferValue, TEXT("%S"),text);
				}
			}
			break;

		// B38
		case LK_NEXT_LD:
			wsprintf(BufferValue,_T(NULLMEDIUM)); // 091221
			if (lktitle)
				_stprintf(BufferTitle, TEXT("OLDNexLD"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value = CALCULATED_INFO.LDNext;
					if (value <1 || value >=ALTERNATE_MAXVALIDGR )
						strcpy(text,NULLMEDIUM);
					else {
						valid=true;
						if (value >= 100) sprintf(text,"%.0lf",value);
							else sprintf(text,"%.1lf",value);
					}
					wsprintf(BufferValue, TEXT("%S"),text);
				}
			}
			break;

		// B53
		case LK_LD_VARIO:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			//_stprintf(BufferUnit,TEXT(""));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if (GPS_INFO.AirspeedAvailable && GPS_INFO.VarioAvailable) {
				value = CALCULATED_INFO.LDvario;
				if (value <1 || value >=ALTERNATE_MAXVALIDGR )
					strcpy(text,NULLMEDIUM);
				else {
					valid=true;
					if (value >= 100) sprintf(text,"%.0lf",value);
						else sprintf(text,"%.1lf",value);
				}
				wsprintf(BufferValue, TEXT("%S"),text);
			}
			break;

		// B64
		case LK_VARIO_DIST:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			if ( ActiveWayPoint >=1) {
				if ( ValidTaskPoint(ActiveWayPoint) ) {
					value = LIFTMODIFY*CALCULATED_INFO.DistanceVario;
					_stprintf(BufferValue,varformat,value);
					valid=true;
				}
			}
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			wsprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
			break;


		// B59
		case LK_SPEEDTASK_INST:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("TskSpI"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			value=0;
			if ( ActiveWayPoint >=1) {
				if ( ValidTaskPoint(ActiveWayPoint) ) {
					value = TASKSPEEDMODIFY*CALCULATED_INFO.TaskSpeedInstantaneous;
					if (value<=0||value>999) value=0; else valid=true;
					sprintf(text,"%d",(int)value);
					wsprintf(BufferValue, TEXT("%S"),text);
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetTaskSpeedName()));
			break;

		// B61
		case LK_SPEEDTASK_ACH:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("TskSp"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			value=0;
			if ( ActiveWayPoint >=1) {
				if ( ValidTaskPoint(ActiveWayPoint) ) {
					value = TASKSPEEDMODIFY*CALCULATED_INFO.TaskSpeedAchieved;
					if (value<0||value>999) value=0; else valid=true;
					sprintf(text,"%d",(int)value);
					wsprintf(BufferValue, TEXT("%S"),text);
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetTaskSpeedName()));
			break;

		// B17
		case LK_SPEEDTASK_AVG:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("TskSpAv"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			value=0;
			if ( ActiveWayPoint >=1) {
				if ( ValidTaskPoint(ActiveWayPoint) ) {
					value = TASKSPEEDMODIFY*CALCULATED_INFO.TaskSpeed;
					if (value<=0||value>999) value=0; else valid=true;
					sprintf(text,"%d",(int)value);
					wsprintf(BufferValue, TEXT("%S"),text);
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetTaskSpeedName()));
			break;


		// B132 Final arrival with MC 0 , no totaly energy.
		case LK_FIN_ALTDIFF0:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("TskArr0"));
			else
				_stprintf(BufferTitle, TEXT("TskArrMc0"));
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value=ALTITUDEMODIFY*DerivedDrawInfo.TaskAltitudeDifference0;
					if ( value > ALTDIFFLIMIT ) {
						valid=true;
						_stprintf(BufferValue,TEXT("%+1.0f"), value);
					}
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B41	091006 using new task ete 
		case LK_FIN_ETE:
		// B133  091222 using old ETE corrected now
		case LK_LKFIN_ETE:
			wsprintf(BufferValue,_T(NULLTIME)); // 091222
			if (lktitle)
				_stprintf(BufferTitle, TEXT("TskETE"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[LK_FIN_ETE].Title );

			if ( ValidTaskPoint(ActiveWayPoint) ) { // 091222
				if (CALCULATED_INFO.LKTaskETE > 0) { 
					valid=true;
					if ( Units::TimeToTextDown(BufferValue, (int)CALCULATED_INFO.LKTaskETE))  // 091112
						wsprintf(BufferUnit, TEXT("h"));
					else
						wsprintf(BufferUnit, TEXT("m"));
				} else {
					index = Task[ActiveWayPoint].Index;
					if ( (WayPointCalc[index].NextETE > 0) && !ValidTaskPoint(1) ) {
						valid=true;
						if (Units::TimeToTextDown(BufferValue, (int)WayPointCalc[index].NextETE))
                                                	wsprintf(BufferUnit, TEXT("h"));
                                        	else
                                        	        wsprintf(BufferUnit, TEXT("m"));
					} else
						wsprintf(BufferValue, TEXT(NULLTIME));
				}
			}
			// wsprintf(BufferUnit, TEXT("h")); 091112 REMOVE
			break;

		// B42
		case LK_NEXT_ETE:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("NexETE"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			index = Task[ActiveWayPoint].Index;
			if ( (ValidTaskPoint(ActiveWayPoint) != false) && (WayPointCalc[index].NextETE < 0.9*ERROR_TIME)) {

				if (WayPointCalc[index].NextETE > 0) {
					valid=true;
					if (Units::TimeToTextDown(BufferValue, (int)WayPointCalc[index].NextETE)) // 091112
						wsprintf(BufferUnit, TEXT("h"));
					else
						wsprintf(BufferUnit, TEXT("m"));
				} else
					wsprintf(BufferValue, TEXT(NULLTIME));
			}
			// wsprintf(BufferUnit, TEXT("h")); 091112
			break;


		// B45
		case LK_FIN_ETA:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("TskETA"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if ( (ValidTaskPoint(ActiveWayPoint) != false) && (CALCULATED_INFO.TaskTimeToGo< 0.9*ERROR_TIME)) {
				if (CALCULATED_INFO.TaskTimeToGo > 0) {
					valid=true;
					Units::TimeToText(BufferValue, (int)CALCULATED_INFO.TaskTimeToGo+DetectCurrentTime());
				} else
					wsprintf(BufferValue, TEXT(NULLTIME));
			}
			wsprintf(BufferUnit, TEXT("h"));
			break;


		// B46
		case LK_NEXT_ETA:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("NexETA"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if ( (ValidTaskPoint(ActiveWayPoint) != false) && (CALCULATED_INFO.LegTimeToGo< 0.9*ERROR_TIME)) {
				if (CALCULATED_INFO.LegTimeToGo > 0) {
					valid=true;
					Units::TimeToText(BufferValue, (int)CALCULATED_INFO.LegTimeToGo+DetectCurrentTime());
				} else
					wsprintf(BufferValue, TEXT(NULLTIME));
			}
			wsprintf(BufferUnit, TEXT("h"));
			break;


		// B36		TmFly
		case LK_TIMEFLIGHT:
			wsprintf(BufferValue,_T(NULLTIME));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("FlyTm"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if (CALCULATED_INFO.FlightTime > 0) {
				valid=true;
				Units::TimeToText(BufferValue, (int)CALCULATED_INFO.FlightTime);
			} else {
				wsprintf(BufferValue, TEXT(NULLTIME));
			}
			wsprintf(BufferUnit, TEXT("h"));
			break;

		// B09		Last thermal time
		case LK_TL_TIME:
			wsprintf(BufferValue,_T(NULLTIME));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("TL.Time"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if (CALCULATED_INFO.LastThermalTime > 0) {
				valid=true;
				Units::TimeToText(BufferValue, (int)CALCULATED_INFO.LastThermalTime);
			} else {
				wsprintf(BufferValue, TEXT(NULLTIME));
			}
			wsprintf(BufferUnit, TEXT("h"));
			break;


		// B27
		case LK_AA_TIME:
			wsprintf(BufferValue,_T(NULLTIME));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

#if (0)
			double dd;
			if (AATEnabled && ValidTaskPoint(ActiveWayPoint)) {
				dd = CALCULATED_INFO.TaskTimeToGo;
				if ((CALCULATED_INFO.TaskStartTime>0.0) && (CALCULATED_INFO.Flying) &&(ActiveWayPoint>0)) {
					dd += GPS_INFO.Time-CALCULATED_INFO.TaskStartTime;
				}
				dd= max(0,min(24.0*3600.0,dd))-AATTaskLength*60;
				if (dd<0) {
					status = 1; // red
				} else {
					if (CALCULATED_INFO.TaskTimeToGoTurningNow > (AATTaskLength+5)*60) {
						status = 2; // blue
					} else {
						status = 0;  // black
					}
				}
			} else {
				dd = 0;
				status = 0; // black
			}
#endif
			if (ValidTaskPoint(ActiveWayPoint) && AATEnabled && (CALCULATED_INFO.AATTimeToGo< 0.9*ERROR_TIME)) {

				Units::TimeToText(BufferValue, (int)CALCULATED_INFO.AATTimeToGo);
				valid=true;
			}
			wsprintf(BufferUnit,_T("h"));
			break;

		// B37
		case LK_GLOAD:
			//wsprintf(BufferValue,_T(NULLMEDIUM)); 100302 obs
			//wsprintf(BufferUnit,_T("g")); 100302 obs
			// _stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title ); 100302 obs
			if ( GPS_INFO.AccelerationAvailable) { 
				value=GPS_INFO.Gload;
				_stprintf(BufferValue,TEXT("%+.1f"), value);
				valid=true;
				_tcscpy(BufferTitle,_T("G"));
			} else {
				value=CALCULATED_INFO.Gload;
				_stprintf(BufferValue,TEXT("%+.1f"), value);
				valid=true;
				_tcscpy(BufferTitle,_T("eG"));
			}
			break;

		// B65 FIXED 100125
		case LK_BATTERY:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

#if (WINDOWSPC<1)
	#ifndef GNAV
			value = PDABatteryPercent;
                	if (value<1||value>100)
				_stprintf(BufferValue,_T("---"));
                	else {
				if (PDABatteryFlag==BATTERY_FLAG_CHARGING || PDABatteryStatus==AC_LINE_ONLINE) {
					_stprintf(BufferValue,TEXT("%2.0f%%C"), value);	 // 100228
				} else {
					_stprintf(BufferValue,TEXT("%2.0f%%D"), value);  // 100228
				}

				valid = true;
			}
	#else
			value = GPS_INFO.SupplyBatteryVoltage;
			if (value>0.0) {
				_stprintf(BufferValue,TEXT("%2.1fV"), value);
				valid = true;
			} else {
				valid = false;
			}
	#endif
#endif
			break;


		// B62
		case LK_AA_DELTATIME:
			wsprintf(BufferValue,_T(NULLTIME));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			wsprintf(BufferUnit,_T("h"));
			// TODO This is in the wrong place, should be moved to calc thread! 090916
			double dd;
			if (AATEnabled && ValidTaskPoint(ActiveWayPoint)) {
				dd = CALCULATED_INFO.TaskTimeToGo;
				if ((CALCULATED_INFO.TaskStartTime>0.0) && (CALCULATED_INFO.Flying) &&(ActiveWayPoint>0)) {
					dd += GPS_INFO.Time-CALCULATED_INFO.TaskStartTime;
				}
				dd= max(0,min(24.0*3600.0,dd))-AATTaskLength*60;
#if (0)
				if (dd<0) {
					status = 1; // red
				} else {
					if (CALCULATED_INFO.TaskTimeToGoTurningNow > (AATTaskLength+5)*60) {
						status = 2; // blue
					} else {
						status = 0;  // black
					}
				}
#endif
				if (dd < (0.9*ERROR_TIME)) {
					valid=true;
					Units::TimeToText(BufferValue, (int)dd);
				}
			}
			break;

#if 0
		// B133  091222 using old ETE corrected now
		case LK_LKFIN_ETE:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("TskETE"));
			else
				_stprintf(BufferTitle, TEXT("TskETE"));

			if ( (ValidTaskPoint(ActiveWayPoint) != false) && (CALCULATED_INFO.LKTaskETE< 0.9*ERROR_TIME)) {
				if (CALCULATED_INFO.LKTaskETE > 0) {
					Units::TimeToText(BufferValue, (int)CALCULATED_INFO.LKTaskETE);
					valid=true;
				} else
					wsprintf(BufferValue, TEXT(NULLTIME));
			}
			wsprintf(BufferUnit, TEXT("h"));
			break;
#endif


		// B134
		// Using MC=0!  total energy disabled
		case LK_NEXT_ALTDIFF0:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("ArrMc0"));
			else
				_stprintf(BufferTitle, TEXT("ArrMc0"));
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value=ALTITUDEMODIFY*DerivedDrawInfo.NextAltitudeDifference0;
					if ( value > ALTDIFFLIMIT ) {
						valid=true;
						_stprintf(BufferValue,TEXT("%+1.0f"), value);
					}
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B67
		case LK_ALTERN1_GR:
		// B68
		case LK_ALTERN2_GR:
		// B69
		case LK_BESTALTERN_GR:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			if (lktitle) {
				if (lkindex==LK_BESTALTERN_GR) 
					_stprintf(BufferTitle, TEXT("Batn"));
				else
					_stprintf(BufferTitle, TEXT("Atn%d.E"), lkindex-LK_ALTERNATESGR+1);
			} else {
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			}
			switch(lkindex) {
				case LK_ALTERN1_GR:
					index=Alternate1;
					break;
				case LK_ALTERN2_GR:
					index=Alternate2;
					break;
				case LK_BESTALTERN_GR:
					index=BestAlternate;
					break;
				default:
					index=0;
					break;
			}

			if(ValidWayPoint(index))
			{
				if ( DisplayTextType == DISPLAYFIRSTTHREE)
				{
					 _tcsncpy(BufferTitle,WayPointList[index].Name,3);
					BufferTitle[3] = '\0';
				}
				else if( DisplayTextType == DISPLAYNUMBER) {
					_stprintf(BufferTitle,TEXT("%d"), WayPointList[index].Number );
				} else {
					_tcsncpy(BufferTitle,WayPointList[index].Name, 12);
					// BufferTitle[(sizeof(Text)/sizeof(TCHAR))-1] = '\0';
					if (lktitle)
						BufferTitle[12] = '\0'; // FIX TUNING
					else
						BufferTitle[8] = '\0';  // FIX TUNING
				}
			}
			// it would be time to use Alternate[] ..
			switch (lkindex) {
				case LK_ALTERN1_GR:
					if ( ValidWayPoint(Alternate1) ) value=WayPointCalc[Alternate1].GR;
					else value=INVALID_GR;
					break;
				case LK_ALTERN2_GR:
					if ( ValidWayPoint(Alternate2) ) value=WayPointCalc[Alternate2].GR;
					else value=INVALID_GR;
					break;
				case LK_BESTALTERN_GR:
					if ( ValidWayPoint(BestAlternate) ) value=WayPointCalc[BestAlternate].GR;
					else value=INVALID_GR;
					break;
				default:
					value = 1;
					break;
			}
			if (value <1 || value >=ALTERNATE_MAXVALIDGR ) {
				strcpy(text,NULLMEDIUM);
				valid=false;
			} else {
				if (value >= 100) sprintf(text,"%.0lf",value);
					else sprintf(text,"%.1lf",value);
				valid=true;
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("")); // 091227 BUGFIX
			break;

		// B75
		case LK_ALTERN1_ARRIV:
		// B76
		case LK_ALTERN2_ARRIV:
		// B77
		case LK_BESTALTERN_ARRIV:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			if (lktitle) {
				if (lkindex==LK_BESTALTERN_ARRIV) 
					_stprintf(BufferTitle, TEXT("BatnArr"));
				else
					_stprintf(BufferTitle, TEXT("Atn%dArr"), lkindex-LK_ALTERNATESARRIV+1);
			} else {
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			}
			switch(lkindex) {
				case LK_ALTERN1_ARRIV:
					index=Alternate1;
					break;
				case LK_ALTERN2_ARRIV:
					index=Alternate2;
					break;
				case LK_BESTALTERN_ARRIV:
					index=BestAlternate;
					break;
				default:
					index=0;
					break;
			}

			if(ValidWayPoint(index))
			{
				if ( DisplayTextType == DISPLAYFIRSTTHREE)
				{
					 _tcsncpy(BufferTitle,WayPointList[index].Name,3);
					BufferTitle[3] = '\0';
				}
				else if( DisplayTextType == DISPLAYNUMBER) {
					_stprintf(BufferTitle,TEXT("%d"), WayPointList[index].Number );
				} else {
					_tcsncpy(BufferTitle,WayPointList[index].Name, 12);
					// BufferTitle[(sizeof(Text)/sizeof(TCHAR))-1] = '\0';
					if (lktitle)
						BufferTitle[12] = '\0'; // FIX TUNING
					else
						BufferTitle[8] = '\0';  // FIX TUNING
				}
			}
			switch (lkindex) {
				case LK_ALTERN1_ARRIV:
					if ( ValidWayPoint(Alternate1) ) value=ALTITUDEMODIFY*WayPointCalc[Alternate1].AltArriv[AltArrivMode];
					else value=INVALID_DIFF;
					break;
				case LK_ALTERN2_ARRIV:
					if ( ValidWayPoint(Alternate2) ) value=ALTITUDEMODIFY*WayPointCalc[Alternate2].AltArriv[AltArrivMode];
					else value=INVALID_DIFF;
					break;
				case LK_BESTALTERN_ARRIV:
					if ( ValidWayPoint(BestAlternate) ) value=ALTITUDEMODIFY*WayPointCalc[BestAlternate].AltArriv[AltArrivMode];
					else value=INVALID_DIFF;
					break;
				default:
					value = INVALID_DIFF;
					break;
			}
			if (value <= ALTDIFFLIMIT ) {
				strcpy(text,NULLLONG);
				valid=false;
			} else { // 091221
				if ( (value>-1 && value<=0) || (value>=0 && value<1))
					sprintf(text,"0");
				else {
					sprintf(text,"%+.0f",value);
				}
				valid=true;
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B80
		case LK_EXTBATTBANK:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			ivalue=GPS_INFO.ExtBatt_Bank;
			if (ivalue>0) {
				_stprintf(BufferValue,TEXT("%d"), ivalue); // 091101
				valid = true;
			} else {
				valid = false;
			}
			break;

		// B81
		// B82
		case LK_EXTBATT1VOLT:
		case LK_EXTBATT2VOLT:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			value = (lkindex==LK_EXTBATT1VOLT?GPS_INFO.ExtBatt1_Voltage:GPS_INFO.ExtBatt2_Voltage);
			if (value>0.0) {
				_stprintf(BufferValue,TEXT("%0.2fv"), value);
				valid = true;
			} else {
				valid = false;
			}
			break;

		// B48 091216  OAT Outside Air Temperature
		case LK_OAT:
			value=GPS_INFO.OutsideAirTemperature;
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if (value<1||value>100) {
				wsprintf(BufferValue, TEXT("---"));
			} else {
                	        sprintf(text,"%.0lf",value);
                	        wsprintf(BufferValue, TEXT("%S%S"),text,_T(DEG));
                	}
			break;

		// B84  100126
		case LK_AQNH:
			if (lktitle)
                        	_stprintf(BufferTitle, TEXT("aAlt"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if (ALTITUDEMODIFY==TOMETER)
				value=TOFEET*DerivedDrawInfo.NavAltitude;
			else
				value=TOMETER*DerivedDrawInfo.NavAltitude;
			valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetInvAltitudeName()));
			break;

		// B85  100126
		case LK_AALTAGL:
			if (lktitle)
                        	_stprintf(BufferTitle, TEXT("aHAGL"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if (!CALCULATED_INFO.TerrainValid) { //@ 101013
				wsprintf(BufferValue, TEXT(NULLLONG));
				wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
				valid=false;
				break;
			}
			if (ALTITUDEMODIFY==TOMETER)
				value=TOFEET*DerivedDrawInfo.AltitudeAGL;
			else
				value=TOMETER*DerivedDrawInfo.AltitudeAGL;
			valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetInvAltitudeName()));
			break;

		// B136
		case LK_TARGET_DIST:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
			} else {
	                        if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
				} else {
					// get values
	                        	value=DISTANCEMODIFY*LKTraffic[LKTargetIndex].Distance;
					if (value>99) {
						strcpy(text,NULLMEDIUM);
					} else {
						valid=true;
						sprintf(text,"%.1f",value);
					}
				}
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			wsprintf(BufferTitle, TEXT("Dist"),text);
			break;

		// B137
		case LK_TARGET_TO:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
			} else {
	                        if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
				} else {
						value = LKTraffic[LKTargetIndex].Bearing -  GPS_INFO.TrackBearing;
						valid=true;
						if (value < -180.0)
							value += 360.0;
						else
							if (value > 180.0)
								value -= 360.0;
						if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0fÂ°Â»"), value);
						else if (value < -1)
						_stprintf(BufferValue, TEXT("Â«%2.0fÂ°"), -value);
						else
							_tcscpy(BufferValue, TEXT("Â«Â»"));
				}
			}
			wsprintf(BufferUnit, TEXT(""));
			wsprintf(BufferTitle, TEXT("To"),text);
			break;

		// B138
		case LK_TARGET_BEARING:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
			} else {
	                        if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
				} else {
						value = LKTraffic[LKTargetIndex].Bearing;
						valid=true;
						if (value == 360)
							_stprintf(BufferValue, TEXT("0Â°"));
						else 
							_stprintf(BufferValue, TEXT("%2.0fÂ°"), value);
				}
			}
			wsprintf(BufferUnit, TEXT(""));
			wsprintf(BufferTitle, TEXT("Brg"),text);
			break;

		// B139
		case LK_TARGET_SPEED:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
			} else {
	                        if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
				} else {
					value=SPEEDMODIFY*GPS_INFO.FLARM_Traffic[LKTargetIndex].Speed;
					if (value<0||value>9999) value=0; else valid=true;
					sprintf(text,"%d",(int)value);
				}
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			wsprintf(BufferTitle, TEXT("GS"));
			break;

		// B140
		case LK_TARGET_ALT:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
			} else {
	                        if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
				} else {
					value=ALTITUDEMODIFY*GPS_INFO.FLARM_Traffic[LKTargetIndex].Altitude;
					valid=true;
					sprintf(text,"%d",(int)value);
				}
			}
                       	_stprintf(BufferTitle, TEXT("Alt"));
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B141
		// DO NOT USE RELATIVE ALTITUDE: when not real time, it won't change in respect to our position!!!
		// This is negative when target is below us because it represent a remote position
		case LK_TARGET_ALTDIFF:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
			} else {
	                        if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
				} else {
					value=ALTITUDEMODIFY*(DerivedDrawInfo.NavAltitude-GPS_INFO.FLARM_Traffic[LKTargetIndex].Altitude)*-1;
					valid=true;
					sprintf(text,"%+d",(int)value);
				}
			}
                       	_stprintf(BufferTitle, TEXT("RelAlt"));
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B142
		case LK_TARGET_VARIO:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
					_tcscpy(BufferUnit, _T(""));
			} else {
	                        if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
					_tcscpy(BufferUnit, _T(""));
				} else {
					value = LIFTMODIFY*GPS_INFO.FLARM_Traffic[LKTargetIndex].ClimbRate;
					valid=true;
					_stprintf(BufferValue,varformat,value);
					wsprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
				}
			}
			_tcscpy(BufferTitle, TEXT("Var"));
			break;

		// B143
		case LK_TARGET_AVGVARIO:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
					_tcscpy(BufferUnit, _T(""));
			} else {
	                        if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
					_tcscpy(BufferUnit, _T(""));
				} else {
					value = LIFTMODIFY*GPS_INFO.FLARM_Traffic[LKTargetIndex].Average30s;
					valid=true;
					_stprintf(BufferValue,varformat,value);
					wsprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
				}
			}
			_tcscpy(BufferTitle, TEXT("Var30"));
			break;

		// B144
		case LK_TARGET_ALTARRIV:
			wsprintf(BufferValue,_T(NULLLONG));
			_stprintf(BufferTitle, TEXT("Arr"));
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
			} else {
	                        if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
				} else {

					value=ALTITUDEMODIFY*LKTraffic[LKTargetIndex].AltArriv;
					if ( value > ALTDIFFLIMIT ) {
						valid=true;
						_stprintf(BufferValue,TEXT("%+1.0f"), value);
					} else {
						strcpy(text,NULLMEDIUM);
						wsprintf(BufferValue, TEXT("%S"),text);
					}
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B145
		case LK_TARGET_GR:
			wsprintf(BufferValue,_T(NULLLONG));
			_stprintf(BufferTitle, TEXT("ReqE"));
			_tcscpy(BufferUnit,_T(""));
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
			} else {
	                        if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
				} else {
					value=LKTraffic[LKTargetIndex].GR;
					if (value <1 || value >=ALTERNATE_MAXVALIDGR )
						strcpy(text,NULLMEDIUM);
					else {
						if (value >= 100) sprintf(text,"%.0lf",value);
							else sprintf(text,"%.1lf",value);
						valid=true;
					}
				}
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			break;

		// B146
		case LK_TARGET_EIAS:
			_stprintf(BufferTitle, TEXT("eIAS"));
			_tcscpy(BufferUnit,_T(""));
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
			} else {
				value=SPEEDMODIFY*LKTraffic[LKTargetIndex].EIAS;
				if (value<0||value>999) value=0; else valid=true;
				sprintf(text,"%d",(int)value);
				wsprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			break;

		case LK_DUMMY:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("Dummy"));
			else
				_stprintf(BufferTitle, TEXT("Dummy"));

			wsprintf(BufferUnit, TEXT("."));
			break;

		case LK_EMPTY:
			wsprintf(BufferValue, TEXT(""));
			//wsprintf(BufferUnit, TEXT(""));
			wsprintf(BufferTitle, TEXT(""));
			break;
		case LK_ERROR:
			// let it be shown entirely to understand the problem
			valid=true;
			wsprintf(BufferValue, TEXT("000"));
			wsprintf(BufferUnit, TEXT("e"));
			wsprintf(BufferTitle, TEXT("Err"));
			break;
		default:
			valid=false;
			wsprintf(BufferValue, TEXT(NULLMEDIUM));
			wsprintf(BufferUnit, TEXT("."));
			if ( lkindex >=NUMSELECTSTRINGS || lkindex <1 ) 
				wsprintf(BufferTitle, TEXT("BadErr"));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;
	}

  return valid;
}

// simple format distance value for a given index. BufferTitle always NULLed
// wpindex is a WayPointList index
// wpvirtual true means virtual waypoint, and relative special checks

void MapWindow::LKFormatDist(const int wpindex, const bool wpvirtual, TCHAR *BufferValue, TCHAR *BufferUnit) {

  static int	index;
  static double value;
  static char	text[LKSIZEBUFFERVALUE];

  index=-1;

  if (wpvirtual) {
	if ( ValidResWayPoint(wpindex) ) index = wpindex;
  } else {
	if ( ValidWayPoint(wpindex) ) index = wpindex;
  }
  if (index>=0) {
	value=WayPointCalc[index].Distance*DISTANCEMODIFY;
	if (value>99)
		sprintf(text,"%.0f",value);
	else
		sprintf(text,"%.1f",value);
  } else {
	strcpy(text,NULLMEDIUM);
  }
  wsprintf(BufferValue, TEXT("%S"),text);
  wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
  return;
}

void MapWindow::LKFormatBrgDiff(const int wpindex, const bool wpvirtual, TCHAR *BufferValue, TCHAR *BufferUnit) {

  static int	index;
  static double value;

  index=-1;

  if (wpvirtual) {
	if ( ValidResWayPoint(wpindex) ) index = wpindex;
  } else {
	if ( ValidWayPoint(wpindex) ) index = wpindex;
  }
  _tcscpy(BufferValue,_T(NULLMEDIUM)); 
  _tcscpy(BufferUnit,_T(""));
  if (index>=0) {
	if (DisplayMode != dmCircling) {
		value = WayPointCalc[index].Bearing -  GPS_INFO.TrackBearing;
		if (value < -180.0)
			value += 360.0;
		else
			if (value > 180.0)
				value -= 360.0;
#ifndef __MINGW32__
		if (value > 1)
			_stprintf(BufferValue, TEXT("%2.0f°»"), value);
		else if (value < -1)
			_stprintf(BufferValue, TEXT("«%2.0f°"), -value);
		else
			_tcscpy(BufferValue, TEXT("«»"));
#else
		if (value > 1)
			_stprintf(BufferValue, TEXT("%2.0fÂ°Â»"), value);
		else if (value < -1)
			_stprintf(BufferValue, TEXT("Â«%2.0fÂ°"), -value);
		else
			_tcscpy(BufferValue, TEXT("Â«Â»"));
	}
#endif
  }
}


void MapWindow::LKFormatGR(const int wpindex, const bool wpvirtual, TCHAR *BufferValue, TCHAR *BufferUnit) {

  static int	index;
  static double value;
  static char	text[LKSIZEBUFFERVALUE];

  index=-1;

  if (wpvirtual) {
	if ( ValidResWayPoint(wpindex) ) index = wpindex;
  } else {
	if ( ValidWayPoint(wpindex) ) index = wpindex;
  }
  _tcscpy(BufferValue,_T(NULLMEDIUM)); 
  _tcscpy(BufferUnit,_T(""));

  if (index>=0) {
	value=WayPointCalc[index].GR;
  } else {
	value=INVALID_GR;
  }

  if (value >=1 && value <MAXEFFICIENCYSHOW ) {
	if (value >= 100)
		sprintf(text,"%.0lf",value);
	else
		sprintf(text,"%.1lf",value);

	wsprintf(BufferValue, TEXT("%S"),text);
  }
}

void MapWindow::LKFormatAltDiff(const int wpindex, const bool wpvirtual, TCHAR *BufferValue, TCHAR *BufferUnit) {

  static int	index;
  static double value;
  static char	text[LKSIZEBUFFERVALUE];

  index=-1;

  if (wpvirtual) {
	if ( ValidResWayPoint(wpindex) ) index = wpindex;
  } else {
	if ( ValidWayPoint(wpindex) ) index = wpindex;
  }
  _tcscpy(BufferValue,_T(NULLMEDIUM)); 
  wsprintf(BufferUnit, _T("%s"),(Units::GetAltitudeName()));

  if (index>=0) {
	value=WayPointCalc[index].AltArriv[AltArrivMode]*ALTITUDEMODIFY;
  } else {
	value=INVALID_DIFF;
  }

  if (value > ALTDIFFLIMIT ) {
	if ( value>-1 && value<1 )
		sprintf(text,"0");
	else
		sprintf(text,"%+.0f",value);

	wsprintf(BufferValue, TEXT("%S"),text);
  }
}
