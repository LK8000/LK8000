/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: LKProcess.cpp,v 1.8 2010/12/11 19:32:14 root Exp root $
*/

#include "externs.h"
#include "McReady.h"
#include "InputEvents.h"
#include "Logger.h"
#include "LKProcess.h"
#include "DoInits.h"
#include "OS/Memory.h"
#include "Calc/Vario.h"
#include "LKInterface.h"
#include "ContestMgr.h"
#include "Library/TimeFunctions.h"
#include "Baro.h"
#include "OS/CpuLoad.h"

// #define NULLSHORT	"--" 
#define NULLMEDIUM	"---"
#define NULLLONG	"---"
#define NULLTIME	"--:--"

#ifdef UNICODE
// utf16 Infinity symbol
constexpr TCHAR infinity[] = { 0x221E, 0x00 };
#else
// utf8 Infinity symbol
constexpr TCHAR infinity[] = { '\xE2', '\x88', '\x9E', '\0' };
#endif


/**
* return -1 if NextETE is Invalid.
*/
double GetGlNextETE(const DERIVED_INFO& info) {
    double value = -1;
    if(ISPARAGLIDER) {
        LockTaskData();
        if (ValidTaskPointFast(ActiveTaskPoint)) {
            int index = (ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute()) ? RESWP_OPTIMIZED: Task[ActiveTaskPoint].Index;
            value = WayPointCalc[index].NextETE;
        }
        UnlockTaskData();
    } else  if (ISCAR || ISGAAIRCRAFT) {
        LockTaskData();
        if (ValidTaskPointFast(ActiveTaskPoint)) {
            value = WayPointCalc[TASKINDEX].NextETE;
        }
        UnlockTaskData();
    } else {
        value = info.LegTimeToGo;
    }

    if((value <= 0) || value >= (0.9*ERROR_TIME)) {
        value = -1;
    }
    return value;
}

double GetAvgNextETE(const DERIVED_INFO &info) {
    double value = -1;
    if(ISPARAGLIDER) {
        LockTaskData();
        if (ValidTaskPointFast(ActiveTaskPoint)) {
            int index = (ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute()) ? RESWP_OPTIMIZED: Task[ActiveTaskPoint].Index;
            value = WayPointCalc[index].NextAvrETE;
        }
        UnlockTaskData();
    } else  if (ISCAR || ISGAAIRCRAFT) {
        LockTaskData();
        if (ValidTaskPointFast(ActiveTaskPoint)) {
            value = WayPointCalc[TASKINDEX].NextAvrETE;
        }
        UnlockTaskData();
    } else {
        value = info.LegTimeToGo;
    }

    if((value <= 0) || value >= (0.9*ERROR_TIME)) {
        value = -1;
    }
    return value;
}





static bool TurnpointQnhArrival(int TpIndex, double &value, TCHAR *BufferValue, TCHAR *BufferUnit) {
	bool valid = false;

	if(ValidWayPoint(TpIndex)) {
		value = WayPointCalc[TpIndex].AltArriv[AltArrivMode]; // Arrival Height
		value += WayPointList[TpIndex].Altitude; // add altitude of waypoint/target
		if(IsSafetyAltitudeInUse(TpIndex)) {
			value += (SAFETYALTITUDEARRIVAL / 10); // add safety altitude if in use for that target
		}
		// even though this is a valid waypoint, if arrival below waypoint altitue, 
		// then valid = false so that set Amber color to get attention of user
		valid = (value >= WayPointList[TpIndex].Altitude); 
		value = Units::ToAltitude(value);
		_stprintf(BufferValue, TEXT("%d"), int(value));
	} else {
		// could be e.g. if display Alternate 1 QNH arr, but no Alternate 1 is defined
		_stprintf(BufferValue,_T(NULLMEDIUM));
		value = 0;
	}
	_stprintf(BufferUnit, TEXT("%s"), (Units::GetAltitudeName()));
	return valid;
}

void MapWindow::LKgetOLCBmp(CContestMgr::TType Type,DrawBmp_t *Bmp,TCHAR *BufferValue){
	switch(Type){
		case CContestMgr::TYPE_XC_FREE_TRIANGLE:
			if (BufferValue != NULL) _tcscpy(BufferValue, MsgToken<2497>()); //_@M002497_ "FT"
			if (Bmp != NULL) *Bmp = BmpFT;
			break;
		case CContestMgr::TYPE_XC_FAI_TRIANGLE:
			if (BufferValue != NULL) _tcscpy(BufferValue, MsgToken<2498>()); //_@M002498_ "FAI"
			if (Bmp != NULL) *Bmp = BmpFAI;
			break;
		default:
			if (BufferValue != NULL) _tcscpy(BufferValue, MsgToken<2496>()); //_@M002496_ "FF"
			if (Bmp != NULL) *Bmp = BmpFF;
			break;
	}
}

//
// CAREFUL CAREFUL CAREFUL here:
// lkindex can be much over the DataOption size, because some values here are not
// an option for infobox. In this case CAREFUL not to use DataOption for them.
// It would result in possible bad crashes, because we are copying a string from 
// DataOption[].Title : if it does not exist, the random stuff been copied can 
// simply overflow the BufferValue capacity, resulting in CRASHES or errors!
//

// Returns true if value is valid, false if not
// lktitle is shorter and limited to 6 or 7 chars, good for navboxes
// Units are empty by default, and valid is false by default

//
// Keep this list sorted, so that the compiler can use a jump (branch) table 
//
bool MapWindow::LKFormatValue(const short lkindex, const bool lktitle, 
			TCHAR (&BufferValue)[LKSIZEBUFFERVALUE],
			TCHAR (&BufferUnit)[LKSIZEBUFFERUNIT],
			TCHAR (&BufferTitle)[LKSIZEBUFFERTITLE],
			DrawBmp_t *BmpValue, DrawBmp_t *BmpTitle) {

  int	index=-1;
  double value;
  int	ivalue;

   BOOL bFAI ;
   double fDist ;
   double fTogo ;
   bool  bShowWPname = true;
  // By default, invalid return value. Set it to true after assigning value in cases
  bool		valid=false;

  
  const TCHAR* varformat = (Units::GetVerticalSpeedUnit() == unFeetPerMinutes) ? TEXT("%+.0f") : TEXT("%+0.1f");


	lk::strcpy(BufferValue,_T(""));
	lk::strcpy(BufferUnit,_T(""));
	lk::strcpy(BufferTitle,_T(""));

        if ( lkindex<0 ) {
           TESTBENCH_DO_ONLY(20,StartupStore(_T("****** CRITICAL: LKFormatValue negative lkindex=%d ******%s"),lkindex,NEWLINE));
           return false;
        }

	switch(lkindex) {


		// B00
		case LK_HNAV:
			if (lktitle)
				// LKTOKEN  _@M1001_ = "Altitude QNH", _@M1002_ = "Alt", 
				lk::strcpy(BufferTitle, MsgToken<1002>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));
			value=Units::ToAltitude(DerivedDrawInfo.NavAltitude);
			valid=true;
			_stprintf(BufferValue, TEXT("%d"),(int)value);
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B01		AltAgl HAGL 100318
		case LK_HAGL:
			if (lktitle)
				// LKTOKEN  _@M1003_ = "Altitude AGL", _@M1004_ = "HAGL"
				lk::strcpy(BufferTitle, MsgToken<1004>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;

			if (!DerivedDrawInfo.TerrainValid) { 
				_stprintf(BufferValue, TEXT(NULLLONG));
				_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
				valid=false;
				break;
			}
			value=Units::ToAltitude(DerivedDrawInfo.AltitudeAGL);
			valid=true;
			_stprintf(BufferValue, TEXT("%d"),(int)value);
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B02 091221
		case LK_TC_30S:
			value=Units::ToVerticalSpeed(DerivedDrawInfo.Average30s);
			if (value==0)
				_stprintf(BufferValue, TEXT(NULLMEDIUM));
			else { 
				valid=true;
				if (value<20) _stprintf(BufferValue, TEXT("%+.1lf"),value);
					else _stprintf(BufferValue, TEXT("%+.0lf"),value);
			}
			_stprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
			if (lktitle)
				// LKTOKEN  _@M1005_ = "Thermal last 30 sec", _@M1006_ = "TC.30\""
				lk::strcpy(BufferTitle, MsgToken<1006>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;
		// B03
		case LK_BRG:
			_stprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1007_ = "Bearing", _@M1008_ = "Brg"
				lk::strcpy(BufferTitle, MsgToken<1008>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			if ( ValidTaskPoint(ActiveTaskPoint) != false ) {
				index = Task[ActiveTaskPoint].Index;
				if (index>=0) {
goto_bearing:
					// we could use only waypointbearing, but lets keep them separated anyway
					if (UseAATTarget())
						value=DerivedDrawInfo.WaypointBearing;
					else
						value = WayPointCalc[index].Bearing;

					valid=true;
					if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0f%s"), value, MsgToken<2179>());
					else if (value < -1)
						_stprintf(BufferValue, TEXT("%2.0f%s"), -value, MsgToken<2179>());
                    else
                        _stprintf(BufferValue, TEXT("0%s"), MsgToken<2179>());
				}
			}
			break;


		// B04
		case LK_LD_INST:
                        _stprintf(BufferValue,_T(NULLLONG));

			if (lktitle)
				// LKTOKEN  _@M1009_ = "Eff.last 20 sec", _@M1010_ = "E.20\""
				lk::strcpy(BufferTitle, MsgToken<1010>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;

                        if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) { 
			   if (DerivedDrawInfo.Flying)
				value=DerivedDrawInfo.LD;
			   else
				value=0;
			   if (value <-99 ||  value >=ALTERNATE_MAXVALIDGR ) {
				lk::strcpy(BufferValue, infinity);
				valid=true;
			   } else {
				if (value==0) _stprintf(BufferValue, TEXT(NULLMEDIUM));
				else {
					_stprintf(BufferValue, TEXT("%.1f"),value);
					valid=true;
				}
                           }
                        }
			break;


		// B05
		case LK_LD_CRUISE:
			if (lktitle)
				// LKTOKEN  _@M1011_ = "Eff.cruise last therm", _@M1012_ = "E.Cru"
				lk::strcpy(BufferTitle, MsgToken<1012>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			if (DerivedDrawInfo.Flying)
				value=DerivedDrawInfo.CruiseLD;
			else
				value=0;
			if (value <-99 ||  value >=ALTERNATE_MAXVALIDGR ) {
				lk::strcpy(BufferValue,infinity); 
				valid=true;
			} else
			if (value==0) _stprintf(BufferValue, TEXT(NULLMEDIUM));
			else {
				_stprintf(BufferValue, TEXT("%.1f"),value);
				valid=true;
			}
			break;
		// B06
		case LK_GNDSPEED:
			value=Units::ToHorizontalSpeed(DrawInfo.Speed);
			valid=true;
			if (value<0||value>9999) value=0; else valid=true;
			_stprintf(BufferValue, TEXT("%d"),(int)value);
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			if (lktitle)
				// LKTOKEN  _@M1013_ = "Speed ground", _@M1014_ = "GS"
				lk::strcpy(BufferTitle, MsgToken<1014>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;


		// B07  091221
		case LK_TL_AVG:
			value= Units::ToVerticalSpeed(DerivedDrawInfo.LastThermalAverage);
			if (value==0)
				_stprintf(BufferValue, TEXT(NULLMEDIUM));
			else { 
				valid=true;
				if (value<20) _stprintf(BufferValue, TEXT("%+.1lf"),value);
                else _stprintf(BufferValue, TEXT("%+.0lf"),value);
			}
			_stprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
			if (lktitle)
				// LKTOKEN  _@M1015_ = "Thermal Average Last", _@M1016_ = "TL.Avg"
				lk::strcpy(BufferTitle, MsgToken<1016>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;

		// B08 091216 091221
		case LK_TL_GAIN:
			value=Units::ToAltitude(DerivedDrawInfo.LastThermalGain);
			if (value==0)
				_stprintf(BufferValue, TEXT(NULLMEDIUM));
			else { 
				valid=true;
				_stprintf(BufferValue, TEXT("%+d"),(int)value);
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			if (lktitle)
				// LKTOKEN  _@M1017_ = "Thermal Gain Last", _@M1018_ = "TL.Gain"
				lk::strcpy(BufferTitle, MsgToken<1018>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;
		// B09		Last thermal time
		case LK_TL_TIME:
			_stprintf(BufferValue,_T(NULLTIME));
			if (lktitle)
				// LKTOKEN  _@M1019_ = "Thermal Time Last", _@M1020_ = "TL.Time"
				lk::strcpy(BufferTitle, MsgToken<1020>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;

			if (DerivedDrawInfo.LastThermalTime > 0) {
				valid=true;
				Units::TimeToText(BufferValue, (int)DerivedDrawInfo.LastThermalTime);
			} else {
				_stprintf(BufferValue, TEXT(NULLTIME));
			}
			_stprintf(BufferUnit, TEXT("h"));
			break;


		// B10
		case LK_MC:
			value = iround(Units::ToVerticalSpeed(MACCREADY*10))/10.0;
			valid=true;
			_stprintf(BufferValue, TEXT("%2.1lf"),value);
			if (!DerivedDrawInfo.AutoMacCready) {
				if (lktitle)
					// LKTOKEN  _@M1183_ = "ManMC"
					lk::strcpy(BufferTitle, MsgToken<1183>());
				else
					lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			} else {
				if (lktitle)
					// LKTOKEN  _@M1184_ = "AutMC"
					lk::strcpy(BufferTitle, MsgToken<1184>());
				else
					lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			}

			break;

		// B11
		case LK_NEXT_DIST:
			if ( ValidTaskPoint(ActiveTaskPoint) != false ) {
			   if (ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute()) {
				value=Units::ToDistance(WayPointCalc[RESWP_OPTIMIZED].Distance);
				valid=true;
				if (value>99)
					_stprintf(BufferValue, TEXT("%.0f"),value);
				else {
					if (ISPARAGLIDER) {
						if (value>10)
							_stprintf(BufferValue, TEXT("%.1f"),value);
						else 
							_stprintf(BufferValue, TEXT("%.2f"),value);
					} else {
						_stprintf(BufferValue, TEXT("%.1f"),value);
					}
				}
			   } else {
				index = Task[ActiveTaskPoint].Index;
				if (index>=0) {
					value=Units::ToDistance(DerivedDrawInfo.WaypointDistance);
					valid=true;
					if (value>99 || value==0)
						_stprintf(BufferValue, TEXT("%.0f"),value);
					else {
						if (ISPARAGLIDER) {
							if (value>10)
								_stprintf(BufferValue, TEXT("%.1f"),value);
							else 
								_stprintf(BufferValue, TEXT("%.2f"),value);
						} else {
							_stprintf(BufferValue, TEXT("%.1f"),value);
						}
					}
				} else {
					_stprintf(BufferValue, TEXT(NULLMEDIUM)); // 091221
				}
			   }
			} else {
				_stprintf(BufferValue, TEXT(NULLMEDIUM)); // 091221
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
				// LKTOKEN  _@M1023_ = "Next Distance", _@M1024_ = "Dist"
				lk::strcpy(BufferTitle, MsgToken<1024>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;
		// B12
		// Arrival altitude using current MC  and total energy. Does not use safetymc.
		// total energy is disabled!
		case LK_NEXT_ALTDIFF:
			_stprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1025_ = "Next Alt.Arrival", _@M1026_ = "NxtArr"
				lk::strcpy(BufferTitle, MsgToken<1026>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
            LockTaskData();
			if ( ValidTaskPoint(ActiveTaskPoint) != false ) {
				if (ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute()) index=RESWP_OPTIMIZED;
				else index = Task[ActiveTaskPoint].Index;
				if (index>=0) {
					// don't use current MC...
					value=Units::ToAltitude(WayPointCalc[index].AltArriv[AltArrivMode]);
					if ( value > ALTDIFFLIMIT ) {
						valid=true;
						_stprintf(BufferValue,TEXT("%+1.0f"), value);
					}
				}
			}
            UnlockTaskData();
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B13
		// Using MC! 
		case LK_NEXT_ALTREQ:
			_stprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1027_ = "Next Alt.Required", _@M1028_ = "NxtAltR"
				lk::strcpy(BufferTitle, MsgToken<1028>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
            LockTaskData();
			if ( ValidTaskPoint(ActiveTaskPoint) != false ) {
				if (ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute()) index=RESWP_OPTIMIZED;
				else index = Task[ActiveTaskPoint].Index;
				if (index>=0) {
					value=Units::ToAltitude(WayPointCalc[index].AltReqd[AltArrivMode]);
					if (value<10000 && value >-10000) {
						_stprintf(BufferValue,TEXT("%1.0f"), value);
						valid=true;
					} 
				}
			}
            UnlockTaskData();
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B14 EMPTY UNUSED
		case LK_NEXT_WP:
			goto lk_error;
			break;

		// B15 Arrival altitude , no more total energy
		case LK_FIN_ALTDIFF:
			_stprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1031_ = "Task Alt.Arrival", _@M1032_ = "TskArr"
				lk::strcpy(BufferTitle, MsgToken<1032>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
            LockTaskData();
			if (ValidTaskPointFast(ActiveTaskPoint)) {
				if (ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute()) index=RESWP_OPTIMIZED;
				else index = Task[ActiveTaskPoint].Index;
				if (index>=0) {
                    if(ISPARAGLIDER && DerivedDrawInfo.TaskAltitudeDifference > 0.0) {
                        value=Units::ToAltitude(DerivedDrawInfo.TaskAltitudeArrival);
                    } else {
    					value=Units::ToAltitude(DerivedDrawInfo.TaskAltitudeDifference);
                    }
					if ( value > ALTDIFFLIMIT ) {
						valid=true;
						_stprintf(BufferValue,TEXT("%+1.0f"), value);
					}
				}
			}
            UnlockTaskData();
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B16
		case LK_FIN_ALTREQ:
			_stprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1033_ = "Task Alt.Required", _@M1034_ = "TskAltR"
				lk::strcpy(BufferTitle, MsgToken<1034>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
            LockTaskData();
			if (ValidTaskPoint(ActiveTaskPoint)) {
				index = Task[ActiveTaskPoint].Index;
				if (index>=0) {
					value=Units::ToAltitude(DerivedDrawInfo.TaskAltitudeRequired);
					if (value<10000 && value >-10000) {
						_stprintf(BufferValue,TEXT("%1.0f"), value);
						valid=true;
					} 
				}
			}
            UnlockTaskData();
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;
		// B17
		case LK_SPEEDTASK_AVG:
			_stprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1035_ = "Task Speed Average", _@M1036_ = "TskSpAv"
				lk::strcpy(BufferTitle, MsgToken<1036>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			value=0;
			if ( ActiveTaskPoint >=1) {
				if ( ValidTaskPoint(ActiveTaskPoint) ) {
					value = Units::ToTaskSpeed(DerivedDrawInfo.TaskSpeed);
					if (value<=0||value>999) value=0; else valid=true;
					if (value<99)
						_stprintf(BufferValue, TEXT("%.1f"),value);
					else
						_stprintf(BufferValue, TEXT("%d"),(int)value);
				}
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetTaskSpeedName()));
			break;


		// B18
		case LK_FIN_DIST:
			if ( ValidTaskPoint(ActiveTaskPoint) != false ) {
				index = Task[ActiveTaskPoint].Index;
				if (index>=0) {
					if (DerivedDrawInfo.ValidFinish) {
						value = Units::ToDistance(DerivedDrawInfo.WaypointDistance);
					} else {
						value = Units::ToDistance(DerivedDrawInfo.TaskDistanceToGo);
					}
					valid=true;
					if (value>99)
						_stprintf(BufferValue, TEXT("%.0f"),value);
					else
						_stprintf(BufferValue, TEXT("%.1f"),value);
				} else {
					_stprintf(BufferValue, TEXT(NULLLONG));
				}
			} else {
				_stprintf(BufferValue, TEXT(NULLLONG));
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
				// LKTOKEN  _@M1037_ = "Task Distance", _@M1038_ = "TskDis"
				lk::strcpy(BufferTitle, MsgToken<1038>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;

		// B19
		case LK_RESERVED1:
			goto lk_error;
			break;

		// B20
		case LK_HGND:
                        _stprintf(BufferValue,_T(NULLLONG));
			if (DerivedDrawInfo.TerrainValid) {
				value=Units::ToAltitude(DerivedDrawInfo.TerrainAlt);
				valid=true;
				_stprintf(BufferValue, TEXT("%d"),(int)value);
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			if (lktitle)
				// LKTOKEN  _@M1041_ = "Terrain Elevation", _@M1042_ = "Gnd"
				lk::strcpy(BufferTitle, MsgToken<1042>());
			else 
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;


		// B21 091221
		case LK_TC_AVG:
			value= Units::ToVerticalSpeed(DerivedDrawInfo.AverageThermal);
			if (value==0)
				_stprintf(BufferValue, TEXT(NULLMEDIUM));
			else { 
				if (value<20) _stprintf(BufferValue, TEXT("%+.1lf"),value);
				else _stprintf(BufferValue, TEXT("%+.0lf"),value);
				valid=true; 
			}
			_stprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
			if (lktitle)
				// LKTOKEN  _@M1043_ = "Thermal Average", _@M1044_ = "TC.Avg"
				lk::strcpy(BufferTitle, MsgToken<1044>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;

		// B22 091221
		case LK_TC_GAIN:
			value=Units::ToAltitude(DerivedDrawInfo.ThermalGain);
			if (value==0)
				_stprintf(BufferValue, TEXT(NULLMEDIUM));
			else { 
				valid=true;
				_stprintf(BufferValue, TEXT("%+d"),(int)value);
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			if (lktitle)
				// LKTOKEN  _@M1045_ = "Thermal Gain", _@M1046_ = "TC.Gain"
				lk::strcpy(BufferTitle, MsgToken<1046>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;

		// B23
		case LK_TRACK:
			_stprintf(BufferValue,_T(NULLLONG));
			//_stprintf(BufferUnit,TEXT(""));
			if (lktitle)
				// LKTOKEN  _@M1047_ = "Track", _@M1048_ = "Track"
				lk::strcpy(BufferTitle, MsgToken<1048>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));
			value = AngleLimit360(DrawInfo.TrackBearing);
			valid=true;
			_stprintf(BufferValue, TEXT("%2.0f%s"), value, MsgToken<2179>());
			break;

		// B24
		case LK_VARIO:
			if (VarioAvailable(DrawInfo)) {
				value = Units::ToVerticalSpeed(DrawInfo.Vario);
			} else {
				value = Units::ToVerticalSpeed(DerivedDrawInfo.Vario);
			}
			valid=true;
			_stprintf(BufferValue,varformat,value);
			_stprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
			if (lktitle)
				// LKTOKEN  _@M1049_ = "Vario", _@M1050_ = "Vario"
				lk::strcpy(BufferTitle, MsgToken<1050>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;


		// B25
		case LK_WIND_SPEED:
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			_stprintf(BufferUnit, TEXT("%s"),Units::GetWindSpeedName());
			
			value=Units::ToWindSpeed(DerivedDrawInfo.WindSpeed);
			if (value>=1 ) {
				_stprintf(BufferValue,TEXT("%1.0f"), value );
				valid=true;
			} else {
				_stprintf(BufferValue,TEXT(NULLMEDIUM)); // 091221
			}
			break;


		// B26
		case LK_WIND_BRG:
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			if (Units::ToWindSpeed(DerivedDrawInfo.WindSpeed)>=1) {
				value = DerivedDrawInfo.WindBearing;
				valid=true;
				if (value==360) value=0;
				_stprintf(BufferValue,TEXT("%1.0f%s"), value, MsgToken<2179>());
			} else {
				_stprintf(BufferValue,TEXT(NULLMEDIUM));
			}
			
			break;


		// B27
		case LK_AA_TIME:
			_stprintf(BufferValue,_T(NULLTIME));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;

			if (ValidTaskPoint(ActiveTaskPoint) && UseAATTarget() && (DerivedDrawInfo.AATTimeToGo< 0.9*ERROR_TIME)) {

				Units::TimeToText(BufferValue, (int)DerivedDrawInfo.AATTimeToGo);
				valid=true;
			}
			_stprintf(BufferUnit,_T("h"));
			break;


		// B28
		case LK_AA_DISTMAX:
		// B29
		case LK_AA_DISTMIN:
			if ( (ValidTaskPoint(ActiveTaskPoint) != false) && UseAATTarget() ) {
				index = Task[ActiveTaskPoint].Index;
				if (index>=0) {
					if ( lkindex == LK_AA_DISTMAX )
						value = Units::ToDistance(DerivedDrawInfo.AATMaxDistance);
					else
						value = Units::ToDistance(DerivedDrawInfo.AATMinDistance);
					valid=true;
					if (value>99)
						_stprintf(BufferValue, TEXT("%.0f"),value);
					else
						_stprintf(BufferValue, TEXT("%.1f"),value);
				} else {
					_stprintf(BufferValue, TEXT(NULLMEDIUM)); // 091221
				}
			} else {
				_stprintf(BufferValue, TEXT(NULLMEDIUM)); // 091221
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;


		// B30
		case LK_AA_SPEEDMAX:
		// B31
		case LK_AA_SPEEDMIN:
			if ( (ValidTaskPoint(ActiveTaskPoint) != false) && UseAATTarget() && DerivedDrawInfo.AATTimeToGo>=1 ) {
				index = Task[ActiveTaskPoint].Index;
				if (index>=0) {
					if ( lkindex == LK_AA_SPEEDMAX )
						value = Units::ToTaskSpeed(DerivedDrawInfo.AATMaxSpeed);
					else
						value = Units::ToTaskSpeed(DerivedDrawInfo.AATMinSpeed);

					valid=true;
					_stprintf(BufferValue, TEXT("%.0f"),value);
				} else {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
				}
			} else {
				_stprintf(BufferValue, TEXT(NULLMEDIUM));
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetTaskSpeedName()));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;


		// B32
		case LK_IAS:
			if (DrawInfo.AirspeedAvailable) {
				if (lktitle)
					// LKTOKEN  _@M1065_ = "Airspeed IAS", _@M1066_ = "IAS"
					lk::strcpy(BufferTitle, MsgToken<1066>());
				else
					lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
				value=Units::ToHorizontalSpeed(DrawInfo.IndicatedAirspeed);
				if (value<0||value>999) value=0; else valid=true;
			} else {
				// LKTOKEN  _@M1065_ = "Airspeed IAS", _@M1066_ = "IAS"
				_stprintf(BufferTitle, TEXT("e%s"), MsgToken<1066>());
				value=Units::ToHorizontalSpeed(DerivedDrawInfo.IndicatedAirspeedEstimated);
				if (value<0||value>999) value=0; else valid=true;
			}
			_stprintf(BufferValue, TEXT("%d"),(int)value);
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			break;


		// B33
		case LK_HBARO:
			if (BaroAltitudeAvailable(DrawInfo)) {
				value=Units::ToAltitude(DrawInfo.BaroAltitude);
				valid=true;
				_stprintf(BufferValue, TEXT("%d"),(int)value);
			} else
				_stprintf(BufferValue, TEXT(NULLLONG));
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;


		// B34
		case LK_SPEED_MC:
			value=Units::ToHorizontalSpeed(DerivedDrawInfo.VMacCready);
			valid=true;
			if (value<=0||value>999) value=0; else valid=true;
			_stprintf(BufferValue, TEXT("%d"),(int)value);
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			if (lktitle)
				// LKTOKEN  _@M1069_ = "Speed MacReady", _@M1070_ = "SpMc"
				lk::strcpy(BufferTitle, MsgToken<1070>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;


		// B35
		case LK_PRCCLIMB:
			value=DerivedDrawInfo.PercentCircling;
			valid=true;
			_stprintf(BufferValue, TEXT("%d"),(int)value);
			_stprintf(BufferUnit, TEXT("%%"));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;

		// B36		TmFly
		case LK_TIMEFLIGHT:
			_stprintf(BufferValue,_T(NULLTIME));
			if (lktitle)
				// LKTOKEN  _@M1073_ = "Time of flight", _@M1074_ = "FlyTime"
				lk::strcpy(BufferTitle, MsgToken<1074>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;

			if (DerivedDrawInfo.FlightTime > 0) {
				valid=true;
				Units::TimeToText(BufferValue, (int)DerivedDrawInfo.FlightTime);
			} else {
				_stprintf(BufferValue, TEXT(NULLTIME));
			}
			_stprintf(BufferUnit, TEXT("h"));
			break;


		case LK_TIMETASK:
			_stprintf(BufferValue,_T(NULLTIME));
			if (lktitle)
				//  _@M2427_  "Task elapsed time" LKTOKEN _@M2428_ "TskTime"
				lk::strcpy(BufferTitle, MsgToken<2428>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;

			if (DerivedDrawInfo.TaskElapsedTime > 0) {
				valid=true;
				if (Units::TimeToTextDown(BufferValue, DerivedDrawInfo.TaskElapsedTime)) // 091112
					_stprintf(BufferUnit, TEXT("h"));
				else
					_stprintf(BufferUnit, TEXT("m"));
			} else {
				_stprintf(BufferValue, TEXT(NULLTIME));
			}

			break;

		// B37
		case LK_GLOAD:
			value=DerivedDrawInfo.Gload;
			_stprintf(BufferValue,TEXT("%+.1f"), value);
			valid=true;
			_stprintf(BufferTitle, TEXT("%s%s"),
						AccelerationAvailable(DrawInfo) ? _T("") : _T("e"),
						MsgToken<1076>());
			break;

		// B38
		case LK_MTG_BRG:
			_stprintf(BufferValue,_T(NULLLONG));
			if (lktitle)				
  // LKTOKEN "_@M001698_": "Multitarget Bearing",
 //  LKTOKEN "_@M001699_": "BrgMtg",              
				lk::strcpy(BufferTitle, MsgToken<1699>());
			else
				_stprintf(BufferTitle, TEXT("%s"), DataOptionsTitle(lkindex));
			ivalue = GetOvertargetIndex(); // Current Multitarget
			if(ivalue > 0) {
			value = WayPointCalc[ivalue].Bearing;  //Bearing;


              valid=true;
              
              if (value > 1)
                  _stprintf(BufferValue, TEXT("%2.0f%s"), value, MsgToken<2179>());
              else if (value < -1)
                  _stprintf(BufferValue, TEXT("%2.0f%s"), -value, MsgToken<2179>());
              else
                  _stprintf(BufferValue, TEXT("0%s"), MsgToken<2179>());
           }
                  else   valid=false;
			break;
            
		// B39
		case LK_TIME_LOCAL:
			Units::TimeToText(BufferValue, LocalTime(DrawInfo.Time));
			valid=true;
			if (lktitle)
				// LKTOKEN  _@M1079_ = "Time local", _@M1080_ = "Time"
				lk::strcpy(BufferTitle, MsgToken<1080>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;

			break;

		// B40
		case LK_TIME_UTC:
			Units::TimeToText(BufferValue,(int) DrawInfo.Time);
			valid=true;
			if (lktitle)
				// LKTOKEN  _@M1081_ = "Time UTC", _@M1082_ = "UTC"
				lk::strcpy(BufferTitle, MsgToken<1082>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;

			break;


		// B41	091006 using new task ete 
		case LK_FIN_ETE:
			goto lkfin_ete;


		// B42
		case LK_NEXT_ETE:
			lk::strcpy(BufferValue,_T(NULLLONG));
			// LKTOKEN  _@M1085_ = "Next Time To Go", _@M1086_ = "NextETE"
			_stprintf(BufferTitle, TEXT("%s"), DataOptionsTitle(lkindex));
			value = GetGlNextETE(DerivedDrawInfo);
			if (value > 0) {
				valid = true;
				if (Units::TimeToTextDown(BufferValue, value))  // 091112
					lk::strcpy(BufferUnit, TEXT("h"));
				else
					lk::strcpy(BufferUnit, TEXT("m"));
			} else {
				lk::strcpy(BufferValue, TEXT(NULLTIME));
			}
			break;

		// B43 AKA STF
		case LK_SPEED_DOLPHIN:
			value=Units::ToHorizontalSpeed(DerivedDrawInfo.VOpt);
			if (value<0||value>999) value=0; else valid=true;
			_stprintf(BufferValue, TEXT("%d"),(int)round(value));
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;


		// B44
		case LK_NETTO:
			value=Units::ToVerticalSpeed(DerivedDrawInfo.NettoVario);
			if (value<-100||value>100) value=0; else valid=true;
			_stprintf(BufferValue,varformat,value);
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetVerticalSpeedName()));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;


		// B45
		case LK_FIN_ETA:
			_stprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1091_ = "Task Arrival Time", _@M1092_ = "TskETA"
				lk::strcpy(BufferTitle, MsgToken<1092>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;

			if (ISCAR || ISGAAIRCRAFT) {
			    if (ValidTaskPoint(ActiveTaskPoint)) {
					if (DerivedDrawInfo.LKTaskETE > 0) {
						valid=true;
						Units::TimeToText(BufferValue, DerivedDrawInfo.LKTaskETE + LocalTime(DrawInfo.Time));
					} else {
						_stprintf(BufferValue, TEXT(NULLTIME));
					}
			    }
			} else {
			    if (IsValidTaskTimeToGo(DerivedDrawInfo) && ValidTaskPoint(ActiveTaskPoint)) {
					valid=true;
					Units::TimeToText(BufferValue, DerivedDrawInfo.TaskTimeToGo + LocalTime(DrawInfo.Time));
				} else {
					_stprintf(BufferValue, TEXT(NULLTIME));
				}
			}
			_stprintf(BufferUnit, TEXT("h"));
			break;


		// B46
		case LK_NEXT_ETA:
			lk::strcpy(BufferValue,_T(NULLLONG));
			// LKTOKEN  _@M1093_ = "Next Arrival Time", _@M1094_ = "NextETA"
			_stprintf(BufferTitle, TEXT("%s"), DataOptionsTitle(lkindex));
			value = GetGlNextETE(DerivedDrawInfo);
			if(value > 0) {
				valid = true;
				Units::TimeToText(BufferValue, value + LocalTime(DrawInfo.Time));
			} else {
				lk::strcpy(BufferValue, TEXT(NULLTIME));
			}
			break;
		// B47
		case LK_BRGDIFF:
		case LK_MTG_BRG_DIFF:
            index =0;
			_stprintf(BufferValue,_T(NULLMEDIUM)); // 091221
			_stprintf(BufferTitle, TEXT("%s"), DataOptionsTitle(lkindex));
            if(lkindex == LK_MTG_BRG_DIFF)
            {
              // LKTOKEN  _@M2476_ = "Multitarget Bearing Difference", _@M2477_ = "DiffMtg"
              if (lktitle)
				 lk::strcpy(BufferTitle, MsgToken<2477>());
              index = GetOvertargetIndex();
            }
            else
              if ( ValidTaskPoint(ActiveTaskPoint) != false ) 
              {
                 if (lktitle)
                  // LKTOKEN  _@M1095_ = "Bearing Difference", _@M1096_ = "To"
                    lk::strcpy(BufferTitle, MsgToken<1096>());

					if (ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute()) index=RESWP_OPTIMIZED;
					else index = Task[ActiveTaskPoint].Index;
              } 

            if (index>=0) {
                // THIS WOULD SET BEARING while circling
                // if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING))
                if (true)
                {
					if (gTaskType == task_type_t::AAT)
                        value=DerivedDrawInfo.WaypointBearing -  DrawInfo.TrackBearing;
                    else
                        value = WayPointCalc[index].Bearing -  DrawInfo.TrackBearing;
                    valid=true;
                    if (value < -180.0)
                        value += 360.0;
                    else
                        if (value > 180.0)
                            value -= 360.0;
                    if (value > 30)
                      _stprintf(BufferValue, TEXT("%2.0f%s%s"), value, MsgToken<2179>(), MsgToken<2183>());
                    else if (value > 2)
                        _stprintf(BufferValue, TEXT("%2.0f%s%s"), value, MsgToken<2179>(), MsgToken<2185>());
                    else if (value < -30)
                        _stprintf(BufferValue, TEXT("%s%2.0f%s"), MsgToken<2182>(), -value, MsgToken<2179>());
                    else if (value < -2)
                        _stprintf(BufferValue, TEXT("%s%2.0f%s"), MsgToken<2184>(), - value, MsgToken<2179>());
                    else
                        _stprintf(BufferValue, TEXT("%s%s"), MsgToken<2182>(), MsgToken<2183>());
                }
                else goto goto_bearing;
            }

			break;


		// B48 091216  OAT Outside Air Temperature
		case LK_OAT:
                  lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));
                  value = DrawInfo.OutsideAirTemperature;
                  if (!DrawInfo.TemperatureAvailable || value<-50||value>100) {
                    _stprintf(BufferValue, TEXT("---"));
                  }
                  else {
                    _stprintf(BufferValue, TEXT("%.0lf"), value);
                    _stprintf(BufferUnit,  TEXT("%s"), MsgToken<2179>());
                    valid = true;
                  }
                  break;
                  
		// B49
		case LK_RELHUM:
                  lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));
                  if(DrawInfo.HumidityAvailable) {
                    value = DrawInfo.RelativeHumidity;
                    _stprintf(BufferValue, TEXT("%.0lf"), value);
                    _stprintf(BufferUnit, TEXT("%%"));
                    valid = true;
                  }
                  else
                    _stprintf(BufferValue, TEXT("---"));
                  break;

		// B50 UNSUPPORTED
		case LK_MAXTEMP:
			goto lk_error;
			break;

		// B51
		case LK_AA_TARG_DIST:
			if ( (ValidTaskPoint(ActiveTaskPoint) != false) && UseAATTarget() ) {
				index = Task[ActiveTaskPoint].Index;
				if (index>=0) {
					value = Units::ToDistance(DerivedDrawInfo.AATTargetDistance);
					valid=true;
					if (value>99)
						_stprintf(BufferValue, TEXT("%.0f"),value);
					else
						_stprintf(BufferValue, TEXT("%.1f"),value);
				} else {
					_stprintf(BufferValue, TEXT(NULLMEDIUM)); // 091221
				}
			} else {
				_stprintf(BufferValue, TEXT(NULLMEDIUM)); // 091221
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;


		// B52
		case LK_AA_TARG_SPEED:
			if ( (ValidTaskPoint(ActiveTaskPoint) != false) && UseAATTarget() && DerivedDrawInfo.AATTimeToGo>=1 ) {
				index = Task[ActiveTaskPoint].Index;
				if (index>=0) {
					value = Units::ToTaskSpeed(DerivedDrawInfo.AATTargetSpeed);
					valid=true;
					_stprintf(BufferValue, TEXT("%.0f"),value);
				} else {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
				}
			} else {
				_stprintf(BufferValue, TEXT(NULLMEDIUM));
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetTaskSpeedName()));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;


		// B53
		case LK_LD_VARIO:
			_stprintf(BufferValue,_T(NULLMEDIUM));
			//_stprintf(BufferUnit,TEXT(""));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));
			if (DrawInfo.AirspeedAvailable && VarioAvailable(DrawInfo)) {
				value = DerivedDrawInfo.LDvario;
				if (value <1 || value >=ALTERNATE_MAXVALIDGR )
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
				else {
					valid=true;
					if (value >= 100) _stprintf(BufferValue, TEXT("%.0lf"),value);
						else _stprintf(BufferValue, TEXT("%.1lf"),value);
				}
			}
			break;


		// B54 091221
		case LK_TAS:
			if (DrawInfo.AirspeedAvailable) {
				if (lktitle)
					// LKTOKEN  _@M1109_ = "Airspeed TAS", _@M1110_ = "TAS"
					lk::strcpy(BufferTitle, MsgToken<1110>());
				else
					lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
				value=Units::ToHorizontalSpeed(DrawInfo.TrueAirspeed);
				if (value<0||value>999) {
					_stprintf(BufferValue, TEXT("%s"),NULLMEDIUM);
				} else {
					valid=true;
					_stprintf(BufferValue, TEXT("%d"),(int)value);
				}
			} else {
				// LKTOKEN  _@M1109_ = "Airspeed TAS", _@M1110_ = "TAS"
				_stprintf(BufferTitle, TEXT("e%s"), MsgToken<1110>());
				value=Units::ToHorizontalSpeed(DerivedDrawInfo.TrueAirspeedEstimated);
				if (value<0||value>999) {
					_stprintf(BufferValue, TEXT("%s"),NULLMEDIUM);
				} else {
					valid=true;
					_stprintf(BufferValue, TEXT("%d"),(int)value);
				}
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			break;

		// B55  Team Code 091216
		case LK_TEAM_CODE:
			if(ValidWayPoint(TeamCodeRefWaypoint)) {
				LK_tcsncpy(BufferValue,DerivedDrawInfo.OwnTeamCode,5);
				valid=true; // 091221
			} else
				_stprintf(BufferValue,_T("----"));
			if (lktitle)
				// LKTOKEN  _@M1111_ = "Team Code", _@M1112_ = "TeamCode"
				lk::strcpy(BufferTitle, MsgToken<1112>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;

		// B56  Team Code 091216
		case LK_TEAM_BRG:
			_stprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1113_ = "Team Bearing", _@M1114_ = "TmBrng"
				lk::strcpy(BufferTitle, MsgToken<1114>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;

			if(ValidWayPoint(TeamCodeRefWaypoint) && TeammateCodeValid) {
				value=DerivedDrawInfo.TeammateBearing;
				valid=true;
				if (value > 1)
					_stprintf(BufferValue, TEXT("%2.0f%s"), value, MsgToken<2179>());
				else if (value < -1)
					_stprintf(BufferValue, TEXT("%2.0f%s"), -value, MsgToken<2179>());
				else
					_stprintf(BufferValue, TEXT("0%s"), MsgToken<2179>());
			}
			break;

		// B57 Team Bearing Difference 091216
		case LK_TEAM_BRGDIFF:
			_stprintf(BufferValue,_T(NULLLONG));

			if (lktitle)
				// LKTOKEN  _@M1115_ = "Team Bearing Diff", _@M1116_ = "TeamBd"
				lk::strcpy(BufferTitle, MsgToken<1116>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;

			if (ValidWayPoint(TeamCodeRefWaypoint) && TeammateCodeValid) {
				value = DerivedDrawInfo.TeammateBearing -  DrawInfo.TrackBearing;
				valid=true; // 091221
				if (value < -180.0)
					value += 360.0;
				else
					if (value > 180.0) value -= 360.0;

	              if (value > 30)
	                _stprintf(BufferValue, TEXT("%2.0f%s%s"), value, MsgToken<2179>(), MsgToken<2183>());
	              else
	                if (value > 2)
	                  _stprintf(BufferValue, TEXT("%2.0f%s%s"), value, MsgToken<2179>(), MsgToken<2185>());
	                else
	                  if (value < -30)
	                    _stprintf(BufferValue, TEXT("%s%2.0f%s"), MsgToken<2182>(), -value, MsgToken<2179>());
	                  else
	                    if (value < -2)
	                      _stprintf(BufferValue, TEXT("%s%2.0f%s"), MsgToken<2184>(), - value, MsgToken<2179>());
	                    else
	                      _stprintf(BufferValue, TEXT("%s%s"), MsgToken<2182>(), MsgToken<2183>());
			}
			break;

		// B58 091216 Team Range Distance
		case LK_TEAM_DIST:
			if ( TeammateCodeValid ) {
				value=Units::ToDistance(DerivedDrawInfo.TeammateRange);
				valid=true;
				if (value>99)
					_stprintf(BufferValue, TEXT("%.0f"),value);
				else
					_stprintf(BufferValue, TEXT("%.1f"),value);
			} else {
				_stprintf(BufferValue, TEXT(NULLLONG));
			}

			_stprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
				// LKTOKEN  _@M1117_ = "Team Range", _@M1118_ = "TeamDis"
				lk::strcpy(BufferTitle, MsgToken<1118>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;


		// B59
		case LK_SPEEDTASK_INST:
			_stprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1119_ = "Task Speed Instantaneous", _@M1120_ = "TskSpI"
				lk::strcpy(BufferTitle, MsgToken<1120>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			value=0;
			if ( ActiveTaskPoint >=1) {
				if ( ValidTaskPoint(ActiveTaskPoint) ) {
					value = Units::ToTaskSpeed(DerivedDrawInfo.TaskSpeedInstantaneous);
					if (value<=0||value>999) value=0; else valid=true;
					if (value<99)
						_stprintf(BufferValue, TEXT("%.1f"),value);
					else
						_stprintf(BufferValue, TEXT("%d"),(int)value);
				}
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetTaskSpeedName()));
			break;
		// B60
		case LK_HOME_DIST:
		case LK_HOME_DISTNM:
			if (HomeWaypoint>=0) {
				if ( ValidWayPoint(HomeWaypoint) != false ) {
					if (lkindex == LK_HOME_DISTNM)
						value = Units::To(unNauticalMiles, DerivedDrawInfo.HomeDistance);
					else 
						value = Units::ToDistance(DerivedDrawInfo.HomeDistance);

					valid=true;
					if (value>99)
						_stprintf(BufferValue, TEXT("%.0f"),value);
					else
						_stprintf(BufferValue, TEXT("%.1f"),value);
				} else {
					_stprintf(BufferValue, TEXT(NULLMEDIUM)); // 091221
				}
			} else {
				_stprintf(BufferValue, TEXT(NULLMEDIUM)); // 091221
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
			{
				// LKTOKEN  _@M1121_ = "Home Distance", _@M1122_ = "HomeDis"
				lk::strcpy(BufferTitle, MsgToken<1122>());
				if (lkindex == LK_HOME_DISTNM)
				{
					lk::strcpy(BufferUnit, TEXT("nm"));
					lk::strcpy(BufferTitle, MsgToken<2493>());
				}
			}
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;


		// B61
		case LK_SPEEDTASK_ACH:
			_stprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1123_ = "Task Speed", _@M1124_ = "TskSp"
				lk::strcpy(BufferTitle, MsgToken<1124>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			value=0;
			if ( ActiveTaskPoint >=1) {
				if ( ValidTaskPoint(ActiveTaskPoint) ) {
					value = Units::ToTaskSpeed(DerivedDrawInfo.TaskSpeedAchieved);
					if (value<0||value>999) value=0; else valid=true;
					if (value<99)
						_stprintf(BufferValue, TEXT("%.1f"),value);
					else
						_stprintf(BufferValue, TEXT("%d"),(int)value);
				}
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetTaskSpeedName()));
			break;


		// B62
		case LK_AA_DELTATIME:
			_stprintf(BufferValue,_T(NULLTIME));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			_stprintf(BufferUnit,_T("h"));
			// TODO This is in the wrong place, should be moved to calc thread! 090916
			if (UseAATTarget() && IsValidTaskTimeToGo(DerivedDrawInfo) && ValidTaskPoint(ActiveTaskPoint)) {
				double dd = DerivedDrawInfo.TaskTimeToGo;
				if ((DerivedDrawInfo.TaskStartTime>0.0) && (DerivedDrawInfo.Flying) &&(ActiveTaskPoint>0)) {
					dd += DrawInfo.Time-DerivedDrawInfo.TaskStartTime;
				}
				dd= max(0.0,min(24.0*3600.0,dd))-AATTaskLength*60;
				if (dd < (0.9*ERROR_TIME)) {
					valid=true;
					Units::TimeToText(BufferValue, (int)dd);
				}
			}
			break;

		// B63 091221
		case LK_TC_ALL:
			if (DerivedDrawInfo.timeCircling <=0)
				//value=0.0;
				_stprintf(BufferValue, TEXT(NULLMEDIUM));
			else {
				value = Units::ToVerticalSpeed(DerivedDrawInfo.TotalHeightClimb /DerivedDrawInfo.timeCircling);
				if (value<20)
					_stprintf(BufferValue, TEXT("%+.1lf"),value);
				else
					_stprintf(BufferValue, TEXT("%+.0lf"),value);
				valid=true;
			}
			_stprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
			if (lktitle)
				// LKTOKEN  _@M1127_ = "Thermal All", _@M1128_ = "Th.All"
				lk::strcpy(BufferTitle, MsgToken<1128>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;
		// B64
		case LK_LOGGER:
			lk::strcpy(BufferTitle, MsgToken<1695>());
			if (LoggerActive)
				lk::strcpy(BufferValue,MsgToken<1700>()); // ON
			else {
				if (DisableAutoLogger)
					lk::strcpy(BufferValue,MsgToken<1701>()); // no!
				else
					lk::strcpy(BufferValue,MsgToken<1696>()); // auto
			}
			valid = true;
			break;

		// B65 FIXED 100125
		case LK_BATTERY:
			_stprintf(BufferValue,_T(NULLMEDIUM));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;

			value = PDABatteryPercent;
			// We may choose this approach, but it is not as in V5.
			/*
			#if TESTBENCH
                	if ( (!SIMMODE && !HaveBatteryInfo) || value<1||value>100)
			#else
                	if (!HaveBatteryInfo || value<1||value>100)
			#endif
			*/
			// This is V5 compatible
                	if (!HaveBatteryInfo || value<1||value>100)
				_stprintf(BufferValue,_T("---"));
                	else {
				if (PDABatteryFlag==Battery::CHARGING || PDABatteryStatus==Battery::ONLINE) {
					_stprintf(BufferValue,TEXT("%2.0f%%C"), value);	 // 100228
				} else {
					_stprintf(BufferValue,TEXT("%2.0f%%D"), value);  // 100228
				}

				valid = true;
			}
			break;


		// B66
		case LK_FIN_GR:
			_stprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1133_ = "Task Req.Efficiency", _@M1134_ = "TskReqE"
				lk::strcpy(BufferTitle, MsgToken<1134>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			if ( (ValidTaskPoint(ActiveTaskPoint) != false) ) {
				index = Task[ActiveTaskPoint].Index;
				if (index>=0) {
					// the ValidFinish() seem to return FALSE when is actually valid.
					// In any case we do not use it for the vanilla GR
					value = DerivedDrawInfo.GRFinish;
					if (value <1 || value >=ALTERNATE_MAXVALIDGR )
						_stprintf(BufferValue, TEXT(NULLMEDIUM));
					else {
						if (value >= 100) _stprintf(BufferValue, TEXT("%.0lf"),value);
							else _stprintf(BufferValue, TEXT("%.1lf"),value);
						valid=true;
					}
				}
			}
			break;

		// B67
		case LK_ALTERN1_GR:
		// B68
		case LK_ALTERN2_GR:
		// B69
		case LK_BESTALTERN_GR:
		// 153
		case LK_HOME_GR:
			bShowWPname = true;
			_stprintf(BufferValue,_T(NULLMEDIUM));
			if (lktitle) {
				switch (lkindex) {
					case LK_BESTALTERN_GR:
						// LKTOKEN  _@M1139_ = "BestAltern Req.Efficiency", _@M1140_ = "BAtn.E"
						lk::strcpy(BufferTitle, MsgToken<1140>());
						break;
					case LK_ALTERN1_GR:
						// LKTOKEN  _@M1135_ = "Alternate1 Req.Efficiency", _@M1136_ = "Atn1.E"
						lk::strcpy(BufferTitle, MsgToken<1136>());
						break;
					case LK_ALTERN2_GR:
						// LKTOKEN  _@M1137_ = "Alternate2 Req.Efficiency", _@M1138_ = "Atn2.E"
						lk::strcpy(BufferTitle, MsgToken<1138>());
						break;
					case LK_HOME_GR:
						// LKTOKEN  _@M2484_ = Home Req.Efficiency", _@M2485_ = "Home.E"
						lk::strcpy(BufferTitle, MsgToken<2485>());
						bShowWPname = false;
						break;
					default:
						_stprintf(BufferTitle, TEXT("Atn%d.E"), lkindex-LK_ALTERNATESGR+1);
						break;
				}//sw
			} else {
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
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
				case LK_HOME_GR:
					index=HomeWaypoint;
					break;
				default:
					index=0;
					break;
			}

			if(ValidWayPoint(index) && bShowWPname)
			{
				if ( DisplayTextType == DISPLAYFIRSTTHREE)
				{
					 LK_tcsncpy(BufferTitle,WayPointList[index].Name,3);
				}
				else if( DisplayTextType == DISPLAYNUMBER) {
					_stprintf(BufferTitle,TEXT("%d"), WayPointList[index].Number );
				} else {
					LK_tcsncpy(BufferTitle,WayPointList[index].Name, 12);
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
				case LK_HOME_GR:
					if ( ValidWayPoint(HomeWaypoint) ) value=WayPointCalc[HomeWaypoint].GR;
					else value=INVALID_GR;
					break;
				default:
					value = 1;
					break;
			}
			if (value <1 || value >=ALTERNATE_MAXVALIDGR ) {
				_stprintf(BufferValue, TEXT(NULLMEDIUM));
				valid=false;
			} else {
				if (value >= 100) _stprintf(BufferValue, TEXT("%.0lf"),value);
					else _stprintf(BufferValue, TEXT("%.1lf"),value);
				valid=true;
			}
			break;


		// B70
		case LK_QFE:
			value=Units::ToAltitude(DerivedDrawInfo.NavAltitude-QFEAltitudeOffset);
			valid=true;
			_stprintf(BufferValue, TEXT("%d"),(int)value);
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;


		// B71	LD AVR 
		case LK_LD_AVR:
			_stprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1143_ = "Average Efficiency", _@M1144_ = "E.Avg"
				lk::strcpy(BufferTitle, MsgToken<1144>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
				if (DerivedDrawInfo.Flying)
					value=DerivedDrawInfo.AverageLD;
				else
					value=0;

				if (value==0) {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
				} else {
					if (value <1 ||  value >=ALTERNATE_MAXVALIDGR ) {
						lk::strcpy(BufferValue, infinity);
						valid=true;
					} else {

						if (value<100)
							_stprintf(BufferValue, TEXT("%.1f"),value);
						else
							_stprintf(BufferValue, TEXT("%2.0f"),value);
						valid=true;
					}
				}
			}
			break;


		// B72	WP REQ EFF
		case LK_NEXT_GR:
			_stprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1145_ = "Next Req.Efficiency", _@M1146_ = "Req.E"
				lk::strcpy(BufferTitle, MsgToken<1146>());
			else
				// LKTOKEN  _@M1145_ = "Next Req.Efficiency", _@M1146_ = "Req.E"
				lk::strcpy(BufferTitle, MsgToken<1146>());
            
            LockTaskData();
			if ( ValidTaskPoint(ActiveTaskPoint) != false ) {
				if (ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute()) index=RESWP_OPTIMIZED;
				else index = Task[ActiveTaskPoint].Index;
				if (index>=0) {
					value=WayPointCalc[index].GR;
					if (value <1 || value >=ALTERNATE_MAXVALIDGR )
						_stprintf(BufferValue, TEXT(NULLMEDIUM));
					else {
						if (value >= 100) _stprintf(BufferValue, TEXT("%.0lf"),value);
							else _stprintf(BufferValue, TEXT("%.1lf"),value);
						valid=true;
					}
				}
			}
            UnlockTaskData();
			break;

		// B73
		case LK_FL:
			lk::strcpy(BufferTitle, MsgToken<1148>());
			// Cant use NavAltitude, because FL should use Baro if available, despite
			// user settings.
			if (BaroAltitudeAvailable(DrawInfo))
				value = Units::To(unFligthLevel,QNHAltitudeToQNEAltitude(DrawInfo.BaroAltitude));
			else
				value = Units::To(unFligthLevel,QNHAltitudeToQNEAltitude(DrawInfo.Altitude));

			if (value>=1) {
				valid=true;
				_stprintf(BufferValue, TEXT("%d"),(int)value);
			} else {
				valid=true;
				_stprintf(BufferValue, TEXT("--"));
			}
			break;


		// B74
		case LK_TASK_DISTCOV:
			if ( (ActiveTaskPoint >=1) && ( ValidTaskPoint(ActiveTaskPoint) )) {
				value = Units::ToDistance(DerivedDrawInfo.TaskDistanceCovered);
				valid=true;
				_stprintf(BufferValue, TEXT("%.0f"),value); // l o f?? TODO CHECK
			} else {
				_stprintf(BufferValue, TEXT(NULLLONG));
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
				// LKTOKEN  _@M1149_ = "Task Covered distance", _@M1150_ = "TskCov"
				lk::strcpy(BufferTitle, MsgToken<1150>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;


		// B75
		case LK_ALTERN1_ARRIV:
		// B76
		case LK_ALTERN2_ARRIV:
		// B77
		case LK_BESTALTERN_ARRIV:
			_stprintf(BufferValue,_T(NULLMEDIUM));
			if (lktitle) {
				switch (lkindex) {
					case LK_BESTALTERN_ARRIV:
						// LKTOKEN  _@M1155_ = "BestAlternate Arrival", _@M1156_ = "BAtnArr"
						lk::strcpy(BufferTitle, MsgToken<1156>());
						break;
					case LK_ALTERN1_ARRIV:
						// LKTOKEN  _@M1151_ = "Alternate1 Arrival", _@M1152_ = "Atn1Arr"
						lk::strcpy(BufferTitle, MsgToken<1152>());
						break;
					case LK_ALTERN2_ARRIV:
						// LKTOKEN  _@M1153_ = "Alternate2 Arrival", _@M1154_ = "Atn2Arr"
						lk::strcpy(BufferTitle, MsgToken<1154>());
						break;
					default:
						_stprintf(BufferTitle, TEXT("Atn%dArr"), lkindex-LK_ALTERNATESARRIV+1);
						break;
				} //sw
			} else {
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
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
					 LK_tcsncpy(BufferTitle,WayPointList[index].Name,3);
				}
				else if( DisplayTextType == DISPLAYNUMBER) {
					_stprintf(BufferTitle,TEXT("%d"), WayPointList[index].Number );
				} else {
					LK_tcsncpy(BufferTitle,WayPointList[index].Name, 12);
					// BufferTitle[(sizeof(Text)/sizeof(TCHAR))-1] = '\0';
					if (lktitle)
						BufferTitle[12] = '\0'; // FIX TUNING
					else
						BufferTitle[8] = '\0';  // FIX TUNING
				}
			}
			switch (lkindex) {
				case LK_ALTERN1_ARRIV:
					if ( ValidWayPoint(Alternate1) ) value=Units::ToAltitude(WayPointCalc[Alternate1].AltArriv[AltArrivMode]);
					else value=INVALID_DIFF;
					break;
				case LK_ALTERN2_ARRIV:
					if ( ValidWayPoint(Alternate2) ) value=Units::ToAltitude(WayPointCalc[Alternate2].AltArriv[AltArrivMode]);
					else value=INVALID_DIFF;
					break;
				case LK_BESTALTERN_ARRIV:
					if ( ValidWayPoint(BestAlternate) ) value=Units::ToAltitude(WayPointCalc[BestAlternate].AltArriv[AltArrivMode]);
					else value=INVALID_DIFF;
					break;
				default:
					value = INVALID_DIFF;
					break;
			}
			if (value <= ALTDIFFLIMIT ) {
				_stprintf(BufferValue, TEXT(NULLLONG));
				valid=false;
			} else { // 091221
				if ( (value>-1 && value<=0) || (value>=0 && value<1))
					_stprintf(BufferValue, TEXT("0"));
				else {
					_stprintf(BufferValue, TEXT("%+.0f"),value);
				}
				valid=true;
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;


		// B78
		case LK_HOMERADIAL:
			_stprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1157_ = "Home Radial", _@M1158_ = "Radial"
				lk::strcpy(BufferTitle, MsgToken<1158>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;

			if ( ValidWayPoint(HomeWaypoint) != false ) {
				if (DerivedDrawInfo.HomeDistance >10.0) {
					// homeradial == 0, ok?
					value = DerivedDrawInfo.HomeRadial;
					valid=true;
					if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0f%s"), value, MsgToken<2179>());
					else if (value < -1)
						_stprintf(BufferValue, TEXT("%2.0f%s"), -value, MsgToken<2179>());
                    else
                        _stprintf(BufferValue, TEXT("0%s"), MsgToken<2179>());
				}
			}
			break;


		// B79
		case LK_AIRSPACEHDIST:
			if (lktitle)
				// LKTOKEN  _@M1159_ = "Airspace Distance", _@M1160_ = "AirSpace"
				lk::strcpy(BufferTitle, MsgToken<1160>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;


			value = fabs(Units::ToDistance(NearestAirspaceHDist));

			if (value < 1.0) {
				_stprintf(BufferValue, TEXT("%1.3f"),value);
			} else {
				_stprintf(BufferValue, TEXT("%1.1f"),value);
			}

			_stprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			valid = (NearestAirspaceHDist > 0);

			break;


		// B80
		case LK_EXTBATTBANK:
			_stprintf(BufferValue,_T(NULLMEDIUM));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			ivalue=DrawInfo.ExtBatt_Bank;
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
			_stprintf(BufferValue,_T(NULLMEDIUM));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			value = (lkindex==LK_EXTBATT1VOLT?DrawInfo.ExtBatt1_Voltage:DrawInfo.ExtBatt2_Voltage);
			// hack to display percentage instead of voltage
			if (value>=1000) {
				value-=1000;
				if (value>0.0) {
					_stprintf(BufferValue,TEXT("%.0f%%"), value);
					valid = true;
				} else {
					valid = false;
				}
			} else {
				if (value>0.0) {
					_stprintf(BufferValue,TEXT("%0.2fv"), value);
					valid = true;
				} else {
					valid = false;
				}
			}
			break;

      
		// B83
		case LK_ODOMETER:
			if (DerivedDrawInfo.Odometer>0) {
				value=Units::ToDistance(DerivedDrawInfo.Odometer);
				valid=true;
				if (value>99)
					_stprintf(BufferValue, TEXT("%.0f"),value);
				else
					_stprintf(BufferValue, TEXT("%.1f"),value);
			} else {
				_stprintf(BufferValue, TEXT(NULLMEDIUM)); // 091221
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
				// LKTOKEN  _@M1167_ = "Odometer", _@M1168_ = "Odo"
				lk::strcpy(BufferTitle, MsgToken<1168>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;

		// B84  100126
		case LK_AQNH:
			if (lktitle)
				// LKTOKEN  _@M1169_ = "Altern QNH", _@M1170_ = "aAlt"
				lk::strcpy(BufferTitle, MsgToken<1170>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;

			value=Units::ToAlternateAltitude(DerivedDrawInfo.NavAltitude);
			valid=true;
			_stprintf(BufferValue, TEXT("%d"),(int)value);
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAlternateAltitudeName()));
			break;

		// B85  100126
		case LK_AALTAGL:
			if (lktitle)
				// LKTOKEN  _@M1171_ = "Altern AGL", _@M1172_ = "aHAGL"
				lk::strcpy(BufferTitle, MsgToken<1172>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			if (!DerivedDrawInfo.TerrainValid) { //@ 101013
				_stprintf(BufferValue, TEXT(NULLLONG));
				_stprintf(BufferUnit, TEXT("%s"),(Units::GetAlternateAltitudeName()));
				valid=false;
				break;
			}
			value=Units::ToAlternateAltitude(DerivedDrawInfo.AltitudeAGL);
			valid=true;
			_stprintf(BufferValue, TEXT("%d"),(int)value);
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAlternateAltitudeName()));
			break;


		// B86
		case LK_HGPS:
			if (lktitle)
				// LKTOKEN  _@M1173_ = "Altitude GPS", _@M1174_ = "HGPS"
				lk::strcpy(BufferTitle, MsgToken<1174>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			if (DrawInfo.NAVWarning || (DrawInfo.SatellitesUsed == 0)) {
				_stprintf(BufferValue, TEXT(NULLLONG));
				valid=false;
			} else {
				value=Units::ToAltitude(DrawInfo.Altitude);
				valid=true;
				_stprintf(BufferValue, TEXT("%d"),(int)value);
				_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			}
			break;


		// B87  100908
		case LK_EQMC:
			// LKTOKEN  _@M1175_ = "MacCready Equivalent", _@M1176_ = "eqMC"
			lk::strcpy(BufferTitle, MsgToken<1176>());
			if ( DerivedDrawInfo.Circling || DerivedDrawInfo.EqMc<0 || DerivedDrawInfo.OnGround) {
				_stprintf(BufferValue, TEXT(NULLMEDIUM));
			} else {
				value = iround(Units::ToVerticalSpeed(DerivedDrawInfo.EqMc*10))/10.0;
				valid=true;
				_stprintf(BufferValue, TEXT("%2.1lf"),value);
			}
			break;


		// B88 B89
		case LK_EXP1:
			_stprintf(BufferTitle, TEXT("RAM"));
			_stprintf(BufferValue, TEXT("%u"),(unsigned int)(CheckFreeRam()/1024));
                        valid=true;
                        break;

		case LK_EXP2:
			_stprintf(BufferTitle, TEXT("hTE"));
			_stprintf(BufferValue, TEXT("%3.0f"),DerivedDrawInfo.EnergyHeight);
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			valid=true;
			break;


		// B90 B91 B92 B93 B94 B95 B96
		case LK_OLC_CLASSIC_DIST:
			ivalue=CContestMgr::TYPE_OLC_CLASSIC;
			goto olc_dist;
		case LK_OLC_FAI_DIST:
			ivalue=CContestMgr::TYPE_OLC_FAI;
			goto olc_dist;
		case LK_OLC_LEAGUE_DIST:
			ivalue=CContestMgr::TYPE_OLC_LEAGUE;
			goto olc_dist;
		case LK_OLC_3TPS_DIST:
			ivalue=CContestMgr::TYPE_FAI_3_TPS;;
			goto olc_dist;
		case LK_OLC_CLASSIC_PREDICTED_DIST:
			ivalue=CContestMgr::TYPE_OLC_CLASSIC_PREDICTED;
			goto olc_dist;
		case LK_OLC_FAI_PREDICTED_DIST:
			ivalue=CContestMgr::TYPE_OLC_FAI_PREDICTED;
			goto olc_dist;
		case LK_OLC_3TPS_PREDICTED_DIST:
			ivalue=CContestMgr::TYPE_FAI_3_TPS_PREDICTED;


olc_dist:
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));

			if (OlcResults[ivalue].Type()==CContestMgr::TYPE_INVALID)
				_stprintf(BufferValue,_T(NULLMEDIUM));
			else {
				_stprintf(BufferValue, TEXT("%5.0f"),Units::ToDistance(OlcResults[ivalue].Distance()));
				valid=true;
			}
			break;

		// B97 B98 B99 B100 B101 B102 B103
		case LK_OLC_CLASSIC_SPEED:
			ivalue=CContestMgr::TYPE_OLC_CLASSIC;
			goto olc_speed;
		case LK_OLC_FAI_SPEED:
			ivalue=CContestMgr::TYPE_OLC_FAI;
			goto olc_speed;
		case LK_OLC_LEAGUE_SPEED:
			ivalue=CContestMgr::TYPE_OLC_LEAGUE;
			goto olc_speed;
		case LK_OLC_3TPS_SPEED:
			ivalue=CContestMgr::TYPE_FAI_3_TPS;;
			goto olc_speed;
		case LK_OLC_CLASSIC_PREDICTED_SPEED:
			ivalue=CContestMgr::TYPE_OLC_CLASSIC_PREDICTED;
			goto olc_speed;
		case LK_OLC_FAI_PREDICTED_SPEED:
			ivalue=CContestMgr::TYPE_OLC_FAI_PREDICTED;
			goto olc_speed;
		case LK_OLC_3TPS_PREDICTED_SPEED:
			ivalue=CContestMgr::TYPE_FAI_3_TPS_PREDICTED;
olc_speed:
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));

			if (OlcResults[ivalue].Type()==CContestMgr::TYPE_INVALID)
				_stprintf(BufferValue,_T(NULLMEDIUM));
			else {
				if ( OlcResults[ivalue].Speed() >999 ) {
					_stprintf(BufferValue,_T(NULLMEDIUM));
				} else {
					_stprintf(BufferValue, TEXT("%3.1f"),Units::ToHorizontalSpeed(OlcResults[ivalue].Speed()));
					valid=true;
				}
			}
			break;
		
		// B104 B105 B106 B107 B108 B109 B110 B111 B112
		case LK_OLC_CLASSIC_SCORE:
			ivalue=CContestMgr::TYPE_OLC_CLASSIC;
			goto olc_score;
		case LK_OLC_FAI_SCORE:
			ivalue=CContestMgr::TYPE_OLC_FAI;
			goto olc_score;
		case LK_OLC_LEAGUE_SCORE:
			ivalue=CContestMgr::TYPE_OLC_LEAGUE;
			goto olc_score;
		case LK_OLC_3TPS_SCORE:
			ivalue=CContestMgr::TYPE_FAI_3_TPS;;
			goto olc_score;
		case LK_OLC_CLASSIC_PREDICTED_SCORE:
			ivalue=CContestMgr::TYPE_OLC_CLASSIC_PREDICTED;
			goto olc_score;
		case LK_OLC_FAI_PREDICTED_SCORE:
			ivalue=CContestMgr::TYPE_OLC_FAI_PREDICTED;
			goto olc_score;
		case LK_OLC_3TPS_PREDICTED_SCORE:
			ivalue=CContestMgr::TYPE_FAI_3_TPS_PREDICTED;
			goto olc_score;
		case LK_OLC_PLUS_SCORE:
			ivalue=CContestMgr::TYPE_OLC_PLUS;
			goto olc_score;
		case LK_OLC_PLUS_PREDICTED_SCORE:
			ivalue=CContestMgr::TYPE_OLC_PLUS_PREDICTED;
olc_score:
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			_stprintf(BufferUnit, TEXT("pt"));

			if (OlcResults[ivalue].Type()==CContestMgr::TYPE_INVALID)
				_stprintf(BufferValue,_T(NULLMEDIUM));
			else {
				_stprintf(BufferValue, TEXT("%3.0f"),OlcResults[ivalue].Score());
				valid=true;
			}
			break;

		// B113
		case LK_FLAPS:			
			lk::strcpy(BufferTitle, MsgToken<1641>());
			if (GlidePolar::FlapsPosCount>0) {
				_stprintf(BufferValue,TEXT("%s"), DerivedDrawInfo.Flaps);
				BufferValue[7]='\0'; // set a limiter to the name: max 7 chars
				valid=true;
			} else {
				_stprintf(BufferValue, TEXT(NULLMEDIUM));
			}
			break;

		// B114
		case LK_AIRSPACEVDIST:
			if (lktitle)
				lk::strcpy(BufferTitle, MsgToken<1286>()); // ArSpcV
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));

			if (NearestAirspaceVDist != 0 && (fabs(NearestAirspaceVDist)<=9999) ) { // 9999 m or ft is ok
				value = Units::ToAltitude(NearestAirspaceVDist);
				_stprintf(BufferValue, TEXT("%.0f"),value);
				valid = true;
			} else {
				valid=false;
				_stprintf(BufferValue, TEXT(NULLMEDIUM));
			}
			if(NearestAirspaceVDist < 0)
			  valid=false;

			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;


		// B115
		case LK_HOME_ARRIVAL:
			if (lktitle)
				// LKTOKEN  _@M1644_ = "Home Alt.Arrival", _@M1645_ = "HomeArr"
				lk::strcpy(BufferTitle, MsgToken<1645>());
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));
      
			if ( ValidWayPoint(HomeWaypoint) != false ) {
				value=Units::ToAltitude(WayPointCalc[HomeWaypoint].AltArriv[AltArrivMode]);
				if ( value > ALTDIFFLIMIT ) {
					valid=true;
					_stprintf(BufferValue,TEXT("%+1.0f"), value);
				}
			}
			if (!valid)
				_stprintf(BufferValue,_T(NULLLONG));

			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;


		// B116
		case LK_ALTERN1_BRG:
		// B117
		case LK_ALTERN2_BRG:
		// B118
		case LK_BESTALTERN_BRG:
		// B155
		case LK_HOME_BRG:
			_stprintf(BufferValue,_T(NULLMEDIUM));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));
			bShowWPname = true;
			switch(lkindex) {
				case LK_ALTERN1_BRG:
					index=Alternate1;
					break;
				case LK_ALTERN2_BRG:
					index=Alternate2;
					break;
				case LK_BESTALTERN_BRG:
					index=BestAlternate;
					break;
				case LK_HOME_BRG:
					index=AirfieldsHomeWaypoint;
					bShowWPname = false;
					break;
				default:
					index=0;
					break;
			}

			if(ValidWayPoint(index) && bShowWPname)
			{
				if ( DisplayTextType == DISPLAYFIRSTTHREE)
				{
					 LK_tcsncpy(BufferTitle,WayPointList[index].Name,3);
				}
				else if( DisplayTextType == DISPLAYNUMBER) {
					_stprintf(BufferTitle,TEXT("%d"), WayPointList[index].Number );
				} else {
					LK_tcsncpy(BufferTitle,WayPointList[index].Name, 12);
					// BufferTitle[(sizeof(Text)/sizeof(TCHAR))-1] = '\0';
					if (lktitle)
						BufferTitle[12] = '\0'; // FIX TUNING
					else
						BufferTitle[8] = '\0';  // FIX TUNING
				}
				value=WayPointCalc[index].Bearing;
				valid=true;
			}

			if (valid) {
				if (value > 1)
					_stprintf(BufferValue, TEXT("%2.0f%s"), value, MsgToken<2179>());
				else if (value < -1)
					_stprintf(BufferValue, TEXT("%2.0f%s"), -value, MsgToken<2179>());
                else
                    _stprintf(BufferValue, TEXT("0%s"), MsgToken<2179>());
			} 
			break;

		// B119
		case LK_ALTERN1_DIST:
		// B120
		case LK_ALTERN2_DIST:
		// B121
		case LK_BESTALTERN_DIST:
			_stprintf(BufferValue,_T(NULLMEDIUM));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			switch(lkindex) {
				case LK_ALTERN1_DIST:
					index=Alternate1;
					break;
				case LK_ALTERN2_DIST:
					index=Alternate2;
					break;
				case LK_BESTALTERN_DIST:
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
					LK_tcsncpy(BufferTitle,WayPointList[index].Name,3);
				}
				else if( DisplayTextType == DISPLAYNUMBER) {
					_stprintf(BufferTitle,TEXT("%d"), WayPointList[index].Number );
				} else {
					LK_tcsncpy(BufferTitle,WayPointList[index].Name, 12);
					// BufferTitle[(sizeof(Text)/sizeof(TCHAR))-1] = '\0';
					if (lktitle)
						BufferTitle[12] = '\0'; // FIX TUNING
					else
						BufferTitle[8] = '\0';  // FIX TUNING
				}
				value=Units::ToDistance(WayPointCalc[index].Distance);
				valid=true;
			}


			if (valid) {
				if (value>99 || value==0)
					_stprintf(BufferValue, TEXT("%.0f"),value);
				else {
					if (ISPARAGLIDER) {
						if (value>10)
							_stprintf(BufferValue, TEXT("%.1f"),value);
						else
							_stprintf(BufferValue, TEXT("%.2f"),value);
					} else {
							_stprintf(BufferValue, TEXT("%.1f"),value);
					}
				}
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			break;



		// B122
		case LK_MAXALT:
			value=Units::ToAltitude(DerivedDrawInfo.MaxAltitude);
			valid=true;
			_stprintf(BufferValue, TEXT("%d"),(int)value);
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;

		// B123
		case LK_MAXHGAINED:
			value=Units::ToAltitude(DerivedDrawInfo.MaxHeightGain);
			valid=true;
			_stprintf(BufferValue, TEXT("%d"),(int)value);
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;


		// B124
		case LK_HEADWINDSPEED:
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));
			lk::strcpy(BufferUnit, Units::GetWindSpeedName());

			if (DerivedDrawInfo.HeadWind != -999) {
				value=Units::ToWindSpeed(DerivedDrawInfo.HeadWind);
				if (std::abs(value) >= 1) {
					_stprintf(BufferValue,TEXT("%+1.0f"), value);
					valid=true;
				}
			} 
			if(!valid) {
				lk::strcpy(BufferValue,TEXT(NULLMEDIUM));
			}
			break;

		// B125
		case LK_OLC_FAI_CLOSE:
			bFAI = CContestMgr::Instance().FAI();
			fTogo =Units::ToDistance(CContestMgr::Instance().GetClosingPointDist());
			if(fTogo >0)
			{
				if (fTogo>99)
					_stprintf(BufferValue, TEXT("%.0f"),fTogo);
				else
					_stprintf(BufferValue, TEXT("%.1f"),fTogo);
				valid = true;
			} else {
				_stprintf(BufferValue, TEXT(NULLLONG));
			}
			_stprintf(BufferUnit, TEXT("%s*"),(Units::GetDistanceName()));
			if (lktitle)
			{
				if(bFAI)
		    			_stprintf(BufferTitle, TEXT("FAI %s"),  MsgToken<1508>()); //   _@M1508_ = "C:"
		      		else
		        		_stprintf(BufferTitle, TEXT("%s"),  MsgToken<1508>());
		    	}
		        else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;

	    		break;

		// B126
		case LK_OLC_FAI_CLOSE_PERCENT:
			bFAI = CContestMgr::Instance().FAI();
			fDist =CContestMgr::Instance().Result(CContestMgr::TYPE_OLC_FAI_PREDICTED, false).Distance();
			fTogo =CContestMgr::Instance().GetClosingPointDist();
        		if((fDist >0) && (fTogo >0))
            		{
              			LKASSERT(fDist >0)
  				valid = true;
             			 value = fTogo / fDist*100.0f;
				_stprintf(BufferValue, TEXT("%.1f"),value);
		    	} else {
		    		_stprintf(BufferValue, TEXT(NULLLONG));
		    	}
		    	_stprintf(BufferUnit, TEXT("%%"));
		    	if (lktitle)
		    	{
		    		if(bFAI)
		        		_stprintf(BufferTitle, TEXT("FAI %s"),  MsgToken<1508>()); // LKTOKEN  _@M1508_ = "C:"
		      		else
		        		_stprintf(BufferTitle, TEXT("%s"),  MsgToken<1508>());
		    	}
		    	else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;

			break;


                // B127
		case LK_BANK_ANGLE:
                        valid=true;
			if(DrawInfo.GyroscopeAvailable) { 
				value=DrawInfo.Roll;
                                // LKTOKEN _@M1197_ "Bank"
                                lk::strcpy(BufferTitle, MsgToken<1197>());
			} else {
				value=DerivedDrawInfo.BankAngle;
				_stprintf(BufferTitle, TEXT("e%s"), MsgToken<1197>());
			}
            _stprintf(BufferValue,TEXT("%.0f%s"), value, MsgToken<2179>());
			break;

		// B128
		case LK_ALTERN1_RAD:
		// B129
		case LK_ALTERN2_RAD:
			_stprintf(BufferValue,_T(NULLMEDIUM));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			switch(lkindex) {
				case LK_ALTERN1_RAD:
					index=Alternate1;
					break;
				case LK_ALTERN2_RAD:
					index=Alternate2;
					break;
				default:
					index=0;
					break;
			}

			if(ValidWayPoint(index))
			{
				if (DisplayTextType == DISPLAYFIRSTTHREE) {
					_stprintf(BufferTitle, _T("<%.3s"), WayPointList[index].Name);
				}
				else if( DisplayTextType == DISPLAYNUMBER) {
					_stprintf(BufferTitle,TEXT("<%d"), WayPointList[index].Number );
				} else {
					_stprintf(BufferTitle, _T("<%.12s"), WayPointList[index].Name);
					if (lktitle)
						BufferTitle[12] = '\0'; // FIX TUNING
					else
						BufferTitle[8] = '\0';  // FIX TUNING
				}
				value=AngleLimit360(WayPointCalc[index].Bearing+180);;
				valid=true;
			}

			if (valid) {
				if (value > 1)
					_stprintf(BufferValue, TEXT("%2.0f%s"), value, MsgToken<2179>());
				else if (value < -1)
					_stprintf(BufferValue, TEXT("%2.0f%s"), -value, MsgToken<2179>());
                else
                    _stprintf(BufferValue, TEXT("0%s"), MsgToken<2179>());
			} 
			break;

		// B130
		case LK_HEADING:
			_stprintf(BufferValue,_T(NULLLONG));
			//_stprintf(BufferUnit,TEXT(""));
			if (DrawInfo.MagneticHeadingAvailable) {
			    _stprintf(BufferTitle, _T("HDG"));
			    value = DrawInfo.MagneticHeading;
			} else {
			    _stprintf(BufferTitle, _T("eHDG"));
			    value = DerivedDrawInfo.Heading;
			}
			valid=true;
			if (value > 1)
				_stprintf(BufferValue, TEXT("%2.0f%s"), value, MsgToken<2179>());
			else if (value < -1)
				_stprintf(BufferValue, TEXT("%2.0f%s"), -value, MsgToken<2179>());
            else
                _stprintf(BufferValue, TEXT("0%s"), MsgToken<2179>());
			break;


		// B131
		case LK_ALTERN1_DISTNM:
		// B132
		case LK_ALTERN2_DISTNM:
			_stprintf(BufferValue,_T(NULLMEDIUM));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			switch(lkindex) {
				case LK_ALTERN1_DISTNM:
					index=Alternate1;
					break;
				case LK_ALTERN2_DISTNM:
					index=Alternate2;
					break;
				default:
					index=0;
					break;
			}

			if(ValidWayPoint(index))
			{
				if ( DisplayTextType == DISPLAYFIRSTTHREE)
				{
					LK_tcsncpy(BufferTitle,WayPointList[index].Name,3);
				}
				else if( DisplayTextType == DISPLAYNUMBER) {
					_stprintf(BufferTitle,TEXT("%d"), WayPointList[index].Number );
				} else {
					LK_tcsncpy(BufferTitle,WayPointList[index].Name, 12);
					// BufferTitle[(sizeof(Text)/sizeof(TCHAR))-1] = '\0';
					if (lktitle)
						BufferTitle[12] = '\0'; // FIX TUNING
					else
						BufferTitle[8] = '\0';  // FIX TUNING
				}
				value= Units::To(unNauticalMiles, WayPointCalc[index].Distance);
				valid=true;
			}


			if (valid) {
				if (value>99 || value==0)
					_stprintf(BufferValue, TEXT("%.0f"),value);
				else {
					if (ISPARAGLIDER) {
						if (value>10)
							_stprintf(BufferValue, TEXT("%.1f"),value);
						else
							_stprintf(BufferValue, TEXT("%.2f"),value);
					} else {
							_stprintf(BufferValue, TEXT("%.1f"),value);
					}
				}
			}
			_stprintf(BufferUnit, TEXT("nm"));
			break;

		// B133 Speed of maximum efficiency
		case LK_SPEED_ME:
			value=Units::ToHorizontalSpeed(DerivedDrawInfo.Vme);
			if (value<0||value>999) value=0; else valid=true;
			_stprintf(BufferValue, TEXT("%d"),(int)value);
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;

		// B134 Target Req. Efficicency
		case LK_TARGET_RE: // Target Req. Efficicency
			switch (OvertargetMode) {
				case OVT_TASK:
					LKFormatValue(LK_NEXT_GR, false, BufferValue, BufferUnit, BufferTitle);
					break;
				case OVT_TASKCENTER:
					LKFormatValue(LK_NEXT_CENTER_GR, false, BufferValue, BufferUnit, BufferTitle);
					break;
				case OVT_BALT:
					LKFormatValue(LK_BESTALTERN_GR, false, BufferValue, BufferUnit, BufferTitle);
					break;
				case OVT_ALT1:
					LKFormatValue(LK_ALTERN1_GR, false, BufferValue, BufferUnit, BufferTitle);
					break;
				case OVT_ALT2:
					LKFormatValue(LK_ALTERN2_GR, false, BufferValue, BufferUnit, BufferTitle);
					break;
				case OVT_HOME:
                    LKFormatValue(LK_HOME_GR, false, BufferValue, BufferUnit, BufferTitle);
					break;
				case OVT_THER:
					LKFormatGR(RESWP_LASTTHERMAL, BufferValue, BufferUnit);
					break;
				case OVT_MATE:
					LKFormatGR(RESWP_TEAMMATE, BufferValue, BufferUnit);
					break;
                case OVT_XC:
                    LKFormatGR(RESWP_FAIOPTIMIZED, BufferValue, BufferUnit);
                    break;
				case OVT_FLARM:
					LKFormatGR(RESWP_FLARMTARGET, BufferValue, BufferUnit);
					break;
				default:
					LKFormatValue(LK_NEXT_GR, false, BufferValue, BufferUnit, BufferTitle);
					break;
			}
			_stprintf(BufferTitle, TEXT("E.Tar"));
			break;

		// B135 Altitude QNE
		case LK_QNE:
			lk::strcpy(BufferTitle, MsgToken<2324>());
			value = -1;
			if (BaroAltitudeAvailable(DrawInfo)) {
				value= QNHAltitudeToQNEAltitude(DrawInfo.BaroAltitude);
				_stprintf(BufferValue, TEXT("%d"),(int)value);
			}
			else {
				_stprintf(BufferValue, TEXT("--"));

			}
			valid=true;
			break;

    case LK_NEXT_DIST_RADIUS:
	  valid=false;
      if ( ValidTaskPoint(ActiveTaskPoint)) {
        const double Distance = WayPointCalc[Task[ActiveTaskPoint].Index].Distance;
		sector_param param = GetTaskSectorParameter(ActiveTaskPoint); 
		switch(param.type) {
			case sector_type_t::CIRCLE:
			case sector_type_t::ESS_CIRCLE:
			value = Distance - param.radius;
			valid=true;
			break;
		default:
			break;
		}
      }

      if(valid) {
        value=Units::ToDistance(value);
        if (value>99 || value==0)
          _stprintf(BufferValue, TEXT("%.0f"),value);
        else {
          if (ISPARAGLIDER) {
            if (value>10)
              _stprintf(BufferValue, TEXT("%.1f"),value);
            else 
              _stprintf(BufferValue, TEXT("%.2f"),value);
          } else {
            _stprintf(BufferValue, TEXT("%.1f"),value);
          }
        }          
      } else {
        lk::strcpy(BufferValue, TEXT(NULLMEDIUM)); // 091221
      }
      
      lk::strcpy(BufferUnit, Units::GetDistanceName());
      lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));
      break;

        //
      case LK_XC_FF_DIST:
        ivalue = CContestMgr::TYPE_XC_FREE_FLIGHT;
        lk::strcpy(BufferUnit, Units::GetDistanceName());
        lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));
        if (OlcResults[ivalue].Type() == CContestMgr::TYPE_INVALID)
          _stprintf(BufferValue, _T(NULLMEDIUM));
        else {
          _stprintf(BufferValue, TEXT("%5.0f"), Units::ToDistance(OlcResults[ivalue].Distance()));
          valid = true;
        }
        break;

      case LK_XC_FF_SCORE:
        ivalue = CContestMgr::TYPE_XC_FREE_FLIGHT;
        lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));
        _stprintf(BufferUnit, TEXT("p"));
        if (OlcResults[ivalue].Type() == CContestMgr::TYPE_INVALID)
          _stprintf(BufferValue, _T(NULLMEDIUM));
        else {
          _stprintf(BufferValue, TEXT("%5.0f"),  OlcResults[ivalue].Score());
          valid = true;
        }
        break;

      case LK_XC_FT_DIST:
        ivalue = CContestMgr::TYPE_XC_FREE_TRIANGLE;
        lk::strcpy(BufferUnit, Units::GetDistanceName());
        lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));
        if (OlcResults[ivalue].Type() == CContestMgr::TYPE_INVALID)
          _stprintf(BufferValue, _T(NULLMEDIUM));
        else {
          _stprintf(BufferValue, TEXT("%5.0f"), Units::ToDistance(OlcResults[ivalue].Distance()));
          valid = true;
        }
        break;

      case LK_XC_FT_SCORE:
        ivalue = CContestMgr::TYPE_XC_FREE_TRIANGLE;
        lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
        _stprintf(BufferUnit, TEXT("p"));
        if (OlcResults[ivalue].Type() == CContestMgr::TYPE_INVALID)
          _stprintf(BufferValue, _T(NULLMEDIUM));
        else {
          _stprintf(BufferValue, TEXT("%3.0f"),  OlcResults[ivalue].Score());
          valid = true;
        }
        break;


      case LK_XC_FAI_DIST:
        ivalue = CContestMgr::TYPE_XC_FAI_TRIANGLE;
        lk::strcpy(BufferUnit, Units::GetDistanceName());
        lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));
        if (OlcResults[ivalue].Type() == CContestMgr::TYPE_INVALID)
          _stprintf(BufferValue, _T(NULLMEDIUM));
        else {
          _stprintf(BufferValue, TEXT("%5.0f"), Units::ToDistance(OlcResults[ivalue].Distance()));
          valid = true;
        }
        break;

      case LK_XC_FAI_SCORE:
        ivalue = CContestMgr::TYPE_XC_FAI_TRIANGLE;
        lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
        _stprintf(BufferUnit, TEXT("p"));
        if (OlcResults[ivalue].Type() == CContestMgr::TYPE_INVALID)
          _stprintf(BufferValue, _T(NULLMEDIUM));
        else {
          _stprintf(BufferValue, TEXT("%5.0f"),  OlcResults[ivalue].Score());
          valid = true;
        }
        break;

      case LK_XC_DIST:
        ivalue = CContestMgr::TYPE_XC;
        lk::strcpy(BufferUnit, Units::GetDistanceName());
        lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));
        if (OlcResults[ivalue].Type() == CContestMgr::TYPE_INVALID)
          _stprintf(BufferValue, _T(NULLMEDIUM));
        else {
          value = Units::ToDistance(OlcResults[ivalue].Distance());
          if (value >= 100){
            _stprintf(BufferValue, TEXT("%.0f"), value);
          }else{
            _stprintf(BufferValue, TEXT("%.1f"), value);
          }
          LKgetOLCBmp(OlcResults[ivalue].Type(),BmpTitle);
          valid = true;
        }
        break;

      case LK_XC_SCORE:
        ivalue = CContestMgr::TYPE_XC;
        lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
        _stprintf(BufferUnit, TEXT("p"));
        if (OlcResults[ivalue].Type() == CContestMgr::TYPE_INVALID)
          _stprintf(BufferValue, _T(NULLMEDIUM));
        else {
          value = OlcResults[ivalue].Score();
          if (value >= 100){
            _stprintf(BufferValue, TEXT("%.0f"), value);
          }else{
            _stprintf(BufferValue, TEXT("%.1f"), value);
          }
          LKgetOLCBmp(OlcResults[ivalue].Type(),BmpTitle);
          valid = true;
        }
        break;

      case LK_XC_TYPE:
        ivalue = CContestMgr::TYPE_XC;
        _stprintf(BufferTitle, TEXT("%s"), DataOptionsTitle(lkindex));
        lk::strcpy(BufferUnit, _T(""));
        LKgetOLCBmp(OlcResults[ivalue].Type(),BmpValue,BufferValue);
        valid = true;    
        break;

      case LK_XC_CLOSURE_DIST:
        ivalue = CContestMgr::TYPE_XC;
        lk::strcpy(BufferUnit, Units::GetDistanceName());
        lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));
        if (CContestMgr::Instance().GetXCTriangleClosureDistance() == 0)
          _stprintf(BufferValue, _T(NULLMEDIUM));
        else if (OlcResults[ivalue].PredictedDistance() == 0 )  {
          _stprintf(BufferValue, _T(NULLMEDIUM));
        }
        else {
          const double dist = Units::ToDistance(CContestMgr::Instance().GetXCTriangleClosureDistance());
          _stprintf(BufferValue, TEXT("%5.1f"), dist);
          valid = true;
        }
        break;

      case LK_XC_CLOSURE_PERC:
        lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
        _stprintf(BufferUnit, TEXT("%s"), TEXT("%"));
        if (CContestMgr::Instance().GetXCTriangleClosurePercentage() <= 0 ||
            CContestMgr::Instance().GetXCTriangleClosurePercentage() >= 100)
          _stprintf(BufferValue, _T(NULLMEDIUM));
        else {
          const double dist = CContestMgr::Instance().GetXCTriangleClosurePercentage()  ;
          _stprintf(BufferValue, TEXT("%5.1f"), dist);
          valid = true;
        }
        break;


      case LK_XC_PREDICTED_DIST:
        lk::strcpy(BufferUnit, Units::GetDistanceName());
        lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));
        if (CContestMgr::Instance().GetXCTriangleDistance() == 0)
          _stprintf(BufferValue, _T(NULLMEDIUM));
        else {
          _stprintf(BufferValue, TEXT("%5.0f"), Units::ToDistance(CContestMgr::Instance().GetXCTriangleDistance()));
          valid = true;
        }
        break;

		case LK_XC_MEAN_SPEED:
			ivalue = CContestMgr::TYPE_XC_FREE_TRIANGLE;
		lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
		_stprintf(BufferUnit, TEXT("%s"), (Units::GetHorizontalSpeedName()));
		if (CContestMgr::Instance().GetXCMeanSpeed() == 0)
			_stprintf(BufferValue, _T(NULLMEDIUM));
		else {
			_stprintf(BufferValue, TEXT("%5.0f"), Units::ToHorizontalSpeed(CContestMgr::Instance().GetXCMeanSpeed()));
			valid = true;
		}
		break;

        // B141
		case LK_WIND:
			// LKTOKEN  _@M1185_ = "Wind"
			lk::strcpy(BufferTitle, MsgToken<1185>());
			if (Units::ToWindSpeed(DerivedDrawInfo.WindSpeed)>=1) {
				value = DerivedDrawInfo.WindBearing;
				valid=true;
				if (UseWindRose) {
					_stprintf(BufferValue,TEXT("%s/%1.0f"), 
						AngleToWindRose(value), Units::ToWindSpeed(DerivedDrawInfo.WindSpeed));
				} else {
					if (value==360) value=0;
					if (HideUnits)
						_stprintf(BufferValue,TEXT("%03.0f/%1.0f"), 
							value, Units::ToWindSpeed(DerivedDrawInfo.WindSpeed));
					else
						_stprintf(BufferValue,TEXT("%03.0f%s/%1.0f"), 
							value, MsgToken<2179>(), Units::ToWindSpeed(DerivedDrawInfo.WindSpeed));
				}
			} else {
				_stprintf(BufferValue,TEXT("--/--"));
			}
			break;



		// B142 Final arrival with MC 0 , no totaly energy.
		case LK_FIN_ALTDIFF0:
			_stprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1191_ = "TskArr0"
				lk::strcpy(BufferTitle, MsgToken<1191>());
			else
				// LKTOKEN  _@M1191_ = "TskArr0"
				lk::strcpy(BufferTitle, MsgToken<1191>());
			if (ValidTaskPoint(ActiveTaskPoint)) {
				index = Task[ActiveTaskPoint].Index;
				if (index>=0) {
					value=Units::ToAltitude(DerivedDrawInfo.TaskAltitudeDifference0);
					if ( value > ALTDIFFLIMIT ) {
						valid=true;
						_stprintf(BufferValue,TEXT("%+1.0f"), value);
					}
				}
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B143  091222 using old ETE corrected now




		case LK_LKFIN_ETE:
lkfin_ete:
			_stprintf(BufferValue,_T(NULLTIME)); // 091222
			// LKTOKEN  _@M1083_ = "Task Time To Go", _@M1084_ = "TskETE"
			lk::strcpy(BufferTitle, DataOptionsTitle(LK_FIN_ETE));
			// ^^ Notice we use LK_FIN_ETE, NOT LK_LKFIN_ETE which does NOT exist in DataOptions!

			if (ISCAR || ISGAAIRCRAFT) {
                LockTaskData();
			    if ( ValidTaskPoint(ActiveTaskPoint)) {
				if (DerivedDrawInfo.LKTaskETE > 0) { 
					valid=true;
					if ( Units::TimeToTextDown(BufferValue, (int)DerivedDrawInfo.LKTaskETE)) {
						_stprintf(BufferUnit, TEXT("h"));
                    }
				} else {
					index = Task[ActiveTaskPoint].Index;
					if ( WayPointCalc[index].NextETE > 0) { // single waypoint? uhm
						valid=true;
						if (Units::TimeToTextDown(BufferValue, (int)WayPointCalc[index].NextETE)) {
                            _stprintf(BufferUnit, TEXT("h"));
                        }
					} else
						_stprintf(BufferValue, TEXT(NULLTIME));
                    }
                }
                UnlockTaskData();
			    break;
			}

            LockTaskData();
			if (ValidTaskPointFast(ActiveTaskPoint)) {
			    if (IsValidTaskTimeToGo(DerivedDrawInfo)) {
					valid=true;
					if (Units::TimeToTextDown(BufferValue, DerivedDrawInfo.TaskTimeToGo)) {
						_stprintf(BufferUnit, TEXT("h"));
                    }
				} else {
					index = Task[ActiveTaskPoint].Index;
					if ( (WayPointCalc[index].NextETE > 0) && !ValidTaskPoint(1) ) {
						valid=true;
						if (Units::TimeToTextDown(BufferValue, (int)WayPointCalc[index].NextETE)) {
                            _stprintf(BufferUnit, TEXT("h"));
                        }
					} else {
						_stprintf(BufferValue, TEXT(NULLTIME));
                    }
				}
			}
            UnlockTaskData();
			break;

		// B144
		// Using MC=0!  total energy disabled
		case LK_NEXT_ALTDIFF0:
			_stprintf(BufferValue,_T(NULLLONG));
			// LKTOKEN  _@M1190_ = "ArrMc0"
			lk::strcpy(BufferTitle, MsgToken<1190>());

			if ( ValidTaskPoint(ActiveTaskPoint) != false ) {
				if (ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute()) index=RESWP_OPTIMIZED;
				else index = Task[ActiveTaskPoint].Index;
				if (index>=0) {
					value=Units::ToAltitude(DerivedDrawInfo.NextAltitudeDifference0);
					if ( value > ALTDIFFLIMIT ) {
						valid=true;
						_stprintf(BufferValue,TEXT("%+1.0f"), value);
					}
				}
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B145
		case LK_TIME_LOCALSEC:
			Units::TimeToTextS(BufferValue, LocalTime(DrawInfo.Time));
			valid=true;
			// LKTOKEN  _@M1079_ = "Time local", _@M1080_ = "Time"
			lk::strcpy(BufferTitle, MsgToken<1080>());
			break;

		// B146
		case LK_TARGET_DIST:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
			} else {
	            if (DrawInfo.FLARM_Traffic[LKTargetIndex].RadioId <= 0) {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
				} else {
					// get values
	                value=Units::ToDistance(LKTraffic[LKTargetIndex].Distance);
					if (value>99) {
						_stprintf(BufferValue, TEXT(NULLMEDIUM));
					} else {
						valid=true;
						_stprintf(BufferValue, TEXT("%.1f"),value);
					}
				}
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			// LKTOKEN  _@M1023_ = "Next Distance", _@M1024_ = "Dist"
			lk::strcpy(BufferTitle, MsgToken<1024>());
			break;

		// B147
		case LK_TARGET_TO:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
			} else {
	            if (DrawInfo.FLARM_Traffic[LKTargetIndex].RadioId <= 0) {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
				} else {
						value = LKTraffic[LKTargetIndex].Bearing -  DrawInfo.TrackBearing;
						valid=true;
						if (value < -180.0)
							value += 360.0;
						else
							if (value > 180.0)
								value -= 360.0;
			              if (value > 30)
			                _stprintf(BufferValue, TEXT("%2.0f%s%s"), value, MsgToken<2179>(), MsgToken<2183>());
			              else
			                if (value > 2)
			                  _stprintf(BufferValue, TEXT("%2.0f%s%s"), value, MsgToken<2179>(), MsgToken<2185>());
			                else
			                  if (value < -30)
			                    _stprintf(BufferValue, TEXT("%s%2.0f%s"), MsgToken<2182>(), -value, MsgToken<2179>());
			                  else
			                    if (value < -2)
			                      _stprintf(BufferValue, TEXT("%s%2.0f%s"), MsgToken<2184>(), - value, MsgToken<2179>());
			                    else
			                      _stprintf(BufferValue, TEXT("%s%s"), MsgToken<2182>(), MsgToken<2183>());
				}
			}
			// LKTOKEN  _@M1095_ = "Bearing Difference", _@M1096_ = "To"
			lk::strcpy(BufferTitle, MsgToken<1096>());
			break;

		// B148
		case LK_TARGET_BEARING:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
			} else {
                if (DrawInfo.FLARM_Traffic[LKTargetIndex].RadioId <= 0) {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
				} else {
						value = LKTraffic[LKTargetIndex].Bearing;
						valid=true;
						if (value == 360)
							_stprintf(BufferValue, TEXT("0%s"), MsgToken<2179>());
						else 
							_stprintf(BufferValue, TEXT("%2.0f%s"), value, MsgToken<2179>());
				}
			}
			// LKTOKEN  _@M1007_ = "Bearing", _@M1008_ = "Brg"
			lk::strcpy(BufferTitle, MsgToken<1008>());
			break;

		// B149
		case LK_TARGET_SPEED:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
			    _stprintf(BufferValue, TEXT(NULLMEDIUM));
			} else {
	            if (DrawInfo.FLARM_Traffic[LKTargetIndex].RadioId <= 0) {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
				} else {
					value=Units::ToHorizontalSpeed(DrawInfo.FLARM_Traffic[LKTargetIndex].Speed);
					if (value<0||value>9999) value=0; else valid=true;
					_stprintf(BufferValue, TEXT("%d"),(int)value);
				}
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			// LKTOKEN  _@M1013_ = "Speed ground", _@M1014_ = "GS"
			lk::strcpy(BufferTitle, MsgToken<1014>());
			break;

		// B150 Multi Target QNH Arrival
		// QNH arrival altitude at the currently selected Multitarget. This is a QNH altitude, not a height
		// above ground. Does not include safety height. Can be negative
		case LK_MTG_QNH_ARRIV:
			valid = TurnpointQnhArrival(GetOvertargetIndex(), value, BufferValue, BufferUnit);
			// LKTOKEN _@M002472_ = "MultiTarget QNH Arrival", _@M002473_ = "MTgtArr"
			lk::strcpy(BufferTitle, MsgToken<2473>());
			break;

		// B152 QNH Arrival at Alternate 1
		case LK_ALTERN1_QNH_ARRIV:
			valid = TurnpointQnhArrival(Alternate1, value, BufferValue, BufferUnit);
			// LKTOKEN  _@M002478_ = "Alternate1 QNH Arrival", _@M002479_ = "Alt1QNH",
			lk::strcpy(BufferTitle, MsgToken<2479>());
			break;

		// B242
		case LK_TARGET_ALT:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
			} else {
                if (DrawInfo.FLARM_Traffic[LKTargetIndex].RadioId <= 0) {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
				} else {
					value=Units::ToAltitude(DrawInfo.FLARM_Traffic[LKTargetIndex].Altitude);
					valid=true;
					_stprintf(BufferValue, TEXT("%d"),(int)value);
				}
			}
			// LKTOKEN  _@M1001_ = "Altitude QNH", _@M1002_ = "Alt", 
			lk::strcpy(BufferTitle, MsgToken<1002>());
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B243
		// DO NOT USE RELATIVE ALTITUDE: when not real time, it won't change in respect to our position!!!
		// This is negative when target is below us because it represent a remote position
		case LK_TARGET_ALTDIFF:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
			} else {
                if (DrawInfo.FLARM_Traffic[LKTargetIndex].RadioId <= 0) {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
				} else {
					value=Units::ToAltitude(DerivedDrawInfo.NavAltitude-DrawInfo.FLARM_Traffic[LKTargetIndex].Altitude)*-1;
					valid=true;
					_stprintf(BufferValue, TEXT("%+d"),(int)value);
				}
			}
			// LKTOKEN  _@M1193_ = "RelAlt"
			lk::strcpy(BufferTitle, MsgToken<1193>());
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B244
		case LK_TARGET_VARIO:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
					lk::strcpy(BufferUnit, _T(""));
			} else {
                if (DrawInfo.FLARM_Traffic[LKTargetIndex].RadioId <= 0) {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
					lk::strcpy(BufferUnit, _T(""));
				} else {
					value = Units::ToVerticalSpeed(DrawInfo.FLARM_Traffic[LKTargetIndex].ClimbRate);
					valid=true;
					_stprintf(BufferValue,varformat,value);
					_stprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
				}
			}
			// LKTOKEN  _@M1194_ = "Var"
			lk::strcpy(BufferTitle, MsgToken<1194>());
			break;

		// B245
		case LK_TARGET_AVGVARIO:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
					lk::strcpy(BufferUnit, _T(""));
			} else {
				if (DrawInfo.FLARM_Traffic[LKTargetIndex].RadioId <= 0) {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
					lk::strcpy(BufferUnit, _T(""));
				} else {
					value = Units::ToVerticalSpeed(DrawInfo.FLARM_Traffic[LKTargetIndex].Average30s);
					valid=true;
					_stprintf(BufferValue,varformat,value);
					_stprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
				}
			}
			// LKTOKEN  _@M1189_ = "Var30"
			lk::strcpy(BufferTitle, MsgToken<1189>());
			break;

		// B246
		case LK_TARGET_ALTARRIV:
			_stprintf(BufferValue,_T(NULLLONG));
			// LKTOKEN  _@M1188_ = "Arr"
			lk::strcpy(BufferTitle, MsgToken<1188>());
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
			} else {
				if (DrawInfo.FLARM_Traffic[LKTargetIndex].RadioId <= 0) {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
				} else {

					value=Units::ToAltitude(LKTraffic[LKTargetIndex].AltArriv);
					if ( value > ALTDIFFLIMIT ) {
						valid=true;
						_stprintf(BufferValue,TEXT("%+1.0f"), value);
					} else {
						_stprintf(BufferValue, TEXT(NULLMEDIUM));
					}
				}
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B247
		case LK_TARGET_GR:
			_stprintf(BufferValue,_T(NULLLONG));
			// LKTOKEN  _@M1187_ = "ReqE"
			lk::strcpy(BufferTitle, MsgToken<1187>());
			lk::strcpy(BufferUnit,_T(""));
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
				_stprintf(BufferValue, TEXT(NULLMEDIUM));
			} else {
                if (DrawInfo.FLARM_Traffic[LKTargetIndex].RadioId <= 0) {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
				} else {
					value=LKTraffic[LKTargetIndex].GR;
					if (value <1 || value >=ALTERNATE_MAXVALIDGR )
						_stprintf(BufferValue, TEXT(NULLMEDIUM));
					else {
						if (value >= 100) _stprintf(BufferValue, TEXT("%.0lf"),value);
							else _stprintf(BufferValue, TEXT("%.1lf"),value);
						valid=true;
					}
				}
			}
			break;

		// B248
		case LK_TARGET_EIAS:
			// LKTOKEN  _@M1186_ = "eIAS"
			lk::strcpy(BufferTitle, MsgToken<1186>());
			lk::strcpy(BufferUnit,_T(""));
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					_stprintf(BufferValue, TEXT(NULLMEDIUM));
			} else {
				value=Units::ToHorizontalSpeed(LKTraffic[LKTargetIndex].EIAS);
				if (value<0||value>999) value=0; else valid=true;
				_stprintf(BufferValue, TEXT("%d"),(int)value);
				_stprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			}
			break;


		// B249 Distance from the start sector, always available also after start
		case LK_START_DIST:
			if ( ValidTaskPoint(0) && ValidTaskPoint(1) ) { // if real task
				if((ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute())&& ActiveTaskPoint == 0) {
					value=Units::ToDistance(WayPointCalc[RESWP_OPTIMIZED].Distance);
					if (value>99 || value==0)
						_stprintf(BufferValue, TEXT("%.0f"),value);
					else {
						if (value>10) {
							_stprintf(BufferValue, TEXT("%.1f"),value);
						} else 
							_stprintf(BufferValue, TEXT("%.3f"),value);
					}
				}
				else {
					index = Task[0].Index;
					if (index>=0) {
						value=Units::ToDistance(DerivedDrawInfo.WaypointDistance-StartRadius);
						if (value<0) value*=-1; // 101112 BUGFIX
						valid=true;
						if (value>99 || value==0)
							_stprintf(BufferValue, TEXT("%.0f"),value);
						else {
							if (value>10) {
								_stprintf(BufferValue, TEXT("%.1f"),value);
							} else 
								_stprintf(BufferValue, TEXT("%.3f"),value);
						}
					} else {
						_stprintf(BufferValue, TEXT(NULLMEDIUM)); // 091221
					}
				}
			} else {
				_stprintf(BufferValue, TEXT(NULLMEDIUM)); // 091221
			}
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			// LKTOKEN  _@M1192_ = "StDis"
			lk::strcpy(BufferTitle, MsgToken<1192>());
			break;

		// B250
		case LK_NEXT_CENTER_ALTDIFF:
			_stprintf(BufferValue,_T(NULLLONG));
			// LKTOKEN  _@M1025_ = "Next Alt.Arrival", _@M1026_ = "NxtArr"
			lk::strcpy(BufferTitle, DataOptionsTitle(LK_NEXT_ALTDIFF));
			lk::strcpy(BufferUnit, Units::GetAltitudeName());
            
            LockTaskData();
			if ( ValidTaskPoint(ActiveTaskPoint) != false ) {
				index = Task[ActiveTaskPoint].Index;
				if (index>=0) {
					// don't use current MC...
					value=Units::ToAltitude(WayPointCalc[index].AltArriv[AltArrivMode]);
					if ( value > ALTDIFFLIMIT ) {
						valid=true;
						_stprintf(BufferValue,TEXT("%+1.0f"), value);
					}
				}
			}
            UnlockTaskData();
			break;

		// B251
		case LK_NEXT_CENTER_GR:
			_stprintf(BufferValue,_T(NULLLONG));
			// LKTOKEN  _@M1145_ = "Next Req.Efficiency", _@M1146_ = "Req.E"
			lk::strcpy(BufferTitle, MsgToken<1146>());
            LockTaskData();
			if ( ValidTaskPoint(ActiveTaskPoint) != false ) {
				index = Task[ActiveTaskPoint].Index;
				if (index>=0) {
					value=WayPointCalc[index].GR;
					if (value <1 || value >=ALTERNATE_MAXVALIDGR )
						_stprintf(BufferValue, TEXT(NULLMEDIUM));
					else {
						if (value >= 100) _stprintf(BufferValue, TEXT("%.0lf"),value);
							else _stprintf(BufferValue, TEXT("%.1lf"),value);
						valid=true;
					}
				}
			}
            UnlockTaskData();
			break;
        // B252
        case LK_START_SPEED:
            valid = false;
            value = 0;
            // Req. Speed For reach Gate
            if (UseGates() && HaveGates()) {
                // Time To Gate
                const int gatechrono = GateTime(ActiveGate) - LocalTime(DrawInfo.Time); // not always already set, update it ... 
                if (gatechrono > 0) {
                    const double DistToGate = WayPointCalc[DoOptimizeRoute() ? RESWP_OPTIMIZED : Task[0].Index].Distance;
                    const double SpeedToGate = DistToGate / gatechrono;
                    const int RoundedSpeed = iround(Units::ToHorizontalSpeed(SpeedToGate));
                    if (SpeedToGate > 300) {
                        // ignore too fast speed
                        lk::strcpy(BufferValue, infinity);
                        lk::strcpy(BufferUnit, TEXT(""));
                    } else if (RoundedSpeed <= 0) {
                        lk::strcpy(BufferValue, TEXT(NULLMEDIUM));
                        lk::strcpy(BufferUnit, TEXT(""));
                    } else {
                        _stprintf(BufferValue, TEXT("%d"), RoundedSpeed);
                        _stprintf(BufferUnit, TEXT("%s"), (Units::GetHorizontalSpeedName()));
                    }
                    valid = true;
                }
                lk::strcpy(BufferTitle, TEXT(""));
            }
			break;

		// B253
		case LK_START_ALT:
			value = Units::ToAltitude(DerivedDrawInfo.TaskStartAltitude);
			if (value > 0) {
				_stprintf(BufferValue, TEXT("%d"), (int)value);
				_stprintf(BufferUnit, TEXT("%s"), (Units::GetAltitudeName()));
				valid = true;
			} else {
				lk::strcpy(BufferValue, TEXT("---"));
				lk::strcpy(BufferUnit, TEXT(""));
				valid = false;
			}
			// LKTOKEN _@M1200_ "Start"
			lk::strcpy(BufferTitle, MsgToken<1200>());
			break;

		case LK_SAT:
			valid = true;
			lk::strcpy(BufferUnit, TEXT(""));
			if (SIMMODE) {
				// LKTOKEN _@M1199_ "Sat"
				_stprintf(BufferTitle, TEXT("%s:?"), MsgToken<1199>());
				_stprintf(BufferValue, TEXT("SIM"));
			} else {
				value = DrawInfo.SatellitesUsed;
				if (value < 1 || value > 30) {
					_stprintf(BufferValue, TEXT("---"));
				} else {
					_stprintf(BufferValue, TEXT("%d"), (int)value);
				}
				for (const auto& dev : DeviceList) {
					if (dev.nmeaParser.activeGPS) {
						// LKTOKEN _@M1199_ "Sat"
						_stprintf(BufferTitle, _T("%s:%c"), MsgToken<1199>(), devLetter(dev.PortNumber));
						break;  // we have got the first active port.
					}
				}
			}
			break;

		case LK_HBAR_AVAILABLE:
			// LKTOKEN _@M1068_ "HBAR"
			lk::strcpy(BufferTitle, MsgToken<1068>());
			if (BaroAltitudeAvailable(DrawInfo)) {
				if (EnableNavBaroAltitude) {
					// LKTOKEN _@M894_ "ON"
					lk::strcpy(BufferValue, MsgToken<894>());
				} else {
					// LKTOKEN _@M491_ "OFF"
					lk::strcpy(BufferValue, MsgToken<491>());
				}
			} else {
				_stprintf(BufferValue, TEXT("---"));
			}
			valid = false;
			break;

		case LK_CPU:
			lk::strcpy(BufferTitle, _T("CPU"));
			value = CpuSummary();
			if (value >= 0 && value <= 100) {
				valid = true;
				_stprintf(BufferValue, _T("%d"), static_cast<int>(value));
				lk::strcpy(BufferUnit, _T("%"));
			} else {
				valid = false;
				lk::strcpy(BufferValue, TEXT("---"));
			}
			break;

		case LK_TRIP_SPEED_AVG:
			lk::strcpy(BufferTitle, MsgToken<2091>());  // Averag (speed average really)
			{
				int totime = (int)(Trip_Steady_Time + Trip_Moving_Time);
				if (totime > 0) {
					_stprintf(BufferValue, _T("%.1f"), Units::ToHorizontalSpeed(DerivedDrawInfo.Odometer) / totime);
					_stprintf(BufferUnit, TEXT("%s"), (Units::GetHorizontalSpeedName()));
					valid = true;
				} else {
					_stprintf(BufferValue, TEXT("---"));
					valid = false;
				}
			}
			break;

		case LK_TRIP_STEADY:
			lk::strcpy(BufferTitle, MsgToken<1810>());  // Steady
			if (DerivedDrawInfo.Flying) {
				valid = Units::TimeToTextDown(BufferValue, (int)Trip_Steady_Time);
				if (valid)
					lk::strcpy(BufferUnit, _T("h"));
				else
					lk::strcpy(BufferUnit, _T(""));
				valid = true;
			} else {
				_stprintf(BufferValue, TEXT("--:--"));
				valid = false;
			}
			break;

		case LK_TRIP_TOTAL:
			lk::strcpy(BufferTitle, MsgToken<1811>());  // Total
			if (DerivedDrawInfo.Flying) {
				valid = Units::TimeToTextDown(BufferValue, (int)(Trip_Steady_Time + Trip_Moving_Time));
				if (valid)
					lk::strcpy(BufferUnit, _T("h"));
				else
					lk::strcpy(BufferUnit, _T(""));
				valid = true;
			} else {
				_stprintf(BufferValue, TEXT("--:--"));
				valid = false;
			}
			break;

		case LK_SPEED_AVG:
			_stprintf(BufferValue, _T("%.1f"), Units::ToHorizontalSpeed(DerivedDrawInfo.AverageGS));
			_stprintf(BufferTitle, _T("AvgSpd"));
			_stprintf(BufferUnit, TEXT("%s"), (Units::GetHorizontalSpeedName()));
			valid = true;
			break;

		case LK_DIST_AVG:
			if (DerivedDrawInfo.AverageDistance < 100000) {
				_stprintf(BufferValue, _T("%.2f"), Units::ToDistance(DerivedDrawInfo.AverageDistance));
			} else {
				_stprintf(BufferValue, _T("%.2f"), 0.0);
			}
			_stprintf(BufferTitle, _T("AvgDist"));
			_stprintf(BufferUnit, TEXT("%s"), (Units::GetDistanceName()));
			valid = true;
			break;

		case LK_HEART_RATE:
			if (HeartRateAvailable(DrawInfo)) {
				valid = true;
				_stprintf(BufferValue, _T("%u"), DrawInfo.HeartRate);
			} else {
				valid = false;
				_stprintf(BufferValue, TEXT("---"));
			}
			_stprintf(BufferUnit, TEXT("bpm"));
			lk::strcpy(BufferTitle, DataOptionsTitle(LK_HEART_RATE));
			break;

        case LK_AVERAGE_GS:
            if (DerivedDrawInfo.AverageGS>0) {
                value=Units::ToHorizontalSpeed(DerivedDrawInfo.AverageGS);
                lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
            } else {
                value=Units::ToHorizontalSpeed(DrawInfo.Speed);
                lk::strcpy(BufferTitle, MsgToken<2504>());
            }
            valid=true;
            if (value<0||value>9999) value=0; else valid=true;
            _stprintf(BufferValue, TEXT("%.0f"),value);
            _stprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
            break;

        case LK_NEXT_AVG_ETE:
            lk::strcpy(BufferValue,_T(NULLLONG));
            _stprintf(BufferTitle, TEXT("%s"), DataOptionsTitle(lkindex));
            value = GetAvgNextETE(DerivedDrawInfo);
            if (value > 0) {
                valid = true;
                if (Units::TimeToTextDown(BufferValue, value))  // 091112
                    lk::strcpy(BufferUnit, TEXT("h"));
                else
                    lk::strcpy(BufferUnit, TEXT("m"));
            } else {
                lk::strcpy(BufferValue, TEXT(NULLTIME));
            }
            break;

        case LK_NEXT_AVG_ETA:
            lk::strcpy(BufferValue,_T(NULLLONG));
            _stprintf(BufferTitle, TEXT("%s"), DataOptionsTitle(lkindex));
            value = GetAvgNextETE(DerivedDrawInfo);
            if(value > 0) {
                valid = true;
                Units::TimeToText(BufferValue, value + LocalTime(DrawInfo.Time));
            } else {
                lk::strcpy(BufferValue, TEXT(NULLTIME));
            }
            break;


		case LK_DUMMY:
			_stprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("Dummy"));
			else
				_stprintf(BufferTitle, TEXT("Dummy"));

			_stprintf(BufferUnit, TEXT("."));
			break;

		case LK_EMPTY:
			lk::strcpy(BufferValue, TEXT(""));
			lk::strcpy(BufferUnit, TEXT(""));
			lk::strcpy(BufferTitle, TEXT(""));
			break;

		case LK_ERROR:
lk_error:
			// let it be shown entirely to understand the problem
			valid=true;
			_stprintf(BufferValue, TEXT("000"));
			_stprintf(BufferUnit, TEXT("e"));
			_stprintf(BufferTitle, TEXT("Err"));
			break;

		default:
			valid=false;
			_stprintf(BufferValue, TEXT(NULLMEDIUM));
			_stprintf(BufferUnit, TEXT("."));
			if ( lkindex >=NumDataOptions || lkindex <1 )  // Notice NumDataOptions check!
				_stprintf(BufferTitle, TEXT("BadErr"));
			else
				lk::strcpy(BufferTitle, DataOptionsTitle(lkindex));;
			break;
	}

  return valid;
}

static int GetValidWayPointIndex(int wpindex) {
    if (wpindex <= RESWP_END) {
        if (ValidResWayPoint(wpindex)) {
            return wpindex;
        }
    } else {
        if (ValidWayPointFast(wpindex)) {
            return wpindex;
        }
    }
    return -1;
}

// simple format distance value for a given index. BufferTitle always NULLed
// wpindex is a WayPointList index
void MapWindow::LKFormatDist(const int wpindex, TCHAR *BufferValue, TCHAR *BufferUnit) {
  ScopeLock lock(CritSec_TaskData);

  int index = GetValidWayPointIndex(wpindex);

  if (index>=0) {
	double value=Units::ToDistance(WayPointCalc[index].Distance);
	if (value<0.001) value=0;
	if (value>99 || value==0)
		_stprintf(BufferValue, TEXT("%.0f"),value);
	else {
		if (ISPARAGLIDER) {
			if (value>10)
				_stprintf(BufferValue, TEXT("%.1f"),value);
			else 
				_stprintf(BufferValue, TEXT("%.3f"),value);
		} else {
			_stprintf(BufferValue, TEXT("%.1f"),value);
		}
	}
  } else {
	_stprintf(BufferValue, TEXT(NULLMEDIUM));
  }
  _stprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
}

// DO NOT use this for AAT values! 
void MapWindow::LKFormatBrgDiff(const int wpindex, TCHAR *BufferValue, TCHAR *BufferUnit) {
  ScopeLock lock(CritSec_TaskData);

  int index = GetValidWayPointIndex(wpindex);

  _tcscpy(BufferValue,_T(NULLMEDIUM));
  _tcscpy(BufferUnit,_T(""));
  if (index>=0) {
    // Warning, for AAT this should be WaypointBearing, so do not use it!
    double value = WayPointCalc[index].Bearing -  DrawInfo.TrackBearing;
    if (value < -180.0)
        value += 360.0;
    else
        if (value > 180.0)
            value -= 360.0;
    if (value > 30)
      _stprintf(BufferValue, TEXT("%2.0f%s%s"), value, MsgToken<2179>(), MsgToken<2183>());
    else
      if (value > 2)
        _stprintf(BufferValue, TEXT("%2.0f%s%s"), value, MsgToken<2179>(), MsgToken<2185>());
      else
        if (value < -30)
          _stprintf(BufferValue, TEXT("%s%2.0f%s"), MsgToken<2182>(), -value, MsgToken<2179>());
        else
          if (value < -2)
            _stprintf(BufferValue, TEXT("%s%2.0f%s"), MsgToken<2184>(), - value, MsgToken<2179>());
          else
            _stprintf(BufferValue, TEXT("%s%s"), MsgToken<2182>(), MsgToken<2183>());
  }
}


void MapWindow::LKFormatGR(const int wpindex, TCHAR *BufferValue, TCHAR *BufferUnit) {
  ScopeLock lock(CritSec_TaskData);

  int index = GetValidWayPointIndex(wpindex);

  _tcscpy(BufferValue,_T(NULLMEDIUM));
  _tcscpy(BufferUnit,_T(""));

  double value;
  if (index>=0) {
	value=WayPointCalc[index].GR;
  } else {
	value=INVALID_GR;
  }

  if (value >=1 && value <MAXEFFICIENCYSHOW ) {
	if (value >= 100)
		_stprintf(BufferValue, TEXT("%.0lf"),value);
	else
		_stprintf(BufferValue, TEXT("%.1lf"),value);

  }
}

void MapWindow::LKFormatAltDiff(const int wpindex, TCHAR *BufferValue, TCHAR *BufferUnit) {
  ScopeLock lock(CritSec_TaskData);

  int index = GetValidWayPointIndex(wpindex);

  _tcscpy(BufferValue,_T(NULLMEDIUM));
  _stprintf(BufferUnit, _T("%s"),(Units::GetAltitudeName()));

  double value;
  if (index>=0) {
	value=Units::ToAltitude(WayPointCalc[index].AltArriv[AltArrivMode]);
  } else {
	value=INVALID_DIFF;
  }

  if (value > ALTDIFFLIMIT ) {
	if ( value>-1 && value<1 )
		_stprintf(BufferValue, TEXT("0"));
	else
		_stprintf(BufferValue, TEXT("%+.0f"),value);
  }
}


// This is called by Draw thread, at each run every second. 
// It is a simple interface to the OLC engine to make all results
// globals, and cooked.
// Instead of locking and accessing the class each time, we do it ONCE
void MapWindow::LKUpdateOlc(void)
{
  static short loop=0;

  ONEHZLIMITER; // this is consolidated already

  // get one result each second
  CContestMgr::TType type = (CContestMgr::TType)(loop++ % CContestMgr::TYPE_NUM);
  OlcResults[type] = CContestMgr::Instance().Result(type, false);
  if(loop == CContestMgr::TYPE_NUM)
    loop = 0;
}

