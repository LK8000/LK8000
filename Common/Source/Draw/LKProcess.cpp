/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKProcess.cpp,v 1.8 2010/12/11 19:32:14 root Exp root $
*/

#include "externs.h"
#include "McReady.h"
#include "InputEvents.h"
#include "InfoBoxLayout.h"
#include "Logger.h"
#include "LKProcess.h"
#include "DoInits.h"


// #define NULLSHORT	"--" 
#define NULLMEDIUM	"---"
#define NULLLONG	"---"
#define NULLTIME	"--:--"
#define INFINVAL	"oo"

TCHAR *WindAngleToText(double angle);

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
bool MapWindow::LKFormatValue(const short lkindex, const bool lktitle, TCHAR *BufferValue, TCHAR *BufferUnit, TCHAR *BufferTitle) {

  static int	index=-1;
  static double value;
  static int	ivalue;
  static char	text[LKSIZETEXT];
  static TCHAR	varformat[10];

   BOOL bFAI ;
   double fDist ;
   double fTogo ;
  // By default, invalid return value. Set it to true after assigning value in cases
  bool		valid=false;

  if (DoInit[MDI_LKPROCESS]) {

	if (LIFTMODIFY==TOFEETPERMINUTE)
		_stprintf(varformat,TEXT("%%+.0f"));
	else
		_stprintf(varformat,TEXT("%%+0.1f"));

	DoInit[MDI_LKPROCESS]=false;
  }


	_tcscpy(BufferUnit,_T(""));

	switch(lkindex) {


		// B00
		case LK_HNAV:
			if (lktitle)
				// LKTOKEN  _@M1001_ = "Altitude QNH", _@M1002_ = "Alt", 
				_stprintf(BufferTitle, MsgToken(1002));
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
				// LKTOKEN  _@M1003_ = "Altitude AGL", _@M1004_ = "HAGL"
				_stprintf(BufferTitle, MsgToken(1004));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if (!DerivedDrawInfo.TerrainValid) { 
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

		// B02 091221
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
				// LKTOKEN  _@M1005_ = "Thermal last 30 sec", _@M1006_ = "TC.30\""
				wsprintf(BufferTitle, MsgToken(1006));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;
		// B03
		case LK_BRG:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1007_ = "Bearing", _@M1008_ = "Brg"
				_stprintf(BufferTitle, MsgToken(1008));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
goto_bearing:
					// we could use only waypointbearing, but lets keep them separated anyway
					if (AATEnabled)
						value=DerivedDrawInfo.WaypointBearing;
					else
						value = WayPointCalc[index].Bearing;

					valid=true;
#ifndef __MINGW32__
					if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0f\xB0"), value);			//Source editor encoding problem fixed
					else if (value < -1)
						_stprintf(BufferValue, TEXT("%2.0f\xB0"), -value);
						else
							_tcscpy(BufferValue, TEXT("0\xB0"));
#else
					if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0f°"), value);
					else if (value < -1)
						_stprintf(BufferValue, TEXT("%2.0f°"), -value);
						else
							_tcscpy(BufferValue, TEXT("0°"));
#endif
				}
			}
			break;


		// B04
		case LK_LD_INST:
                        wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1009_ = "Eff.last 20 sec", _@M1010_ = "E.20\""
				_stprintf(BufferTitle, MsgToken(1010));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if (DerivedDrawInfo.Flying)
				value=DerivedDrawInfo.LD;
			else
				value=0;
			if (value <-99 ||  value >=ALTERNATE_MAXVALIDGR ) {
				strcpy(text,INFINVAL);
				valid=true;
			} else
				if (value==0) sprintf(text,NULLMEDIUM);
				else {
					sprintf(text,"%.1f",value);
					valid=true;
				}
			wsprintf(BufferValue, TEXT("%S"),text);
			break;


		// B05
		case LK_LD_CRUISE:
			if (lktitle)
				// LKTOKEN  _@M1011_ = "Eff.cruise last therm", _@M1012_ = "E.Cru"
				_stprintf(BufferTitle, MsgToken(1012));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if (DerivedDrawInfo.Flying)
				value=DerivedDrawInfo.CruiseLD;
			else
				value=0;
			if (value <-99 ||  value >=ALTERNATE_MAXVALIDGR ) {
				strcpy(text,INFINVAL); 
				valid=true;
			} else
			if (value==0) sprintf(text,NULLMEDIUM);
			else {
				sprintf(text,"%.1f",value);
				valid=true;
			}
			wsprintf(BufferValue, TEXT("%S"),text);
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
				// LKTOKEN  _@M1013_ = "Speed ground", _@M1014_ = "GS"
				wsprintf(BufferTitle, MsgToken(1014));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;


		// B07  091221
		case LK_TL_AVG:
			value= LIFTMODIFY*DerivedDrawInfo.LastThermalAverage;
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
				// LKTOKEN  _@M1015_ = "Thermal Average Last", _@M1016_ = "TL.Avg"
				wsprintf(BufferTitle, MsgToken(1016));
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
				// LKTOKEN  _@M1017_ = "Thermal Gain Last", _@M1018_ = "TL.Gain"
				wsprintf(BufferTitle, MsgToken(1018));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;
		// B09		Last thermal time
		case LK_TL_TIME:
			wsprintf(BufferValue,_T(NULLTIME));
			if (lktitle)
				// LKTOKEN  _@M1019_ = "Thermal Time Last", _@M1020_ = "TL.Time"
				_stprintf(BufferTitle, MsgToken(1020));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if (DerivedDrawInfo.LastThermalTime > 0) {
				valid=true;
				Units::TimeToText(BufferValue, (int)DerivedDrawInfo.LastThermalTime);
			} else {
				wsprintf(BufferValue, TEXT(NULLTIME));
			}
			wsprintf(BufferUnit, TEXT("h"));
			break;


		// B10
		case LK_MC:
			value = iround(LIFTMODIFY*MACCREADY*10)/10.0;
			valid=true;
			//sprintf(text,"%.1lf",value);
			sprintf(text,"%2.1lf",value);
			wsprintf(BufferValue, TEXT("%S"),text);
			//if (!ValidTaskPoint(ActiveWayPoint) && ((AutoMcMode==0) || (AutoMcMode==2))) {
			if (!DerivedDrawInfo.AutoMacCready) {
				if (lktitle)
					// LKTOKEN  _@M1183_ = "ManMC"
					wsprintf(BufferTitle, MsgToken(1183));
				else
					_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			} else {
				if (lktitle)
					// LKTOKEN  _@M1184_ = "AutMC"
					wsprintf(BufferTitle, MsgToken(1184));
				else
					_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			}

			break;

		// B11
		case LK_NEXT_DIST:
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
			   if (ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute()) {
				value=WayPointCalc[RESWP_OPTIMIZED].Distance*DISTANCEMODIFY;
				valid=true;
				if (value>99)
					sprintf(text,"%.0f",value);
				else {
					if (ISPARAGLIDER) {
						if (value>10)
							sprintf(text,"%.1f",value);
						else 
							sprintf(text,"%.2f",value);
					} else {
						sprintf(text,"%.1f",value);
					}
				}
			   } else {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value=DerivedDrawInfo.WaypointDistance*DISTANCEMODIFY;
					valid=true;
					if (value>99 || value==0)
						sprintf(text,"%.0f",value);
					else {
						if (ISPARAGLIDER) {
							if (value>10)
								sprintf(text,"%.1f",value);
							else 
								sprintf(text,"%.2f",value);
						} else {
							sprintf(text,"%.1f",value);
						}
					}
				} else {
					strcpy(text,NULLMEDIUM); // 091221
				}
			   }
			} else {
				strcpy(text,NULLMEDIUM); // 091221
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
				// LKTOKEN  _@M1023_ = "Next Distance", _@M1024_ = "Dist"
				_tcscpy(BufferTitle, MsgToken(1024));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;
		// B12
		// Arrival altitude using current MC  and total energy. Does not use safetymc.
		// total energy is disabled!
		case LK_NEXT_ALTDIFF:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1025_ = "Next Alt.Arrival", _@M1026_ = "NxtArr"
				_stprintf(BufferTitle, MsgToken(1026));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
            LockTaskData();
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				if (ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute()) index=RESWP_OPTIMIZED;
				else index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					// don't use current MC...
					value=ALTITUDEMODIFY*WayPointCalc[index].AltArriv[AltArrivMode];
					if ( value > ALTDIFFLIMIT ) {
						valid=true;
						_stprintf(BufferValue,TEXT("%+1.0f"), value);
					}
				}
			}
            UnlockTaskData();
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B13
		// Using MC! 
		case LK_NEXT_ALTREQ:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1027_ = "Next Alt.Required", _@M1028_ = "NxtAltR"
				_stprintf(BufferTitle, MsgToken(1028));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
            LockTaskData();
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				if (ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute()) index=RESWP_OPTIMIZED;
				else index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value=ALTITUDEMODIFY*WayPointCalc[index].AltReqd[AltArrivMode];
					if (value<10000 && value >-10000) {
						_stprintf(BufferValue,TEXT("%1.0f"), value);
						valid=true;
					} 
				}
			}
            UnlockTaskData();
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B14 EMPTY UNUSED
		case LK_NEXT_WP:
			goto lk_error;
			break;

		// B15 Arrival altitude , no more total energy
		case LK_FIN_ALTDIFF:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1031_ = "Task Alt.Arrival", _@M1032_ = "TskArr"
				_stprintf(BufferTitle, MsgToken(1032));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
            LockTaskData();
			if ( (ValidTaskPoint(ActiveWayPoint) != false) && DerivedDrawInfo.ValidStart ) {
				if (ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute()) index=RESWP_OPTIMIZED;
				else index = Task[ActiveWayPoint].Index;
				if (index>=0) {
                    if(ISPARAGLIDER && DerivedDrawInfo.TaskAltitudeDifference > 0.0) {
                        value=ALTITUDEMODIFY*DerivedDrawInfo.TaskAltitudeArrival;
                    } else {
    					value=ALTITUDEMODIFY*DerivedDrawInfo.TaskAltitudeDifference;
                    }
					if ( value > ALTDIFFLIMIT ) {
						valid=true;
						_stprintf(BufferValue,TEXT("%+1.0f"), value);
					}
				}
			}
            UnlockTaskData();
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B16
		case LK_FIN_ALTREQ:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1033_ = "Task Alt.Required", _@M1034_ = "TskAltR"
				_stprintf(BufferTitle, MsgToken(1034));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
            LockTaskData();
			if ( (ValidTaskPoint(ActiveWayPoint) != false) && DerivedDrawInfo.ValidStart ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value=ALTITUDEMODIFY*DerivedDrawInfo.TaskAltitudeRequired;
					if (value<10000 && value >-10000) {
						_stprintf(BufferValue,TEXT("%1.0f"), value);
						valid=true;
					} 
				}
			}
            UnlockTaskData();
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;
		// B17
		case LK_SPEEDTASK_AVG:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1035_ = "Task Speed Average", _@M1036_ = "TskSpAv"
				_stprintf(BufferTitle, MsgToken(1036));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			value=0;
			if ( ActiveWayPoint >=1) {
				if ( ValidTaskPoint(ActiveWayPoint) ) {
					value = TASKSPEEDMODIFY*DerivedDrawInfo.TaskSpeed;
					if (value<=0||value>999) value=0; else valid=true;
					if (value<99)
						sprintf(text,"%.1f",value);
					else
						sprintf(text,"%d",(int)value);
					wsprintf(BufferValue, TEXT("%S"),text);
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetTaskSpeedName()));
			break;


		// B18
		case LK_FIN_DIST:
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					if (DerivedDrawInfo.ValidFinish) {
						value = DISTANCEMODIFY*DerivedDrawInfo.WaypointDistance;
					} else {
						value = DISTANCEMODIFY*DerivedDrawInfo.TaskDistanceToGo;
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
				// LKTOKEN  _@M1037_ = "Task Distance", _@M1038_ = "TskDis"
				_tcscpy(BufferTitle, MsgToken(1038));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B19
		case LK_RESERVED1:
			goto lk_error;
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
				// LKTOKEN  _@M1041_ = "Terrain Elevation", _@M1042_ = "Gnd"
				wsprintf(BufferTitle, MsgToken(1042));
			else 
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;


		// B21 091221
		case LK_TC_AVG:
			value= LIFTMODIFY*DerivedDrawInfo.AverageThermal;
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
				// LKTOKEN  _@M1043_ = "Thermal Average", _@M1044_ = "TC.Avg"
				wsprintf(BufferTitle, MsgToken(1044));
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
				// LKTOKEN  _@M1045_ = "Thermal Gain", _@M1046_ = "TC.Gain"
				wsprintf(BufferTitle, MsgToken(1046));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B23
		case LK_TRACK:
			wsprintf(BufferValue,_T(NULLLONG));
			//_stprintf(BufferUnit,TEXT(""));
			if (lktitle)
				// LKTOKEN  _@M1047_ = "Track", _@M1048_ = "Track"
				_stprintf(BufferTitle, MsgToken(1048));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			value = DrawInfo.TrackBearing;
			valid=true;
#ifndef __MINGW32__
			if (value > 1)
				_stprintf(BufferValue, TEXT("%2.0f\xB0"), value);
			else if (value < -1)
				_stprintf(BufferValue, TEXT("%2.0f\xB0"), -value);
				else
					_tcscpy(BufferValue, TEXT("0\xB0"));
#else
			if (value > 1)
				_stprintf(BufferValue, TEXT("%2.0f°"), value);
			else if (value < -1)
				_stprintf(BufferValue, TEXT("%2.0f°"), -value);
				else
					_tcscpy(BufferValue, TEXT("0°"));
#endif
			break;


		// B24
		case LK_VARIO:
			if (DrawInfo.VarioAvailable) {
				value = LIFTMODIFY*DrawInfo.Vario;
			} else {
				value = LIFTMODIFY*DerivedDrawInfo.Vario;
			}
			valid=true;
			_stprintf(BufferValue,varformat,value);
			wsprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
			if (lktitle)
				// LKTOKEN  _@M1049_ = "Vario", _@M1050_ = "Vario"
				wsprintf(BufferTitle, MsgToken(1050));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
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


		// B27
		case LK_AA_TIME:
			wsprintf(BufferValue,_T(NULLTIME));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

#if (0)
			double dd;
			if (AATEnabled && ValidTaskPoint(ActiveWayPoint)) {
				dd = DerivedDrawInfo.TaskTimeToGo;
				if ((DerivedDrawInfo.TaskStartTime>0.0) && (DerivedDrawInfo.Flying) &&(ActiveWayPoint>0)) {
					dd += DrawInfo.Time-DerivedDrawInfo.TaskStartTime;
				}
				dd= max(0,min(24.0*3600.0,dd))-AATTaskLength*60;
				if (dd<0) {
					status = 1; // red
				} else {
					if (DerivedDrawInfo.TaskTimeToGoTurningNow > (AATTaskLength+5)*60) {
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
			if (ValidTaskPoint(ActiveWayPoint) && AATEnabled && (DerivedDrawInfo.AATTimeToGo< 0.9*ERROR_TIME)) {

				Units::TimeToText(BufferValue, (int)DerivedDrawInfo.AATTimeToGo);
				valid=true;
			}
			wsprintf(BufferUnit,_T("h"));
			break;


		// B28
		case LK_AA_DISTMAX:
		// B29
		case LK_AA_DISTMIN:
			if ( (ValidTaskPoint(ActiveWayPoint) != false) && AATEnabled ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					if ( lkindex == LK_AA_DISTMAX )
						value = DISTANCEMODIFY*DerivedDrawInfo.AATMaxDistance ;
					else
						value = DISTANCEMODIFY*DerivedDrawInfo.AATMinDistance ;
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
			if ( (ValidTaskPoint(ActiveWayPoint) != false) && AATEnabled && DerivedDrawInfo.AATTimeToGo>=1 ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					if ( lkindex == LK_AA_SPEEDMAX )
						value = TASKSPEEDMODIFY*DerivedDrawInfo.AATMaxSpeed;
					else
						value = TASKSPEEDMODIFY*DerivedDrawInfo.AATMinSpeed;

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


		// B32
		case LK_IAS:
			if (DrawInfo.AirspeedAvailable) {
				if (lktitle)
					// LKTOKEN  _@M1065_ = "Airspeed IAS", _@M1066_ = "IAS"
					wsprintf(BufferTitle, MsgToken(1066));
				else
					_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
				value=SPEEDMODIFY*DrawInfo.IndicatedAirspeed;
				if (value<0||value>999) value=0; else valid=true;
				sprintf(text,"%d",(int)value);
				wsprintf(BufferValue, TEXT("%S"),text);
			} else {
				// LKTOKEN  _@M1065_ = "Airspeed IAS", _@M1066_ = "IAS"
				wsprintf(BufferTitle, TEXT("e%s"), MsgToken(1066));
				value=SPEEDMODIFY*DerivedDrawInfo.IndicatedAirspeedEstimated;
				if (value<0||value>999) value=0; else valid=true;
				sprintf(text,"%d",(int)value);
				wsprintf(BufferValue, TEXT("%S"),text);
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			break;


		// B33
		case LK_HBARO:
			if (DrawInfo.BaroAltitudeAvailable) {
				value=ALTITUDEMODIFY*DrawInfo.BaroAltitude;
				valid=true;
				sprintf(text,"%d",(int)value);
				wsprintf(BufferValue, TEXT("%S"),text);
			} else
				wsprintf(BufferValue, TEXT(NULLLONG));
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
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
				// LKTOKEN  _@M1069_ = "Speed MacReady", _@M1070_ = "SpMc"
				wsprintf(BufferTitle, MsgToken(1070));
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

		// B36		TmFly
		case LK_TIMEFLIGHT:
			wsprintf(BufferValue,_T(NULLTIME));
			if (lktitle)
				// LKTOKEN  _@M1073_ = "Time of flight", _@M1074_ = "FlyTime"
				_stprintf(BufferTitle, MsgToken(1074));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if (DerivedDrawInfo.FlightTime > 0) {
				valid=true;
				Units::TimeToText(BufferValue, (int)DerivedDrawInfo.FlightTime);
			} else {
				wsprintf(BufferValue, TEXT(NULLTIME));
			}
			wsprintf(BufferUnit, TEXT("h"));
			break;


		// B37
		case LK_GLOAD:
			//wsprintf(BufferValue,_T(NULLMEDIUM)); 100302 obs
			//wsprintf(BufferUnit,_T("g")); 100302 obs
			// _stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title ); 100302 obs
			if ( DrawInfo.AccelerationAvailable) { 
				value=DrawInfo.AccelZ;
				_stprintf(BufferValue,TEXT("%+.1f"), value);
				valid=true;
				// LKTOKEN  _@M1075_ = "G load", _@M1076_ = "G"
				_tcscpy(BufferTitle, MsgToken(1076));
			} else {
				value=DerivedDrawInfo.Gload;
				_stprintf(BufferValue,TEXT("%+.1f"), value);
				valid=true;
				_stprintf(BufferTitle, TEXT("e%s"), MsgToken(1076));
			}
			break;

		// B38
		case LK_MTG_BRG:
			goto lk_empty;
			break;

		// B39
		case LK_TIME_LOCAL:
			Units::TimeToText(BufferValue, (int)DetectCurrentTime());
			valid=true;
			if (lktitle)
				// LKTOKEN  _@M1079_ = "Time local", _@M1080_ = "Time"
				_stprintf(BufferTitle, MsgToken(1080));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			break;

		// B40
		case LK_TIME_UTC:
			Units::TimeToText(BufferValue,(int) DrawInfo.Time);
			valid=true;
			if (lktitle)
				// LKTOKEN  _@M1081_ = "Time UTC", _@M1082_ = "UTC"
				_stprintf(BufferTitle, MsgToken(1082));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			break;


		// B41	091006 using new task ete 
		case LK_FIN_ETE:
			goto lkfin_ete;


		// B42
		case LK_NEXT_ETE:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1085_ = "Next Time To Go", _@M1086_ = "NextETE"
				_stprintf(BufferTitle, MsgToken(1086));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

            LockTaskData();
			index = (ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute())?RESWP_OPTIMIZED:Task[ActiveWayPoint].Index;
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
            UnlockTaskData();
			// wsprintf(BufferUnit, TEXT("h")); 091112
			break;


		// B43 AKA STF
		case LK_SPEED_DOLPHIN:
			// if (DrawInfo.AirspeedAvailable) {
				value=SPEEDMODIFY*DerivedDrawInfo.VOpt;
				if (value<0||value>999) value=0; else valid=true;
				sprintf(text,"%d",(int)value);
				wsprintf(BufferValue, TEXT("%S"),text);
			// } else
			//	wsprintf(BufferValue, TEXT(NULLMEDIUM));
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;
				
		// B44
		case LK_NETTO:
			value=LIFTMODIFY*DerivedDrawInfo.NettoVario;
			if (value<-100||value>100) value=0; else valid=true;
			_stprintf(BufferValue,varformat,value);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetVerticalSpeedName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

				
		// B45
		case LK_FIN_ETA:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1091_ = "Task Arrival Time", _@M1092_ = "TskETA"
				_stprintf(BufferTitle, MsgToken(1092));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if (ISCAR || ISGAAIRCRAFT) {
			    if ( ValidTaskPoint(ActiveWayPoint) && DerivedDrawInfo.ValidStart ) {
				if (DerivedDrawInfo.LKTaskETE > 0) {
					valid=true;
					Units::TimeToText(BufferValue, (int)DerivedDrawInfo.LKTaskETE+DetectCurrentTime());
				} else
					wsprintf(BufferValue, TEXT(NULLTIME));
			    }
			} else {
			    if ( (ValidTaskPoint(ActiveWayPoint) != false) && DerivedDrawInfo.ValidStart && (DerivedDrawInfo.TaskTimeToGo< 0.9*ERROR_TIME)) {
				if (DerivedDrawInfo.TaskTimeToGo > 0) {
					valid=true;
					Units::TimeToText(BufferValue, (int)DerivedDrawInfo.TaskTimeToGo+DetectCurrentTime());
				} else
					wsprintf(BufferValue, TEXT(NULLTIME));
			    }
			}
			wsprintf(BufferUnit, TEXT("h"));
			break;


		// B46
		case LK_NEXT_ETA:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1093_ = "Next Arrival Time", _@M1094_ = "NextETA"
				_stprintf(BufferTitle, MsgToken(1094));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if(ISPARAGLIDER) {
                                LockTaskData();
				index = DoOptimizeRoute()?RESWP_OPTIMIZED:Task[ActiveWayPoint].Index;
				if ( (ValidTaskPoint(ActiveWayPoint) != false) && (WayPointCalc[index].NextETE < 0.9*ERROR_TIME)) {
					if (WayPointCalc[index].NextETE > 0) {
						valid=true;
						Units::TimeToText(BufferValue, (int)(WayPointCalc[index].NextETE+DetectCurrentTime()));
					} else
						wsprintf(BufferValue, TEXT(NULLTIME));
				}
                                UnlockTaskData();
                                break;
			} 
                        if (ISCAR || ISGAAIRCRAFT) { 
                            LockTaskData();
			    if ( ValidTaskPoint(ActiveWayPoint) && (WayPointCalc[TASKINDEX].NextETE< 0.9*ERROR_TIME)) {
				if (WayPointCalc[TASKINDEX].NextETE > 0) {
					valid=true;
					Units::TimeToText(BufferValue, (int)(WayPointCalc[TASKINDEX].NextETE+DetectCurrentTime()));
				} else
					wsprintf(BufferValue, TEXT(NULLTIME));
			    }
                            UnlockTaskData();
                            break;
			}

			if ( (ValidTaskPoint(ActiveWayPoint) != false) && (DerivedDrawInfo.LegTimeToGo< 0.9*ERROR_TIME)) {
				if (DerivedDrawInfo.LegTimeToGo > 0) {
					valid=true;
					Units::TimeToText(BufferValue, (int)DerivedDrawInfo.LegTimeToGo+DetectCurrentTime());
				} else
					wsprintf(BufferValue, TEXT(NULLTIME));
			}
			break;
		// B47
		case LK_BRGDIFF:
			wsprintf(BufferValue,_T(NULLMEDIUM)); // 091221
			if (lktitle)
				// LKTOKEN  _@M1095_ = "Bearing Difference", _@M1096_ = "To"
				_stprintf(BufferTitle, MsgToken(1096));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				if (ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute()) index=RESWP_OPTIMIZED;
				else index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					// THIS WOULD SET BEARING while circling
					// if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING))
					if (true)
					{
						if (AATEnabled && !DoOptimizeRoute())
							value=DerivedDrawInfo.WaypointBearing -  DrawInfo.TrackBearing;
						else
							value = WayPointCalc[index].Bearing -  DrawInfo.TrackBearing;
						valid=true;
						if (value < -180.0)
							value += 360.0;
						else
							if (value > 180.0)
								value -= 360.0;
#ifndef __MINGW32__
						if (value > 1)
							_stprintf(BufferValue, TEXT("%2.0f\xB0\xBB"), value);
						else if (value < -1)
							_stprintf(BufferValue, TEXT("\xAB%2.0f\xB0"), -value);
							else
								_tcscpy(BufferValue, TEXT("\xAB\xBB"));
#else

              if (value > 30)
                _stprintf(BufferValue, TEXT("%2.0f°»"), value);
              else
                if (value > 2)
                  _stprintf(BufferValue, TEXT("%2.0f°›"), value);
                else
                  if (value < -30)
                    _stprintf(BufferValue, TEXT("«%2.0f°"), -value);
                  else
                    if (value < -2)
                      _stprintf(BufferValue, TEXT("‹%2.0f°"),- value);
                    else
                      _stprintf(BufferValue, TEXT("«»"));
                  //    _stprintf(BufferValue, TEXT("‹%2.0f°›"),0 /*fabs(value)*/);
#endif
					}
					else goto goto_bearing;
				}
			}
			break;


		// B48 091216  OAT Outside Air Temperature
		case LK_OAT:
                  _stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
                  if (!DrawInfo.TemperatureAvailable || value<-50||value>100) {
                    wsprintf(BufferValue, TEXT("---"));
                  }
                  else {
                    value = DrawInfo.OutsideAirTemperature;
                    sprintf(text,"%.0lf",value);
                    wsprintf(BufferValue, TEXT("%S"), text);
                    wsprintf(BufferUnit,  TEXT("%S"), _T(DEG));
                    valid = true;
                  }
                  break;
                  
		// B49
		case LK_RELHUM:
                  _stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
                  if(DrawInfo.HumidityAvailable) {
                    value = DrawInfo.RelativeHumidity;
                    sprintf(text, "%.0lf", value);
                    wsprintf(BufferValue, TEXT("%S"), text);
                    wsprintf(BufferUnit, TEXT("%%"));
                    valid = true;
                  }
                  else
                    wsprintf(BufferValue, TEXT("---"));
                  break;

		// B50 UNSUPPORTED
		case LK_MAXTEMP:
			goto lk_error;
			break;

		// B51
		case LK_AA_TARG_DIST:
			if ( (ValidTaskPoint(ActiveWayPoint) != false) && AATEnabled ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value = DISTANCEMODIFY*DerivedDrawInfo.AATTargetDistance ;
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


		// B52
		case LK_AA_TARG_SPEED:
			if ( (ValidTaskPoint(ActiveWayPoint) != false) && AATEnabled && DerivedDrawInfo.AATTimeToGo>=1 ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					value = TASKSPEEDMODIFY*DerivedDrawInfo.AATTargetSpeed;
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


		// B53
		case LK_LD_VARIO:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			//_stprintf(BufferUnit,TEXT(""));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if (DrawInfo.AirspeedAvailable && DrawInfo.VarioAvailable) {
				value = DerivedDrawInfo.LDvario;
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


		// B54 091221
		case LK_TAS:
			if (DrawInfo.AirspeedAvailable) {
				if (lktitle)
					// LKTOKEN  _@M1109_ = "Airspeed TAS", _@M1110_ = "TAS"
					wsprintf(BufferTitle, MsgToken(1110));
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
				// LKTOKEN  _@M1109_ = "Airspeed TAS", _@M1110_ = "TAS"
				wsprintf(BufferTitle, TEXT("e%s"), MsgToken(1110));
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
				LK_tcsncpy(BufferValue,DerivedDrawInfo.OwnTeamCode,5);
				valid=true; // 091221
			} else
				wsprintf(BufferValue,_T("----"));
			if (lktitle)
				// LKTOKEN  _@M1111_ = "Team Code", _@M1112_ = "TeamCode"
				wsprintf(BufferTitle, MsgToken(1112));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B56  Team Code 091216
		case LK_TEAM_BRG:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1113_ = "Team Bearing", _@M1114_ = "TmBrng"
				_stprintf(BufferTitle, MsgToken(1114));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if(ValidWayPoint(TeamCodeRefWaypoint) && TeammateCodeValid) {
				value=DerivedDrawInfo.TeammateBearing;
				valid=true;
				if (value > 1)
					_stprintf(BufferValue, TEXT("%2.0f°"), value);
				else if (value < -1)
					_stprintf(BufferValue, TEXT("%2.0f°"), -value);
				else
					_tcscpy(BufferValue, TEXT("0°"));
			}
			break;

		// B57 Team Bearing Difference 091216
		case LK_TEAM_BRGDIFF:
			wsprintf(BufferValue,_T(NULLLONG));

			if (lktitle)
				// LKTOKEN  _@M1115_ = "Team Bearing Diff", _@M1116_ = "TeamBd"
				_stprintf(BufferTitle, MsgToken(1116));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if (ValidWayPoint(TeamCodeRefWaypoint) && TeammateCodeValid) {
				value = DerivedDrawInfo.TeammateBearing -  DrawInfo.TrackBearing;
				valid=true; // 091221
				if (value < -180.0)
					value += 360.0;
				else
					if (value > 180.0) value -= 360.0;

	              if (value > 30)
	                _stprintf(BufferValue, TEXT("%2.0f°»"), value);
	              else
	                if (value > 2)
	                  _stprintf(BufferValue, TEXT("%2.0f°›"), value);
	                else
	                  if (value < -30)
	                    _stprintf(BufferValue, TEXT("«%2.0f°"), -value);
	                  else
	                    if (value < -2)
	                      _stprintf(BufferValue, TEXT("‹%2.0f°"),- value);
	                    else
	                      _stprintf(BufferValue, TEXT("«»"));
			}
			break;

		// B58 091216 Team Range Distance
		case LK_TEAM_DIST:
			if ( TeammateCodeValid ) {
				value=DISTANCEMODIFY*DerivedDrawInfo.TeammateRange;
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
				// LKTOKEN  _@M1117_ = "Team Range", _@M1118_ = "TeamDis"
				_tcscpy(BufferTitle, MsgToken(1118));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;


		// B59
		case LK_SPEEDTASK_INST:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1119_ = "Task Speed Instantaneous", _@M1120_ = "TskSpI"
				_stprintf(BufferTitle, MsgToken(1120));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			value=0;
			if ( ActiveWayPoint >=1) {
				if ( ValidTaskPoint(ActiveWayPoint) ) {
					value = TASKSPEEDMODIFY*DerivedDrawInfo.TaskSpeedInstantaneous;
					if (value<=0||value>999) value=0; else valid=true;
					if (value<99)
						sprintf(text,"%.1f",value);
					else
						sprintf(text,"%d",(int)value);
					wsprintf(BufferValue, TEXT("%S"),text);
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetTaskSpeedName()));
			break;
		// B60
		case LK_HOME_DIST:
			if (HomeWaypoint>=0) {
				if ( ValidWayPoint(HomeWaypoint) != false ) {
					value=DerivedDrawInfo.HomeDistance*DISTANCEMODIFY;
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
				// LKTOKEN  _@M1121_ = "Home Distance", _@M1122_ = "HomeDis"
				wsprintf(BufferTitle, MsgToken(1122));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;


		// B61
		case LK_SPEEDTASK_ACH:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1123_ = "Task Speed", _@M1124_ = "TskSp"
				_stprintf(BufferTitle, MsgToken(1124));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			value=0;
			if ( ActiveWayPoint >=1) {
				if ( ValidTaskPoint(ActiveWayPoint) ) {
					value = TASKSPEEDMODIFY*DerivedDrawInfo.TaskSpeedAchieved;
					if (value<0||value>999) value=0; else valid=true;
					if (value<99)
						sprintf(text,"%.1f",value);
					else
						sprintf(text,"%d",(int)value);
					wsprintf(BufferValue, TEXT("%S"),text);
				}
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetTaskSpeedName()));
			break;


		// B62
		case LK_AA_DELTATIME:
			wsprintf(BufferValue,_T(NULLTIME));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			wsprintf(BufferUnit,_T("h"));
			// TODO This is in the wrong place, should be moved to calc thread! 090916
			double dd;
			if (AATEnabled && ValidTaskPoint(ActiveWayPoint)) {
				dd = DerivedDrawInfo.TaskTimeToGo;
				if ((DerivedDrawInfo.TaskStartTime>0.0) && (DerivedDrawInfo.Flying) &&(ActiveWayPoint>0)) {
					dd += DrawInfo.Time-DerivedDrawInfo.TaskStartTime;
				}
				dd= max(0.0,min(24.0*3600.0,dd))-AATTaskLength*60;
#if (0)
				if (dd<0) {
					status = 1; // red
				} else {
					if (DerivedDrawInfo.TaskTimeToGoTurningNow > (AATTaskLength+5)*60) {
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

		// B63 091221
		case LK_TC_ALL:
			if (DerivedDrawInfo.timeCircling <=0)
				//value=0.0;
				sprintf(text,NULLMEDIUM);
			else {
				value = LIFTMODIFY*DerivedDrawInfo.TotalHeightClimb /DerivedDrawInfo.timeCircling;
				if (value<20)
					sprintf(text,"%+.1lf",value);
				else
					sprintf(text,"%+.0lf",value);
				valid=true;
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
			if (lktitle)
				// LKTOKEN  _@M1127_ = "Thermal All", _@M1128_ = "Th.All"
				wsprintf(BufferTitle, MsgToken(1128));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;
		// B64
		case LK_LOGGER:
			_tcscpy(BufferTitle, MsgToken(1695));
			if (LoggerActive)
				wsprintf(BufferValue,MsgToken(1700)); // ON
			else {
				if (DisableAutoLogger)
					wsprintf(BufferValue,MsgToken(1701)); // no!
				else
					wsprintf(BufferValue,MsgToken(1696)); // auto
			}
			valid = true;
			break;

		// B65 FIXED 100125
		case LK_BATTERY:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

#if (WINDOWSPC<1)
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
#endif
			break;


		// B66
		case LK_FIN_GR:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1133_ = "Task Req.Efficiency", _@M1134_ = "TskReqE"
				_stprintf(BufferTitle, MsgToken(1134));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if ( (ValidTaskPoint(ActiveWayPoint) != false) ) {
				index = Task[ActiveWayPoint].Index;
				if (index>=0) {
					// the ValidFinish() seem to return FALSE when is actually valid.
					// In any case we do not use it for the vanilla GR
					value = DerivedDrawInfo.GRFinish;
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

		// B67
		case LK_ALTERN1_GR:
		// B68
		case LK_ALTERN2_GR:
		// B69
		case LK_BESTALTERN_GR:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			if (lktitle) {
				switch (lkindex) {
					case LK_BESTALTERN_GR:
						// LKTOKEN  _@M1139_ = "BestAltern Req.Efficiency", _@M1140_ = "BAtn.E"
						_stprintf(BufferTitle, MsgToken(1140));
						break;
					case LK_ALTERN1_GR:
						// LKTOKEN  _@M1135_ = "Alternate1 Req.Efficiency", _@M1136_ = "Atn1.E"
						_stprintf(BufferTitle, MsgToken(1136));
						break;
					case LK_ALTERN2_GR:
						// LKTOKEN  _@M1137_ = "Alternate2 Req.Efficiency", _@M1138_ = "Atn2.E"
						_stprintf(BufferTitle, MsgToken(1138));
						break;

					default:
						_stprintf(BufferTitle, TEXT("Atn%d.E"), lkindex-LK_ALTERNATESGR+1);
						break;
				}//sw
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


		// B70
		case LK_QFE:
			value=ALTITUDEMODIFY*DerivedDrawInfo.NavAltitude-QFEAltitudeOffset;;
			valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;


		// B71	LD AVR 
		case LK_LD_AVR:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1143_ = "Average Efficiency", _@M1144_ = "E.Avg"
				_stprintf(BufferTitle, MsgToken(1144));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
				if (DerivedDrawInfo.Flying)
					value=DerivedDrawInfo.AverageLD;
				else
					value=0;

				if (value==0) {
					sprintf(text,NULLMEDIUM);
				} else {
					if (value <1 ||  value >=ALTERNATE_MAXVALIDGR ) {
						strcpy(text,INFINVAL); 
						valid=true;
					} else {

						if (value<100)
							sprintf(text,"%.1f",value);
						else
							sprintf(text,"%2.0f",value);
						valid=true;
					}
				}
				wsprintf(BufferValue, TEXT("%S"),text);
			}
			break;


		// B72	WP REQ EFF
		case LK_NEXT_GR:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1145_ = "Next Req.Efficiency", _@M1146_ = "Req.E"
				_stprintf(BufferTitle, MsgToken(1146));
			else
				// LKTOKEN  _@M1145_ = "Next Req.Efficiency", _@M1146_ = "Req.E"
				_stprintf(BufferTitle, MsgToken(1146));
            
            LockTaskData();
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				if (ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute()) index=RESWP_OPTIMIZED;
				else index = Task[ActiveWayPoint].Index;
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
            UnlockTaskData();
			break;

		// B73
		case LK_FL:
			_stprintf(BufferTitle, MsgToken(1148));
			// Cant use NavAltitude, because FL should use Baro if available, despite
			// user settings.
			if (DrawInfo.BaroAltitudeAvailable)
				value=(TOFEET*(AltitudeToQNEAltitude(DrawInfo.BaroAltitude)))/100.0;
			else
				value=(TOFEET*(AltitudeToQNEAltitude(DrawInfo.Altitude)))/100.0;

			#if 0
	if (DrawInfo.BaroAltitudeAvailable) {
	StartupStore(_T(".... FL BARO: GPSAlt=%.0f BaroAlt=%.0f QNH=%.2f QNEAlt=%.0f FLAlt=%.0f FL=%d\n"),
		DrawInfo.Altitude,DrawInfo.BaroAltitude,QNH, 
		AltitudeToQNEAltitude(DrawInfo.BaroAltitude),
		TOFEET*AltitudeToQNEAltitude(DrawInfo.BaroAltitude),(int)value);
	} else {
	StartupStore(_T(".... FL GPS: GPSAlt=%.0f BaroAlt=%.0f QNH=%.2f QNEAlt=%.0f FLAlt=%.0f FL=%d\n"),
		DrawInfo.Altitude,DrawInfo.Altitude,QNH, 
		AltitudeToQNEAltitude(DrawInfo.Altitude),
		TOFEET*AltitudeToQNEAltitude(DrawInfo.Altitude),(int)value);
	}
			#endif

			if (value>=1) {
				valid=true;
				sprintf(text,"%d",(int)value);
				wsprintf(BufferValue, TEXT("%S"),text);
			} else {
				valid=true;
				wsprintf(BufferValue, TEXT("--"));
			}
			break;


		// B74
		case LK_TASK_DISTCOV:
			if ( (ActiveWayPoint >=1) && ( ValidTaskPoint(ActiveWayPoint) )) {
				value = DISTANCEMODIFY*DerivedDrawInfo.TaskDistanceCovered;
				valid=true;
				sprintf(text,"%.0f",value); // l o f?? TODO CHECK
			} else {
				strcpy(text,NULLLONG);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
				// LKTOKEN  _@M1149_ = "Task Covered distance", _@M1150_ = "TskCov"
				wsprintf(BufferTitle, MsgToken(1150));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;


		// B75
		case LK_ALTERN1_ARRIV:
		// B76
		case LK_ALTERN2_ARRIV:
		// B77
		case LK_BESTALTERN_ARRIV:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			if (lktitle) {
				switch (lkindex) {
					case LK_BESTALTERN_ARRIV:
						// LKTOKEN  _@M1155_ = "BestAlternate Arrival", _@M1156_ = "BAtnArr"
						_stprintf(BufferTitle, MsgToken(1156));
						break;
					case LK_ALTERN1_ARRIV:
						// LKTOKEN  _@M1151_ = "Alternate1 Arrival", _@M1152_ = "Atn1Arr"
						_stprintf(BufferTitle, MsgToken(1152));
						break;
					case LK_ALTERN2_ARRIV:
						// LKTOKEN  _@M1153_ = "Alternate2 Arrival", _@M1154_ = "Atn2Arr"
						_stprintf(BufferTitle, MsgToken(1154));
						break;
					default:
						_stprintf(BufferTitle, TEXT("Atn%dArr"), lkindex-LK_ALTERNATESARRIV+1);
						break;
				} //sw
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


		// B78
		case LK_HOMERADIAL:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1157_ = "Home Radial", _@M1158_ = "Radial"
				_stprintf(BufferTitle, MsgToken(1158));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if ( ValidWayPoint(HomeWaypoint) != false ) {
				if (DerivedDrawInfo.HomeDistance >10.0) {
					// homeradial == 0, ok?
					value = DerivedDrawInfo.HomeRadial;
					valid=true;
#ifndef __MINGW32__
					if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0f\xB0"), value);
					else if (value < -1)
						_stprintf(BufferValue, TEXT("%2.0f\xB0"), -value);
						else
							_tcscpy(BufferValue, TEXT("0\xB0"));
#else
					if (value > 1)
						_stprintf(BufferValue, TEXT("%2.0f°"), value);
					else if (value < -1)
						_stprintf(BufferValue, TEXT("%2.0f°"), -value);
						else
							_tcscpy(BufferValue, TEXT("0°"));
#endif
				}
			}
			break;


		// B79
		case LK_AIRSPACEHDIST:
			if (lktitle)
				// LKTOKEN  _@M1159_ = "Airspace Distance", _@M1160_ = "AirSpace"
				wsprintf(BufferTitle, MsgToken(1160));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if (NearestAirspaceHDist >0) {
				value = DISTANCEMODIFY*NearestAirspaceHDist;

				if (NearestAirspaceHDist<1000) {
					sprintf(text,"%1.3f",value);
				} else {
					sprintf(text,"%1.1f",value);
				}

				wsprintf(BufferValue, TEXT("%S"),text);
				wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
				valid = true;
			} else {
				valid=false;
				wsprintf(BufferValue, TEXT(NULLMEDIUM),text);
				wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			}
			break;


		// B80
		case LK_EXTBATTBANK:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
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
			wsprintf(BufferValue,_T(NULLMEDIUM));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
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
				value=DerivedDrawInfo.Odometer*DISTANCEMODIFY;
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
				// LKTOKEN  _@M1167_ = "Odometer", _@M1168_ = "Odo"
				wsprintf(BufferTitle, MsgToken(1168));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B84  100126
		case LK_AQNH:
			if (lktitle)
				// LKTOKEN  _@M1169_ = "Altern QNH", _@M1170_ = "aAlt"
				_stprintf(BufferTitle, MsgToken(1170));
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
				// LKTOKEN  _@M1171_ = "Altern AGL", _@M1172_ = "aHAGL"
				_stprintf(BufferTitle, MsgToken(1172));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if (!DerivedDrawInfo.TerrainValid) { //@ 101013
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


		// B86
		case LK_HGPS:
			if (lktitle)
				// LKTOKEN  _@M1173_ = "Altitude GPS", _@M1174_ = "HGPS"
				_stprintf(BufferTitle, MsgToken(1174));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			if (DrawInfo.NAVWarning || (DrawInfo.SatellitesUsed == 0)) {
				wsprintf(BufferValue, TEXT(NULLLONG));
				valid=false;
			} else {
				value=ALTITUDEMODIFY*DrawInfo.Altitude;
				valid=true;
				sprintf(text,"%d",(int)value);
				wsprintf(BufferValue, TEXT("%S"),text);
				wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			}
			break;


		// B87  100908
		case LK_EQMC:
			// LKTOKEN  _@M1175_ = "MacCready Equivalent", _@M1176_ = "eqMC"
			wsprintf(BufferTitle, MsgToken(1176));
			if ( DerivedDrawInfo.Circling || DerivedDrawInfo.EqMc<0 || DerivedDrawInfo.OnGround) {
				wsprintf(BufferValue, TEXT(NULLMEDIUM));
			} else {
				value = iround(LIFTMODIFY*DerivedDrawInfo.EqMc*10)/10.0;
				valid=true;
				sprintf(text,"%2.1lf",value);
				wsprintf(BufferValue, TEXT("%S"),text);
			}
			break;


		// B88 B89
		case LK_EXP1:
			_stprintf(BufferTitle, TEXT("RAM"));
			_stprintf(BufferValue, TEXT("%d"),(int)(CheckFreeRam()/1024));
                        valid=true;
                        break;

		case LK_EXP2:
			_stprintf(BufferTitle, TEXT("hTE"));
			_stprintf(BufferValue, TEXT("%3.0f"),DerivedDrawInfo.EnergyHeight);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
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
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));

			if (OlcResults[ivalue].Type()==CContestMgr::TYPE_INVALID)
				wsprintf(BufferValue,_T(NULLMEDIUM));
			else {
				_stprintf(BufferValue, TEXT("%5.0f"),DISTANCEMODIFY*OlcResults[ivalue].Distance());
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
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));

			if (OlcResults[ivalue].Type()==CContestMgr::TYPE_INVALID)
				wsprintf(BufferValue,_T(NULLMEDIUM));
			else {
				if ( OlcResults[ivalue].Speed() >999 ) {
					wsprintf(BufferValue,_T(NULLMEDIUM));
				} else {
					_stprintf(BufferValue, TEXT("%3.1f"),SPEEDMODIFY*OlcResults[ivalue].Speed());
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
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			wsprintf(BufferUnit, TEXT("pt"));

			if (OlcResults[ivalue].Type()==CContestMgr::TYPE_INVALID)
				wsprintf(BufferValue,_T(NULLMEDIUM));
			else {
				_stprintf(BufferValue, TEXT("%3.0f"),OlcResults[ivalue].Score());
				valid=true;
			}
			break;

		// B113
		case LK_FLAPS:			
			_stprintf(BufferTitle, MsgToken(1641));
			if (GlidePolar::FlapsPosCount>0) {
				_stprintf(BufferValue,TEXT("%s"), DerivedDrawInfo.Flaps);
				BufferValue[7]='\0'; // set a limiter to the name: max 7 chars
				valid=true;
			} else {
				wsprintf(BufferValue, TEXT(NULLMEDIUM));
			}
			break;

		// B114
		case LK_AIRSPACEVDIST:
			if (lktitle)
				wsprintf(BufferTitle, MsgToken(1286)); // ArSpcV
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			if (NearestAirspaceVDist != 0 && (fabs(NearestAirspaceVDist)<=9999) ) { // 9999 m or ft is ok
				value = ALTITUDEMODIFY*NearestAirspaceVDist;
				sprintf(text,"%.0f",value);
				wsprintf(BufferValue, TEXT("%S"),text);
				valid = true;
			} else {
				valid=false;
				wsprintf(BufferValue, TEXT(NULLMEDIUM),text);
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;


		// B115
		case LK_HOME_ARRIVAL:
			if (lktitle)
				// LKTOKEN  _@M1644_ = "Home Alt.Arrival", _@M1645_ = "HomeArr"
				_stprintf(BufferTitle, MsgToken(1645));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
      
			if ( ValidWayPoint(HomeWaypoint) != false ) {
        value=WayPointCalc[HomeWaypoint].AltArriv[AltArrivMode]*ALTITUDEMODIFY;
        if ( value > ALTDIFFLIMIT ) {
          valid=true;
          _stprintf(BufferValue,TEXT("%+1.0f"), value);
        }
			}
      if (!valid)
				wsprintf(BufferValue,_T(NULLLONG));
        
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;


		// B116
		case LK_ALTERN1_BRG:
		// B117
		case LK_ALTERN2_BRG:
		// B118
		case LK_BESTALTERN_BRG:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
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
				value=WayPointCalc[index].Bearing;
				valid=true;
			}

			if (valid) {
				if (value > 1)
					_stprintf(BufferValue, TEXT("%2.0f°"), value);
				else if (value < -1)
					_stprintf(BufferValue, TEXT("%2.0f°"), -value);
					else
						_tcscpy(BufferValue, TEXT("0°"));
			} 

			wsprintf(BufferUnit, TEXT(""));
			break;

		// B119
		case LK_ALTERN1_DIST:
		// B120
		case LK_ALTERN2_DIST:
		// B121
		case LK_BESTALTERN_DIST:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
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
				value=DISTANCEMODIFY*WayPointCalc[index].Distance;
				valid=true;
			}


			if (valid) {
				if (value>99 || value==0)
					sprintf(text,"%.0f",value);
				else {
					if (ISPARAGLIDER) {
						if (value>10)
							sprintf(text,"%.1f",value);
						else
							sprintf(text,"%.2f",value);
					} else {
							sprintf(text,"%.1f",value);
					}
				}
				wsprintf(BufferValue, TEXT("%S"),text);
			}
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			break;



		// B122
		case LK_MAXALT:
			value=ALTITUDEMODIFY*DerivedDrawInfo.MaxAltitude;
			valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;

		// B123
		case LK_MAXHGAINED:
			value=ALTITUDEMODIFY*DerivedDrawInfo.MaxHeightGain;
			valid=true;
			sprintf(text,"%d",(int)value);
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			break;


		// B124
		case LK_HEADWINDSPEED:
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
			wsprintf(BufferUnit, TEXT("%s"),Units::GetHorizontalSpeedName());

			if (DerivedDrawInfo.HeadWind==-999) {
				_stprintf(BufferValue,TEXT(NULLMEDIUM)); 
				break;
			}
			
			value=DerivedDrawInfo.HeadWind*SPEEDMODIFY;
			if (value>=1 ) {
				_stprintf(BufferValue,TEXT("%+1.0f"), value );
				valid=true;
			} else 
			  if (value<=-1 ) {
				_stprintf(BufferValue,TEXT("%-1.0f"), value );
				valid=true;
			  } else {
				_stprintf(BufferValue,TEXT(NULLMEDIUM)); 
			  }
			break;

		// B125
		case LK_OLC_FAI_CLOSE:
			bFAI = CContestMgr::Instance().FAI();
			fTogo =DISTANCEMODIFY*CContestMgr::Instance().GetClosingPointDist();
			if(fTogo >0)
			{
				if (value>99)
					sprintf(text,"%.0f",fTogo);
				else
					sprintf(text,"%.1f",fTogo);
				valid = true;
			} else {
				strcpy(text,NULLLONG);
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			if (lktitle)
			{
				if(bFAI)
		    			wsprintf(BufferTitle, TEXT("FAI %s"),  gettext(TEXT("_@M1508_"))); //   _@M1508_ = "C:"
		      		else
		        		wsprintf(BufferTitle, TEXT("%s"),  gettext(TEXT("_@M1508_")));
		    	}
		        else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

	    		break;

		// B126
		case LK_OLC_FAI_CLOSE_PERCENT:
			bFAI = CContestMgr::Instance().FAI();
			fDist =CContestMgr::Instance().Result(CContestMgr::TYPE_FAI_TRIANGLE, false).Distance();
			fTogo =CContestMgr::Instance().GetClosingPointDist();
        		if((fDist >0) && (fTogo >0))
            		{
              			LKASSERT(fDist >0)
  				valid = true;
             			 value = fTogo / fDist*100.0f;
				sprintf(text,"%.1f",value);
		    	} else {
		    		strcpy(text,NULLLONG);
		    	}
		    	wsprintf(BufferValue, TEXT("%S"),text);
		    	wsprintf(BufferUnit, TEXT("%%"));
		    	if (lktitle)
		    	{
		    		if(bFAI)
		        		wsprintf(BufferTitle, TEXT("FAI %s"),  gettext(TEXT("_@M1508_"))); // LKTOKEN  _@M1508_ = "C:"
		      		else
		        		wsprintf(BufferTitle, TEXT("%s"),  gettext(TEXT("_@M1508_")));
		    	}
		    	else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );

			break;


                // B127
		case LK_BANK_ANGLE:
                        valid=true;
			if(DrawInfo.GyroscopeAvailable) { 
				value=DrawInfo.Roll;
                                // LKTOKEN _@M1197_ "Bank"
                                _tcscpy(BufferTitle, gettext(TEXT("_@M1197_")));
			} else {
				value=DerivedDrawInfo.BankAngle;
				_stprintf(BufferTitle, TEXT("e%s"), gettext(TEXT("_@M1197_")));
			}
                        _stprintf(BufferValue,TEXT("%.0f°"), value);
			wsprintf(BufferUnit, TEXT(""));
			break;

		// B128
		case LK_ALTERN1_RAD:
		// B129
		case LK_ALTERN2_RAD:
			wsprintf(BufferValue,_T(NULLMEDIUM));
			_stprintf(BufferTitle, TEXT("%s"), Data_Options[lkindex].Title );
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
				wsprintf(BufferTitle,_T("<"));
				if ( DisplayTextType == DISPLAYFIRSTTHREE)
				{
					 LK_tcsncpy(BufferTitle+1,WayPointList[index].Name,3);
				}
				else if( DisplayTextType == DISPLAYNUMBER) {
					_stprintf(BufferTitle+1,TEXT("%d"), WayPointList[index].Number );
				} else {
					LK_tcsncpy(BufferTitle+1,WayPointList[index].Name, 12);
					// BufferTitle[(sizeof(Text)/sizeof(TCHAR))-1] = '\0';
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
					_stprintf(BufferValue, TEXT("%2.0f°"), value);
				else if (value < -1)
					_stprintf(BufferValue, TEXT("%2.0f°"), -value);
					else
						_tcscpy(BufferValue, TEXT("0°"));
			} 

			wsprintf(BufferUnit, TEXT(""));
			break;

		// B130
		case LK_HEADING:
			wsprintf(BufferValue,_T(NULLLONG));
			//_stprintf(BufferUnit,TEXT(""));
			if (DrawInfo.MagneticHeadingAvailable) {
			    _stprintf(BufferTitle, _T("HDG"));
			    value = DrawInfo.MagneticHeading;
			} else {
			    _stprintf(BufferTitle, _T("eHDG"));
			    value = DerivedDrawInfo.Heading;
			}
			valid=true;
#ifndef __MINGW32__
			if (value > 1)
				_stprintf(BufferValue, TEXT("%2.0f\xB0"), value);
			else if (value < -1)
				_stprintf(BufferValue, TEXT("%2.0f\xB0"), -value);
				else
					_tcscpy(BufferValue, TEXT("0\xB0"));
#else
			if (value > 1)
				_stprintf(BufferValue, TEXT("%2.0f°"), value);
			else if (value < -1)
				_stprintf(BufferValue, TEXT("%2.0f°"), -value);
				else
					_tcscpy(BufferValue, TEXT("0°"));
#endif
			break;



		// B131
		case LK_WIND:
			// LKTOKEN  _@M1185_ = "Wind"
			_stprintf(BufferTitle, MsgToken(1185));
			if (DerivedDrawInfo.WindSpeed*SPEEDMODIFY>=1) {
				value = DerivedDrawInfo.WindBearing;
				valid=true;
				if (UseWindRose) {
					_stprintf(BufferValue,TEXT("%s/%1.0f"), 
						WindAngleToText(value), SPEEDMODIFY*DerivedDrawInfo.WindSpeed );
				} else {
					if (value==360) value=0;
					if (HideUnits)
						_stprintf(BufferValue,TEXT("%03.0f/%1.0f"), 
							value, SPEEDMODIFY*DerivedDrawInfo.WindSpeed );
					else
						_stprintf(BufferValue,TEXT("%03.0f")_T(DEG)_T("/%1.0f"), 
							value, SPEEDMODIFY*DerivedDrawInfo.WindSpeed );
				}
			} else {
				_stprintf(BufferValue,TEXT("--/--"));
			}
			break;



		// B132 Final arrival with MC 0 , no totaly energy.
		case LK_FIN_ALTDIFF0:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1191_ = "TskArr0"
				_stprintf(BufferTitle, MsgToken(1191));
			else
				// LKTOKEN  _@M1191_ = "TskArr0"
				_stprintf(BufferTitle, MsgToken(1191));
			if ( (ValidTaskPoint(ActiveWayPoint) != false) && DerivedDrawInfo.ValidStart ) {
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

		// B133  091222 using old ETE corrected now
		case LK_LKFIN_ETE:
lkfin_ete:
			wsprintf(BufferValue,_T(NULLTIME)); // 091222
			if (lktitle)
				// LKTOKEN  _@M1083_ = "Task Time To Go", _@M1084_ = "TskETE"
				_stprintf(BufferTitle, MsgToken(1084));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[LK_FIN_ETE].Title );
				// ^^ Notice we use LK_FIN_ETE, NOT LK_LKFIN_ETE which does NOT exist in DataOptions!

			if (ISCAR || ISGAAIRCRAFT) {
                            LockTaskData();
			    if ( ValidTaskPoint(ActiveWayPoint)) {
				if (DerivedDrawInfo.LKTaskETE > 0) { 
					valid=true;
					if ( Units::TimeToTextDown(BufferValue, (int)DerivedDrawInfo.LKTaskETE))
						wsprintf(BufferUnit, TEXT("h"));
					else
						wsprintf(BufferUnit, TEXT(""));
				} else {
					index = Task[ActiveWayPoint].Index;
					if ( WayPointCalc[index].NextETE > 0) { // single waypoint? uhm
						valid=true;
						if (Units::TimeToTextDown(BufferValue, (int)WayPointCalc[index].NextETE))
                                                	wsprintf(BufferUnit, TEXT("h"));
                                        	else
                                        	        wsprintf(BufferUnit, TEXT(""));
					} else
						wsprintf(BufferValue, TEXT(NULLTIME));
				}
                            }
                            UnlockTaskData();
			    break;
			}

            LockTaskData();
			if ( (ValidTaskPoint(ActiveWayPoint) != false) && DerivedDrawInfo.ValidStart ) {
				if (DerivedDrawInfo.TaskTimeToGo > 0) { 
					valid=true;
					if ( Units::TimeToTextDown(BufferValue, (int)DerivedDrawInfo.TaskTimeToGo))
						wsprintf(BufferUnit, TEXT("h"));
					else
						wsprintf(BufferUnit, TEXT(""));
				} else {
					index = Task[ActiveWayPoint].Index;
					if ( (WayPointCalc[index].NextETE > 0) && !ValidTaskPoint(1) ) {
						valid=true;
						if (Units::TimeToTextDown(BufferValue, (int)WayPointCalc[index].NextETE))
                                                	wsprintf(BufferUnit, TEXT("h"));
                                        	else
                                        	        wsprintf(BufferUnit, TEXT(""));
					} else
						wsprintf(BufferValue, TEXT(NULLTIME));
				}
			}
            UnlockTaskData();
			break;

		// B134
		// Using MC=0!  total energy disabled
		case LK_NEXT_ALTDIFF0:
			wsprintf(BufferValue,_T(NULLLONG));
			// LKTOKEN  _@M1190_ = "ArrMc0"
			_stprintf(BufferTitle, MsgToken(1190));

			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				if (ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute()) index=RESWP_OPTIMIZED;
				else index = Task[ActiveWayPoint].Index;
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

		// B135
		case LK_TIME_LOCALSEC:
			Units::TimeToTextS(BufferValue, (int)DetectCurrentTime());
			valid=true;
			// LKTOKEN  _@M1079_ = "Time local", _@M1080_ = "Time"
			_stprintf(BufferTitle, MsgToken(1080));
			break;

		// B136
		case LK_TARGET_DIST:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
			} else {
	                        if (DrawInfo.FLARM_Traffic[LKTargetIndex].ID <=0) {
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
			// LKTOKEN  _@M1023_ = "Next Distance", _@M1024_ = "Dist"
			wsprintf(BufferTitle, MsgToken(1024),text);
			break;

		// B137
		case LK_TARGET_TO:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
			} else {
	                        if (DrawInfo.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
				} else {
						value = LKTraffic[LKTargetIndex].Bearing -  DrawInfo.TrackBearing;
						valid=true;
						if (value < -180.0)
							value += 360.0;
						else
							if (value > 180.0)
								value -= 360.0;
			              if (value > 30)
			                _stprintf(BufferValue, TEXT("%2.0f°»"), value);
			              else
			                if (value > 2)
			                  _stprintf(BufferValue, TEXT("%2.0f°›"), value);
			                else
			                  if (value < -30)
			                    _stprintf(BufferValue, TEXT("«%2.0f°"), -value);
			                  else
			                    if (value < -2)
			                      _stprintf(BufferValue, TEXT("‹%2.0f°"),- value);
			                    else
			                      _stprintf(BufferValue, TEXT("«»"));
				}
			}
			wsprintf(BufferUnit, TEXT(""));
			// LKTOKEN  _@M1095_ = "Bearing Difference", _@M1096_ = "To"
			_stprintf(BufferTitle, MsgToken(1096));
			break;

		// B138
		case LK_TARGET_BEARING:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
			} else {
	                        if (DrawInfo.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
				} else {
						value = LKTraffic[LKTargetIndex].Bearing;
						valid=true;
						if (value == 360)
							_stprintf(BufferValue, TEXT("0°"));
						else 
							_stprintf(BufferValue, TEXT("%2.0f°"), value);
				}
			}
			wsprintf(BufferUnit, TEXT(""));
			// LKTOKEN  _@M1007_ = "Bearing", _@M1008_ = "Brg"
			wsprintf(BufferTitle, MsgToken(1008));
			break;

		// B139
		case LK_TARGET_SPEED:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
			} else {
	                        if (DrawInfo.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
				} else {
					value=SPEEDMODIFY*DrawInfo.FLARM_Traffic[LKTargetIndex].Speed;
					if (value<0||value>9999) value=0; else valid=true;
					sprintf(text,"%d",(int)value);
				}
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			// LKTOKEN  _@M1013_ = "Speed ground", _@M1014_ = "GS"
			wsprintf(BufferTitle, MsgToken(1014));
			break;

		// B140
		case LK_TARGET_ALT:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
			} else {
	                        if (DrawInfo.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
				} else {
					value=ALTITUDEMODIFY*DrawInfo.FLARM_Traffic[LKTargetIndex].Altitude;
					valid=true;
					sprintf(text,"%d",(int)value);
				}
			}
			// LKTOKEN  _@M1001_ = "Altitude QNH", _@M1002_ = "Alt", 
			_stprintf(BufferTitle, MsgToken(1002));
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
	                        if (DrawInfo.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
				} else {
					value=ALTITUDEMODIFY*(DerivedDrawInfo.NavAltitude-DrawInfo.FLARM_Traffic[LKTargetIndex].Altitude)*-1;
					valid=true;
					sprintf(text,"%+d",(int)value);
				}
			}
			// LKTOKEN  _@M1193_ = "RelAlt"
			_stprintf(BufferTitle, MsgToken(1193));
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
	                        if (DrawInfo.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
					_tcscpy(BufferUnit, _T(""));
				} else {
					value = LIFTMODIFY*DrawInfo.FLARM_Traffic[LKTargetIndex].ClimbRate;
					valid=true;
					_stprintf(BufferValue,varformat,value);
					wsprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
				}
			}
			// LKTOKEN  _@M1194_ = "Var"
			_tcscpy(BufferTitle, MsgToken(1194));
			break;

		// B143
		case LK_TARGET_AVGVARIO:
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
					_tcscpy(BufferUnit, _T(""));
			} else {
	                        if (DrawInfo.FLARM_Traffic[LKTargetIndex].ID <=0) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
					_tcscpy(BufferUnit, _T(""));
				} else {
					value = LIFTMODIFY*DrawInfo.FLARM_Traffic[LKTargetIndex].Average30s;
					valid=true;
					_stprintf(BufferValue,varformat,value);
					wsprintf(BufferUnit, TEXT("%s"),Units::GetVerticalSpeedName());
				}
			}
			// LKTOKEN  _@M1189_ = "Var30"
			_tcscpy(BufferTitle, MsgToken(1189));
			break;

		// B144
		case LK_TARGET_ALTARRIV:
			wsprintf(BufferValue,_T(NULLLONG));
			// LKTOKEN  _@M1188_ = "Arr"
			_stprintf(BufferTitle, MsgToken(1188));
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
					wsprintf(BufferValue, TEXT("%S"),text);
			} else {
	                        if (DrawInfo.FLARM_Traffic[LKTargetIndex].ID <=0) {
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
			// LKTOKEN  _@M1187_ = "ReqE"
			_stprintf(BufferTitle, MsgToken(1187));
			_tcscpy(BufferUnit,_T(""));
			if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
					strcpy(text,NULLMEDIUM);
			} else {
	                        if (DrawInfo.FLARM_Traffic[LKTargetIndex].ID <=0) {
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
			// LKTOKEN  _@M1186_ = "eIAS"
			_stprintf(BufferTitle, MsgToken(1186));
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


		// B147 Distance from the start sector, always available also after start
		case LK_START_DIST:
			if ( ValidTaskPoint(0) && ValidTaskPoint(1) ) { // if real task
				if((ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute())&& ActiveWayPoint == 0) {
					value=WayPointCalc[RESWP_OPTIMIZED].Distance*DISTANCEMODIFY;
					if (value>99 || value==0)
						sprintf(text,"%.0f",value);
					else {
						if (value>10) {
							sprintf(text,"%.1f",value);
						} else 
							sprintf(text,"%.3f",value);
					}
				}
				else {
					index = Task[0].Index;
					if (index>=0) {
						value=(DerivedDrawInfo.WaypointDistance-StartRadius)*DISTANCEMODIFY;
						if (value<0) value*=-1; // 101112 BUGFIX
						valid=true;
						if (value>99 || value==0)
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
				}
			} else {
				strcpy(text,NULLMEDIUM); // 091221
			}
			wsprintf(BufferValue, TEXT("%S"),text);
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			// LKTOKEN  _@M1192_ = "StDis"
			_tcscpy(BufferTitle, MsgToken(1192));
			break;

		// B156
		case LK_NEXT_CENTER_ALTDIFF:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1025_ = "Next Alt.Arrival", _@M1026_ = "NxtArr"
				_stprintf(BufferTitle, MsgToken(1026));
			else
				_stprintf(BufferTitle, TEXT("%s"), Data_Options[LK_NEXT_ALTDIFF].Title );
            
            LockTaskData();
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
            UnlockTaskData();
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
			break;

		// B157
		case LK_NEXT_CENTER_GR:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				// LKTOKEN  _@M1145_ = "Next Req.Efficiency", _@M1146_ = "Req.E"
				_stprintf(BufferTitle, MsgToken(1146));
			else
				// LKTOKEN  _@M1145_ = "Next Req.Efficiency", _@M1146_ = "Req.E"
				_stprintf(BufferTitle, MsgToken(1146));
            LockTaskData();
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
            UnlockTaskData();
			break;

		// B253
		case LK_DUMMY:
			wsprintf(BufferValue,_T(NULLLONG));
			if (lktitle)
				_stprintf(BufferTitle, TEXT("Dummy"));
			else
				_stprintf(BufferTitle, TEXT("Dummy"));

			wsprintf(BufferUnit, TEXT("."));
			break;

		// B254
		case LK_EMPTY:
lk_empty:
			wsprintf(BufferValue, TEXT(""));
			// wsprintf(BufferUnit, TEXT(""));
			wsprintf(BufferTitle, TEXT(""));
			break;

		// B255
		case LK_ERROR:
lk_error:
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
			if ( lkindex >=NumDataOptions || lkindex <1 )  // Notice NumDataOptions check!
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
	if (value<0.001) value=0;
	if (value>99 || value==0)
		sprintf(text,"%.0f",value);
	else {
		if (ISPARAGLIDER) {
			if (value>10)
				sprintf(text,"%.1f",value);
			else 
				sprintf(text,"%.3f",value);
		} else {
			sprintf(text,"%.1f",value);
		}
	}
  } else {
	strcpy(text,NULLMEDIUM);
  }
  wsprintf(BufferValue, TEXT("%S"),text);
  wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
  return;
}

// DO NOT use this for AAT values! 
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
	// THIS is forcing bearing while circling
	// if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
	if (true) {
		// Warning, for AAT this should be WaypointBearing, so do not use it!
		value = WayPointCalc[index].Bearing -  DrawInfo.TrackBearing;
		if (value < -180.0)
			value += 360.0;
		else
			if (value > 180.0)
				value -= 360.0;
        if (value > 30)
          _stprintf(BufferValue, TEXT("%2.0f°»"), value);
        else
          if (value > 2)
            _stprintf(BufferValue, TEXT("%2.0f°›"), value);
          else
            if (value < -30)
              _stprintf(BufferValue, TEXT("«%2.0f°"), -value);
            else
              if (value < -2)
                _stprintf(BufferValue, TEXT("‹%2.0f°"),- value);
              else
                _stprintf(BufferValue, TEXT("«»"));
	} else {
		// while circling, print simple bearing
		value = WayPointCalc[index].Bearing;
		if (value > 1)
			_stprintf(BufferValue, TEXT("%2.0f°"), value);
		else if (value < -1)
				_stprintf(BufferValue, TEXT("%2.0f°"), -value);
			else
				_tcscpy(BufferValue, TEXT("0°"));
	}
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

