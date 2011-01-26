/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKDrawInfoPage.cpp,v 1.1 2010/12/11 14:25:09 root Exp root $
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
#include "RasterTerrain.h" // 091109
#include "LKUtils.h"
#include "LKMapWindow.h"


#if (WINDOWSPC>0)
#include <wingdi.h>
#endif

extern void DrawGlideCircle(HDC hdc, POINT Orig, RECT rc );
extern void MapWaypointLabelAdd(TCHAR *Name, int X, int Y, TextInBoxMode_t Mode, int AltArivalAGL, bool inTask, 
	bool isLandable, bool isAirport, bool isExcluded, int index);
extern int _cdecl MapWaypointLabelListCompare(const void *elem1, const void *elem2 );

extern void DrawMapSpace(HDC hdc, RECT rc);
extern void DrawNearest(HDC hdc, RECT rc);
extern void DrawNearestTurnpoint(HDC hdc, RECT rc);
extern void DrawCommon(HDC hdc, RECT rc);
extern void DrawWelcome8000(HDC hdc, RECT rc);
#ifdef CPUSTATS
extern void DrawCpuStats(HDC hdc, RECT rc);
#endif
#ifdef DRAWDEBUG
extern void DrawDebug(HDC hdc, RECT rc);
#endif

extern void WriteInfo(HDC hdc, bool *showunit, TCHAR *BufferValue, TCHAR *BufferUnit, TCHAR *BufferTitle, 
				short *columnvalue, short *columntitle, short *row1, short *row2, short *row3);

extern int PDABatteryPercent;
extern int PDABatteryTemperature;
extern int MapWaypointLabelListCount;
extern void ConvToUpper(TCHAR *str);

typedef struct{
  TCHAR Name[NAME_SIZE+1];
  POINT Pos;
  TextInBoxMode_t Mode;
  int AltArivalAGL;
  bool inTask;
  bool isLandable;
  bool isAirport;
  bool isExcluded;
  int  index;
}MapWaypointLabel_t;


extern MapWaypointLabel_t MapWaypointLabelList[];


void VDrawLine(const HDC&hdc, const RECT rc, int x1, int y1, int x2, int y2, COLORREF col) {
  POINT p0,p1;
  p0.x = x1;
  p0.y = y1;
  p1.x = x2;
  p1.y = y2;
MapWindow::_DrawLine(hdc, PS_SOLID, NIBLSCALE(1), p0, p1, col, rc);
}


void MapWindow::DrawInfoPage(HDC hdc,  RECT rc, bool forceinit )
{
  HFONT		oldfont=0;
  //SIZE TextSize;
  TCHAR Buffer[LKSIZEBUFFERLARGE];
  TCHAR BufferTitle[LKSIZEBUFFERTITLE];
  TCHAR BufferValue[LKSIZEBUFFERVALUE];
  TCHAR BufferUnit[LKSIZEBUFFERUNIT];
  TCHAR Empty[2]; // 100407
  int index=-1;

  static short tlen=0;
  static bool doinit=true;
  static short	column[PANELCOLUMNS+1], hcolumn[(PANELCOLUMNS*2)+1], qcolumn[(PANELCOLUMNS*4)+1];
  static short	row[PANELROWS+1], hrow[(PANELCOLUMNS*2)+1], qrow[(PANELROWS*4)+1];

  bool showunit=false;
  _tcscpy(Empty,_T(""));

  if (forceinit) doinit=true;

	oldfont = (HFONT)SelectObject(hdc, LKINFOFONT); // save font

  if (doinit) {
	doinit=false;
	// function can only be called in fullscreen  and thus can be inited here
	if ( ScreenSize < (ScreenSize_t)sslandscape ) {
		switch (ScreenSize) {			// portrait fullscreen
			case (ScreenSize_t)ss240x320:
				tlen=6;
				break;
			default:
				tlen=7;
				break;
		}
	} else  {
		switch (ScreenSize) {			// landscape fullscreen
			case (ScreenSize_t)ss800x480:
			case (ScreenSize_t)ss400x240:
			case (ScreenSize_t)ss480x272:
			case (ScreenSize_t)ss720x408:
				tlen=9;
				break;
			case (ScreenSize_t)ss320x240:
			case (ScreenSize_t)ss640x480:
			case (ScreenSize_t)ss896x672:
				tlen=9;
				break;
			default:
				tlen=8;
				break;
		}
	}


	column[0]=LEFTLIMITER;
	column[1]=((rc.right-RIGHTLIMITER-LEFTLIMITER)/PANELCOLUMNS)+LEFTLIMITER;
	column[2]=column[1]*2-LEFTLIMITER;
	column[3]=column[1]*3-LEFTLIMITER*2;
	column[PANELCOLUMNS]=rc.right-RIGHTLIMITER;
	row[0]=rc.top+TOPLIMITER;
	row[1]=((rc.bottom-BottomSize-row[0]-BOTTOMLIMITER)/PANELROWS)+row[0];
	row[2]=((rc.bottom-BottomSize-row[0]-BOTTOMLIMITER)/PANELROWS)*2+row[0];
	row[3]=((rc.bottom-BottomSize-row[0]-BOTTOMLIMITER)/PANELROWS)*3+row[0];
	row[PANELROWS]=rc.bottom-BottomSize-BOTTOMLIMITER;

	hcolumn[0]=column[0];
	hcolumn[1]=(column[1]-column[0])/2;
	hcolumn[2]=column[1];
	hcolumn[3]=(column[2]-column[1])/2+column[1];
	hcolumn[4]=column[2];
	hcolumn[5]=(column[3]-column[2])/2+column[2];
	hcolumn[6]=column[3];
	hcolumn[7]=(column[4]-column[3])/2+column[3];
	hcolumn[8]=column[4];

	hrow[0]=row[0];
	hrow[1]=(row[1]-row[0])/2;
	hrow[2]=row[1];
	hrow[3]=(row[2]-row[1])/2+row[1];
	hrow[4]=row[2];
	hrow[5]=(row[3]-row[2])/2+row[2];
	hrow[6]=row[3];
	hrow[7]=(row[4]-row[3])/2+row[3];
	hrow[8]=row[4];

	qcolumn[0]=hcolumn[0];
	qcolumn[1]=(hcolumn[1]-hcolumn[0])/2+hcolumn[0];
	qcolumn[2]=hcolumn[1];
	qcolumn[3]=(hcolumn[2]-hcolumn[1])/2+hcolumn[1];
	qcolumn[4]=hcolumn[2];
	qcolumn[5]=(hcolumn[3]-hcolumn[2])/2+hcolumn[2];
	qcolumn[6]=hcolumn[3];
	qcolumn[7]=(hcolumn[4]-hcolumn[3])/2+hcolumn[3];
	qcolumn[8]=hcolumn[4];
	qcolumn[9]=(hcolumn[5]-hcolumn[4])/2+hcolumn[4];
	qcolumn[10]=hcolumn[5];
	qcolumn[11]=(hcolumn[6]-hcolumn[5])/2+hcolumn[5];
	qcolumn[12]=hcolumn[6];
	qcolumn[13]=(hcolumn[7]-hcolumn[6])/2+hcolumn[6];
	qcolumn[14]=hcolumn[7];
	qcolumn[15]=(hcolumn[8]-hcolumn[7])/2+hcolumn[7];
	qcolumn[16]=hcolumn[8];

	qrow[0]=hrow[0];
	qrow[1]=(hrow[1]-hrow[0])/2+hrow[0];
	qrow[2]=hrow[1];
	qrow[3]=(hrow[2]-hrow[1])/2+hrow[1];
	qrow[4]=hrow[2];
	qrow[5]=(hrow[3]-hrow[2])/2+hrow[2];
	qrow[6]=hrow[3];
	qrow[7]=(hrow[4]-hrow[3])/2+hrow[3];
	qrow[8]=hrow[4];
	qrow[9]=(hrow[5]-hrow[4])/2+hrow[4];
	qrow[10]=hrow[5];
	qrow[11]=(hrow[6]-hrow[5])/2+hrow[5];
	qrow[12]=hrow[6];
	qrow[13]=(hrow[7]-hrow[6])/2+hrow[6];
	qrow[14]=hrow[7];
	qrow[15]=(hrow[8]-hrow[7])/2+hrow[7];
	qrow[16]=hrow[8];

	qrow[5]+=NIBLSCALE(6);
	qrow[6]+=NIBLSCALE(6);
	qrow[7]+=NIBLSCALE(6);
	qrow[8]+=NIBLSCALE(6)*2;
	qrow[9]+=NIBLSCALE(6)*2;
	qrow[10]+=NIBLSCALE(6)*2;
	qrow[11]+=NIBLSCALE(6)*3;
	qrow[12]+=NIBLSCALE(6)*3;
	qrow[13]+=NIBLSCALE(6)*3;

  } // doinit

#include "./LKMW3include2.cpp"

	SelectObject(hdc, LK8PanelBigFont);
#include "./LKMW3include1.cpp"

	int curtype; // 100404
	bool ontarget=false;
	FLARM_TRAFFIC *pTarget=NULL;
	if ( (MapSpaceMode != MSM_INFO_TRF) && (MapSpaceMode != MSM_INFO_TARGET) ) {
		curtype=CURTYPE;
	} else {
		if (MapSpaceMode == MSM_INFO_TRF)
			curtype=IM_TRF + IM_TOP;
		else
			curtype=IM_TARGET + IM_TOP;

		if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
			ontarget=false;
		} else {
			if (GPS_INFO.FLARM_Traffic[LKTargetIndex].ID <=0) {
				ontarget=false;
			} else {
				ontarget=true;
				pTarget=&GPS_INFO.FLARM_Traffic[LKTargetIndex];
			}
		}
	}

	if (ontarget) DoTarget(&GPS_INFO, &CALCULATED_INFO);

	switch (LKevent) {
		case LKEVENT_NONE:
			break;
		case LKEVENT_ENTER:
			break;
		case LKEVENT_NEWRUN:
			break;
		case LKEVENT_PAGEUP:
			break;
		case LKEVENT_PAGEDOWN:
			break;
		default:
			break;
	}
	LKevent=LKEVENT_NONE; 

	COLORREF icolor; // 091229

	// R0 C0	= status
	SelectObject(hdc, LK8PanelMediumFont);
	switch(curtype) {
		case IM_THERMAL:
			wsprintf(Buffer,_T("%d.%d %s"),ModeIndex, curtype+1, gettext(TEXT("_@M905_"))); // Thermal
			break;
		case IM_CRUISE:
			wsprintf(Buffer,_T("%d.%d %s"),ModeIndex, curtype+1, gettext(TEXT("_@M906_"))); // Cruise
			break;
		case IM_TASK:
			wsprintf(Buffer,_T("%d.%d %s"),ModeIndex, curtype+1, gettext(TEXT("_@M907_"))); // Task
			break;
		case IM_AUX:
			wsprintf(Buffer,_T("%d.%d %s"),ModeIndex, curtype+1, gettext(TEXT("_@M908_"))); // Custom
			break;
		case IM_TRI:
			wsprintf(Buffer,_T("%d.%d %s"), ModeIndex, curtype+1, gettext(TEXT("_@M909_"))); // Turn
			break;
		case IM_TRF+IM_TOP:
			wsprintf(Buffer,_T("%d.%d %s"), ModeIndex, IM_TRF+1, gettext(TEXT("_@M910_"))); // Target
			break;
		case IM_TARGET+IM_TOP:
			wsprintf(Buffer,_T("%d.%d %s"), ModeIndex, IM_TARGET+1, gettext(TEXT("_@M911_"))); // Sight
			break;
		default:
			wsprintf(Buffer,_T("error"));
			break;
	}
        LKWriteText(hdc, Buffer, qcolumn[0],qrow[0], 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);

	// R0 C1
	icolor=RGB_WHITE;
	switch(curtype) {
		case IM_THERMAL:
		case IM_CRUISE:
		case IM_TASK:
		case IM_AUX:
			if ( ValidTaskPoint(ActiveWayPoint) != false ) {
				index = Task[ActiveWayPoint].Index;
				if ( index >=0 ) {
					_tcscpy(Buffer, WayPointList[index].Name);
				} else {
					wsprintf(Buffer,gettext(TEXT("_@M912_"))); // [no dest]
					icolor=RGB_AMBER;
				}
			} else {
				wsprintf(Buffer,gettext(TEXT("_@M912_"))); // [no dest]
				icolor=RGB_AMBER;
			}
			break;
		case IM_TRI:
			wsprintf(Buffer,gettext(TEXT("_@M913_"))); // Experimental
			break;
		case IM_TRF+IM_TOP:
		case IM_TARGET+IM_TOP:
			if (ontarget) {
				switch (pTarget->Status ) {
					case LKT_GHOST:
						icolor=RGB_LIGHTYELLOW;
						break;
					case LKT_ZOMBIE:
						icolor=RGB_LIGHTRED;
						break;
					default:
						icolor=RGB_WHITE;
						break;
				}
				//TCHAR status[80];
				if (_tcslen(pTarget->Name) == 1) {
					_stprintf(Buffer,_T("%0x"),pTarget->ID);
				} else {
					_stprintf(Buffer,_T("%s"),pTarget->Name);
				}

			} else {
				_stprintf(Buffer,gettext(TEXT("_@M914_"))); // [no target]
				icolor=RGB_AMBER;
			}

			break;
		default:
			wsprintf(Buffer,_T("error"));
			icolor=RGB_AMBER;
			break;
	}
        LKWriteText(hdc, Buffer, qcolumn[8],qrow[1], 0, WTMODE_NORMAL, WTALIGN_CENTER, icolor, false);

	// R0 C2	= time of day
	if ( (curtype == (IM_TOP+IM_TRF)) || (curtype == (IM_TOP+IM_TARGET)) ) {
		if (ontarget) {
			TCHAR tpas[30];
			Units::TimeToTextDown(tpas,(int)(GPS_INFO.Time - pTarget->Time_Fix));

			switch (pTarget->Status) {
				case LKT_REAL:
					_stprintf(Buffer,_T("LIVE"));
					break;
				case LKT_GHOST:
					_stprintf(Buffer,_T("ghost %s\""),tpas);
					break;
				case LKT_ZOMBIE:
					_stprintf(Buffer,_T("zombie %s\""),tpas);
					break;
				default:
					_stprintf(Buffer,_T("??? %s\""),tpas);
					break;
			}
        		LKWriteText(hdc, Buffer, qcolumn[16],qrow[0], 0, WTMODE_NORMAL, WTALIGN_RIGHT, icolor, false);
		} else {
			LKFormatValue(LK_TIME_LOCALSEC, false, BufferValue, BufferUnit, BufferTitle); // 091219
        		LKWriteText(hdc, BufferValue, qcolumn[16],qrow[0], 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);
		}
		if ( curtype == (IM_TOP+IM_TARGET) ) goto label_Target;
	} else {
		LKFormatValue(LK_TIME_LOCALSEC, false, BufferValue, BufferUnit, BufferTitle); // 091219
        	LKWriteText(hdc, BufferValue, qcolumn[16],qrow[0], 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);
	}

	if (curtype == IM_TRI) goto label_TRI;

	VDrawLine(hdc,rc, qcolumn[0],qrow[2],qcolumn[16],qrow[2],RGB_DARKGREEN);
	VDrawLine(hdc,rc, qcolumn[0],qrow[8],qcolumn[16],qrow[8],RGB_DARKGREEN);

	// R1 C1
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_NEXT_DIST, true, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_CRUISE:
		case IM_TASK:
			showunit=LKFormatValue(LK_NEXT_DIST, true, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_AUX:
			index=GetInfoboxType(1);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_DIST, true, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4], &qrow[3],&qrow[4],&qrow[2]);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4], &qrow[3],&qrow[4],&qrow[2]);
			break;
	}

	// R1 C2
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_BRG, true, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8],&qcolumn[8], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_CRUISE:
		case IM_TASK:
			showunit=LKFormatValue(LK_BRGDIFF, true, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9],&qcolumn[8], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_AUX:
			index=GetInfoboxType(2);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8],&qcolumn[8], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_TO, true, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9],&qcolumn[8], &qrow[3],&qrow[4],&qrow[2]);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9],&qcolumn[8], &qrow[3],&qrow[4],&qrow[2]);
			break;
	}


	// R1 C3
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_NEXT_ALTDIFF, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[12], &qcolumn[12], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_CRUISE:
		case IM_TASK:
			showunit=LKFormatValue(LK_NEXT_GR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[12], &qcolumn[12], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_AUX:
			index=GetInfoboxType(3);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[12], &qcolumn[12], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_GR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[12], &qcolumn[12], &qrow[3],&qrow[4],&qrow[2]);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[12], &qcolumn[12], &qrow[3],&qrow[4],&qrow[2]);
			break;
	}

	// R1 C4
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_NEXT_GR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16],&qcolumn[16], 
											&qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_CRUISE:
		case IM_TASK:
			showunit=LKFormatValue(LK_LD_AVR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16],&qcolumn[16], 
											&qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_AUX:
			index=GetInfoboxType(4);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			_stprintf(BufferUnit,_T(""));
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16],&qcolumn[16], 
											&qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_ALTARRIV, false, BufferValue, BufferUnit, BufferTitle);
			_tcscpy(BufferUnit,_T(""));
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16],&qcolumn[16], 
											&qrow[3],&qrow[4],&qrow[2]);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			_stprintf(BufferUnit,_T(""));
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16],&qcolumn[16], 
											&qrow[3],&qrow[4],&qrow[2]);
			break;
	}

	// R2  C1
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_HNAV, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_CRUISE:
		case IM_TASK:
			showunit=LKFormatValue(LK_NEXT_ALTDIFF, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_AUX:
			index=GetInfoboxType(5);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_ALTDIFF, false, BufferValue, BufferUnit, BufferTitle);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			break;
	}
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4],&qcolumn[4], &qrow[6],&qrow[7],&qrow[5]);

	// R2  C2
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_GNDSPEED, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8], &qcolumn[8], &qrow[6],&qrow[7],&qrow[5]);
			break;
		case IM_CRUISE:
		case IM_TASK:
			showunit=LKFormatValue(LK_BRG, true , BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9], &qcolumn[8], &qrow[6],&qrow[7],&qrow[5]);
			break;
		case IM_AUX:
			index=GetInfoboxType(6);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8], &qcolumn[8], &qrow[6],&qrow[7],&qrow[5]);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_BEARING, true , BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9], &qcolumn[8], &qrow[6],&qrow[7],&qrow[5]);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9], &qcolumn[8], &qrow[6],&qrow[7],&qrow[5]);
			break;
	}

	// R2  C3
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_VARIO, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_CRUISE:
		case IM_TASK:
			showunit=LKFormatValue(LK_LD_CRUISE, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_AUX:
			index=GetInfoboxType(7);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_EMPTY, true , BufferValue, BufferUnit, BufferTitle);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			break;
	}
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[12], &qcolumn[12],&qrow[6],&qrow[7],&qrow[5]);

	// R2  C4
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_FL, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_CRUISE:
		case IM_TASK:
			showunit=LKFormatValue(LK_LD_INST, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_AUX:
			index=GetInfoboxType(8);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			_stprintf(BufferUnit,_T(""));
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_EMPTY, false, BufferValue, BufferUnit, BufferTitle);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			_stprintf(BufferUnit,_T(""));
			break;
	}
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[6],&qrow[7],&qrow[5]);

	// R3  C1
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_TC_GAIN, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_CRUISE:
			showunit=LKFormatValue(LK_HNAV, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_TASK:
			showunit=LKFormatValue(LK_FIN_ALTDIFF, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_AUX:
			index=GetInfoboxType(9);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_ALT, false, BufferValue, BufferUnit, BufferTitle);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			break;
	}
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[9],&qrow[10],&qrow[8]);

	// R3  C2
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_TC_30S, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8], &qcolumn[8],&qrow[9],&qrow[10],&qrow[8]);
			break;
		case IM_CRUISE:
			showunit=LKFormatValue(LK_TRACK, true, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9], &qcolumn[8],&qrow[9],&qrow[10],&qrow[8]);
			break;
		case IM_TASK:
			showunit=LKFormatValue(LK_FIN_DIST, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8], &qcolumn[8],&qrow[9],&qrow[10],&qrow[8]);
			break;
		case IM_AUX:
			index=GetInfoboxType(10);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8], &qcolumn[8],&qrow[9],&qrow[10],&qrow[8]);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_SPEED, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8], &qcolumn[8],&qrow[9],&qrow[10],&qrow[8]);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9], &qcolumn[8],&qrow[9],&qrow[10],&qrow[8]);
			break;
	}

	// R3  C3
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_TC_AVG, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_CRUISE:
			showunit=LKFormatValue(LK_GNDSPEED, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_TASK:
			showunit=LKFormatValue(LK_TASK_DISTCOV, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_AUX:
			index=GetInfoboxType(11);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_VARIO, false, BufferValue, BufferUnit, BufferTitle);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			break;
	}
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[12], &qcolumn[12],&qrow[9],&qrow[10],&qrow[8]);


	// R3  C4
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_TC_ALL, false, BufferValue, BufferUnit, BufferTitle);
			_stprintf(BufferUnit,_T(""));
			break;
		case IM_CRUISE:
			showunit=LKFormatValue(LK_FL, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_TASK:
			showunit=LKFormatValue(LK_FIN_GR, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_AUX:
			index=GetInfoboxType(12);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			_stprintf(BufferUnit,_T(""));
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_AVGVARIO, false, BufferValue, BufferUnit, BufferTitle);
			_stprintf(BufferUnit,_T(""));
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			_stprintf(BufferUnit,_T(""));
			break;
	}
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[9],&qrow[10],&qrow[8]);


	// R4  C1
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_WIND, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[5], &qcolumn[4],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_CRUISE:
			showunit=LKFormatValue(LK_WIND, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[5], &qcolumn[4],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_TASK:
			showunit=LKFormatValue(LK_FIN_ALTDIFF0, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_AUX:
			index=GetInfoboxType(13);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_TRF+IM_TOP:
			LKFormatValue(LK_EMPTY, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[12],&qrow[13],&qrow[11]);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[12],&qrow[13],&qrow[11]);
			break;
	}

	// R4  C2
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_EMPTY, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9], &qcolumn[9],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_CRUISE:
			showunit=LKFormatValue(LK_TL_AVG, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9], &qcolumn[9],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_TASK:
			showunit=LKFormatValue(LK_LKFIN_ETE, false, BufferValue, BufferUnit, BufferTitle); 
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9], &qcolumn[9],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_AUX:
			index=GetInfoboxType(14);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8], &qcolumn[8],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_EIAS, false, BufferValue, BufferUnit, BufferTitle); 
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8], &qcolumn[8],&qrow[12],&qrow[13],&qrow[11]);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9], &qcolumn[9],&qrow[12],&qrow[13],&qrow[11]);
			break;
	}

	// R4  C3
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_EMPTY, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[13], &qcolumn[13],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_CRUISE:
			showunit=LKFormatValue(LK_TC_ALL, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[13], &qcolumn[13],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_TASK:
			showunit=LKFormatValue(LK_SPEEDTASK_ACH, true, BufferValue, BufferUnit, BufferTitle); 
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[13], &qcolumn[13],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_AUX:
			index=GetInfoboxType(15);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[12], &qcolumn[12],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_EMPTY, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[13], &qcolumn[13],&qrow[12],&qrow[13],&qrow[11]);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[13], &qcolumn[13],&qrow[12],&qrow[13],&qrow[11]);
			break;
	}

	// R4  C4
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_EMPTY, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_CRUISE:
		case IM_TASK:
			showunit=LKFormatValue(LK_MC, true, BufferValue, BufferUnit, BufferTitle); 
			break;
		case IM_AUX:
			index=GetInfoboxType(16);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			_stprintf(BufferUnit,_T(""));
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_EMPTY, true, BufferValue, BufferUnit, BufferTitle); 
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			_stprintf(BufferUnit,_T(""));
			break;
	}
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[12],&qrow[13],&qrow[11]);
	goto label_End;

	// This is the TRI page
label_TRI:

	VDrawLine(hdc,rc, qcolumn[0],qrow[2],qcolumn[16],qrow[2],RGB_DARKGREEN);
	DrawTRI(hdc, rc);

     if (InfoBoxLayout::landscape) {
	showunit=true; // 091219
	// right
	LKFormatValue(LK_TRACK, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[14], &qcolumn[14],&qrow[3],&qrow[4],&qrow[2]);
	LKFormatValue(LK_GNDSPEED, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[14], &qcolumn[14],&qrow[6],&qrow[7],&qrow[5]);
	LKFormatValue(LK_HNAV, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[15], &qcolumn[15],&qrow[9],&qrow[10],&qrow[8]);
	LKFormatValue(LK_VARIO, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[14], &qcolumn[14],&qrow[12],&qrow[13],&qrow[11]);

	// left
	LKFormatValue(LK_IAS, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[6],&qrow[7],&qrow[5]);

	_stprintf(BufferValue,_T("%.0f°"),CALCULATED_INFO.BankAngle);
	// LKTOKEN _@M1197_ "Bank"
	_stprintf(BufferTitle, gettext(TEXT("_@M1197_")));
	_stprintf(BufferUnit,_T(""));
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[3],&qrow[4],&qrow[2]);

	LKFormatValue(LK_GLOAD, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[9],&qrow[10],&qrow[8]);
     } else {
	showunit=true;
	// right
	LKFormatValue(LK_TRACK, true, BufferValue, BufferUnit, BufferTitle);
	_stprintf(BufferUnit,_T(""));
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[14], &qcolumn[14],&qrow[3],&qrow[4],&qrow[2]);
	LKFormatValue(LK_GNDSPEED, true, BufferValue, BufferUnit, BufferTitle);
	_stprintf(BufferUnit,_T(""));
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[6],&qrow[7],&qrow[5]);
	LKFormatValue(LK_HNAV, true, BufferValue, BufferUnit, BufferTitle);
	_stprintf(BufferUnit,_T(""));
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[9],&qrow[10],&qrow[8]);
	LKFormatValue(LK_VARIO, true, BufferValue, BufferUnit, BufferTitle);
	_stprintf(BufferUnit,_T(""));
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[12],&qrow[13],&qrow[11]);

	// left
	LKFormatValue(LK_IAS, true, BufferValue, BufferUnit, BufferTitle);
	_stprintf(BufferUnit,_T(""));
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[6],&qrow[7],&qrow[5]);

	_stprintf(BufferValue,_T("%.0f°"),CALCULATED_INFO.BankAngle);
	// LKTOKEN _@M1197_ "Bank"
	_stprintf(BufferTitle, gettext(TEXT("_@M1197_")));
	_stprintf(BufferUnit,_T(""));
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[3],&qrow[4],&qrow[2]);

	LKFormatValue(LK_GLOAD, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[9],&qrow[10],&qrow[8]);
     }

#if 0
	_stprintf(BufferValue,_T("%0.1f"),CALCULATED_INFO.TurnRate);
	_stprintf(BufferTitle,_T("Rate"));
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[9],&qrow[10],&qrow[8]);

	LKFormatValue(LK_GLOAD, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[12],&qrow[13],&qrow[11]);
#endif

        wsprintf(BufferTitle, gettext(TEXT("_@M915_"))); // NOT FOR IFR USAGE
	SelectObject(hdc, LK8PanelSmallFont);
	LKWriteText(hdc, BufferTitle, qcolumn[8],qrow[12], 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_ORANGE, false);

	goto label_End;
	// End of TRI

	// Traffic Target page
	
label_Target:
	VDrawLine(hdc,rc, qcolumn[0],qrow[2],qcolumn[16],qrow[2],RGB_DARKGREEN);
	// pass the sight rectangle to use on the screen. Warning: DrawTarget will cache these values
	// and will not use them after the first run time anymore...
	DrawTarget(hdc, rc, qrow[3],qrow[15],qcolumn[3],qcolumn[13]);

     if (InfoBoxLayout::landscape) {
	showunit=false; // 091219

	// left
	
	showunit=LKFormatValue(LK_TARGET_DIST, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[3],&qrow[4],&qrow[2]);

	showunit=LKFormatValue(LK_TARGET_EIAS, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[6],&qrow[7],&qrow[5]);

	showunit=LKFormatValue(LK_TARGET_AVGVARIO, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[9],&qrow[10],&qrow[8]);

	showunit=LKFormatValue(LK_TARGET_GR, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[12],&qrow[13],&qrow[11]);

	// right
	showunit=LKFormatValue(LK_TARGET_ALTDIFF, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[3],&qrow[4],&qrow[2]);

	showunit=LKFormatValue(LK_EMPTY, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[6],&qrow[7],&qrow[5]);

	showunit=LKFormatValue(LK_EMPTY, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[9],&qrow[10],&qrow[8]);

	showunit=LKFormatValue(LK_TARGET_ALTARRIV, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[12],&qrow[13],&qrow[11]);
     } else {
	showunit=false;

	// left
	
	showunit=LKFormatValue(LK_TARGET_DIST, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[3],&qrow[4],&qrow[2]);

	showunit=LKFormatValue(LK_TARGET_EIAS, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[6],&qrow[7],&qrow[5]);

	showunit=LKFormatValue(LK_TARGET_AVGVARIO, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[9],&qrow[10],&qrow[8]);

	showunit=LKFormatValue(LK_TARGET_GR, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[12],&qrow[13],&qrow[11]);

	// right
	showunit=LKFormatValue(LK_TARGET_ALTDIFF, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[3],&qrow[4],&qrow[2]);

	showunit=LKFormatValue(LK_EMPTY, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[6],&qrow[7],&qrow[5]);

	showunit=LKFormatValue(LK_EMPTY, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[9],&qrow[10],&qrow[8]);

	showunit=LKFormatValue(LK_TARGET_ALTARRIV, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(hdc, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[12],&qrow[13],&qrow[11]);
     }



label_End:
  // restore font and return
  SelectObject(hdc, oldfont); 

}


