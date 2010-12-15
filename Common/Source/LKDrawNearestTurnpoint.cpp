/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKDrawNearestTurnpoint.cpp,v 1.1 2010/12/11 18:05:37 root Exp root $
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
#include "RasterTerrain.h" 
#include "LKUtils.h"
#include "LKMapWindow.h"
#if LKOBJ
#include "LKObjects.h"
#endif

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

void MapWindow::DrawNearestTurnpoint(HDC hdc, RECT rc) {

  SIZE WPTextSize, DSTextSize, BETextSize, RETextSize, AATextSize, HLTextSize, MITextSize;
  TCHAR Buffer[LKSIZEBUFFERLARGE];
  static RECT s_sortBox[6]; 
  static TCHAR Buffer1[MAXNEARTURNPOINT][MAXTURNPOINTNUMPAGES][24], Buffer2[MAXNEARTURNPOINT][MAXTURNPOINTNUMPAGES][10];
  static TCHAR Buffer3[MAXNEARTURNPOINT][MAXTURNPOINTNUMPAGES][10];
  static TCHAR Buffer4[MAXNEARTURNPOINT][MAXTURNPOINTNUMPAGES][12], Buffer5[MAXNEARTURNPOINT][MAXTURNPOINTNUMPAGES][12];
  static short s_maxnlname;
  char text[30];
  short i, k, iRaw, wlen, rli=0, curpage, drawn_items_onpage;
  double value;
  COLORREF rcolor;

  static short Column0;
  static short Column1, Column2, Column3, Column4, Column5;
  static POINT p1, p2;
  static short s_rawspace;
  static short left,right,bottom;

  #define INTERRAW	1
  #define HEADRAW	NIBLSCALE(6)	
  HBRUSH sortbrush;
  RECT invsel;

  static bool doinit=true;

  #if LKOBJ
  if (INVERTCOLORS) {
  	sortbrush=LKBrush_LightGreen;
  } else {
  	sortbrush=LKBrush_DarkGreen;
  }
  #else
  if (INVERTCOLORS) {
  	sortbrush=CreateSolidBrush(RGB_LIGHTGREEN);
  } else {
  	sortbrush=CreateSolidBrush(RGB_DARKGREEN);
  }
  #endif

  if (doinit) {

  if ( ScreenSize < (ScreenSize_t)sslandscape ) {
	left=rc.left+NIBLSCALE(1);
	right=rc.right-NIBLSCALE(1);
  	bottom=rc.bottom-BottomSize-NIBLSCALE(2);
	s_maxnlname=MAXNLNAME-5; // 7 chars max, 8 sized
  	_stprintf(Buffer,TEXT("MAKSJSMM"));  
  } else {
	left=rc.left+NIBLSCALE(5);
	right=rc.right-NIBLSCALE(5);
  	bottom=rc.bottom-BottomSize;
	s_maxnlname=MAXNLNAME-3; // 9 chars, sized 10
  	_stprintf(Buffer,TEXT("ABCDEFGHMx")); 
  }

  SelectObject(hdc, LK8InfoBigFont); // Text font for Nearest  was LK8Title
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &WPTextSize);

  _stprintf(Buffer,TEXT("000.0")); 
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &DSTextSize);

  _stprintf(Buffer,TEXT("<<123")); 
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &BETextSize);

  _stprintf(Buffer,TEXT("5299")); 
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &RETextSize);

  _stprintf(Buffer,TEXT("+9999")); 
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &AATextSize);

  SelectObject(hdc, LK8InfoNormalFont); // Heading line  was MapWindow QUI
  _stprintf(Buffer,TEXT("MMMM")); 
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &HLTextSize);

  SelectObject(hdc, LK8PanelMediumFont);  
  _stprintf(Buffer,TEXT("1.1")); 
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &MITextSize);

  short afterwpname=left+WPTextSize.cx+NIBLSCALE(5);
  short intercolumn=(right-afterwpname- DSTextSize.cx-BETextSize.cx-RETextSize.cx-AATextSize.cx)/3; 

  Column0=MITextSize.cx+LEFTLIMITER+NIBLSCALE(5);
  Column1=left;							// WP align left
  Column2=afterwpname+DSTextSize.cx;						// DS align right
  Column3=Column2+intercolumn+BETextSize.cx;			// BE align right
  Column4=Column3+intercolumn+RETextSize.cx;			// RE align right
  Column5=Column4+intercolumn+AATextSize.cx;			// AA align right


  if ( ScreenSize < (ScreenSize_t)sslandscape ) {
  	TopSize=rc.top+HEADRAW*2+HLTextSize.cy;
  	p1.x=0; p1.y=TopSize; p2.x=rc.right; p2.y=p1.y;
  	TopSize+=HEADRAW;
  	TurnpointNumraws=(bottom - TopSize) / (WPTextSize.cy+(INTERRAW*2));
  	if (TurnpointNumraws>MAXNEARTURNPOINT) TurnpointNumraws=MAXNEARTURNPOINT;
  	s_rawspace=(WPTextSize.cy+INTERRAW);
  } else {
  	TopSize=rc.top+HEADRAW*2+HLTextSize.cy;
  	p1.x=0; p1.y=TopSize; p2.x=rc.right; p2.y=p1.y;
  	TopSize+=HEADRAW/2;
  	TurnpointNumraws=(bottom - TopSize) / (WPTextSize.cy+INTERRAW);
  	if (TurnpointNumraws>MAXNEARTURNPOINT) TurnpointNumraws=MAXNEARTURNPOINT;
  	s_rawspace=(WPTextSize.cy+INTERRAW);
  }

#define INTERBOX intercolumn/2

  s_sortBox[0].left=Column0; 
  if ( ScreenSize < (ScreenSize_t)sslandscape ) s_sortBox[0].right=left+WPTextSize.cx-NIBLSCALE(2);
  else s_sortBox[0].right=left+WPTextSize.cx-NIBLSCALE(10);
  s_sortBox[0].top=2;
  s_sortBox[0].bottom=p1.y;
  SortBoxX[0]=s_sortBox[0].right;

  if ( ScreenSize < (ScreenSize_t)sslandscape ) s_sortBox[1].left=Column1+afterwpname-INTERBOX;
  else s_sortBox[1].left=Column1+afterwpname-INTERBOX-NIBLSCALE(2);
  s_sortBox[1].right=Column2+INTERBOX;
  s_sortBox[1].top=2;
  s_sortBox[1].bottom=p1.y;
  SortBoxX[1]=s_sortBox[1].right;

  s_sortBox[2].left=Column2+INTERBOX;
  s_sortBox[2].right=Column3+INTERBOX;
  s_sortBox[2].top=2;
  s_sortBox[2].bottom=p1.y;
  SortBoxX[2]=s_sortBox[2].right;

  s_sortBox[3].left=Column3+INTERBOX;
  s_sortBox[3].right=Column4+INTERBOX;
  s_sortBox[3].top=2;
  s_sortBox[3].bottom=p1.y;
  SortBoxX[3]=s_sortBox[3].right;

  s_sortBox[4].left=Column4+INTERBOX;
  s_sortBox[4].right=rc.right-1;
  s_sortBox[4].top=2;
  s_sortBox[4].bottom=p1.y;
  SortBoxX[4]=s_sortBox[4].right;

  SortBoxY=p1.y;

  TurnpointNumpages=roundupdivision(MAXNEARTURNPOINT, TurnpointNumraws);
  if (TurnpointNumpages>MAXTURNPOINTNUMPAGES) TurnpointNumpages=MAXTURNPOINTNUMPAGES;
  else if (TurnpointNumpages<1) TurnpointNumpages=1;

  SelectedRaw[MSM_NEARTPS]=0;
  SelectedPage[MSM_NEARTPS]=0;

  doinit=false;
  #ifndef LKOBJ
  DeleteObject(sortbrush);
  #endif
  return;
  } // doinit

  TurnpointNumpages=roundupdivision(SortedTurnpointNumber, TurnpointNumraws);
  if (TurnpointNumpages>MAXTURNPOINTNUMPAGES) TurnpointNumpages=MAXTURNPOINTNUMPAGES;
  else if (TurnpointNumpages<1) TurnpointNumpages=1;

  curpage=SelectedPage[MapSpaceMode];
  if (curpage<0||curpage>=MAXTURNPOINTNUMPAGES) { // TODO also >Numpages
	DoStatusMessage(_T("ERR-091 curpage invalid!")); 
	SelectedPage[MapSpaceMode]=0;
	LKevent=LKEVENT_NONE;
	#ifndef LKOBJ
  	DeleteObject(sortbrush); 
	#endif
	return;
  }

  switch (LKevent) {
	case LKEVENT_NONE:
		break;
	case LKEVENT_ENTER:
		LKevent=LKEVENT_NONE;
		i=SortedTurnpointIndex[SelectedRaw[MapSpaceMode]+(curpage*TurnpointNumraws)];

		if ( !ValidWayPoint(i)) {
			if (SortedTurnpointNumber>0)
				DoStatusMessage(_T("ERR-055 Invalid selection")); 
			break;
		}
		SelectedWaypoint=i;
		LastDoNearestTp = GPS_INFO.Time + NEARESTONHOLD; 
		PopupWaypointDetails();
		LastDoNearestTp = 0; //@ 101003
		SetModeType(LKMODE_MAP,MP_MOVING);
		LKevent=LKEVENT_NONE; 
		#ifndef LKOBJ
  		DeleteObject(sortbrush);
		#endif
		return;
		break;
	case LKEVENT_DOWN:
		if (++SelectedRaw[MapSpaceMode] >=TurnpointNumraws) SelectedRaw[MapSpaceMode]=0;
		LastDoNearestTp=GPS_INFO.Time+PAGINGTIMEOUT-1.0; 
		break;
	case LKEVENT_UP:
		if (--SelectedRaw[MapSpaceMode] <0) SelectedRaw[MapSpaceMode]=TurnpointNumraws-1;
		LastDoNearestTp=GPS_INFO.Time+PAGINGTIMEOUT-1.0; 
		break;
	case LKEVENT_PAGEUP:
		LKevent=LKEVENT_NONE;
		break;
	case LKEVENT_PAGEDOWN:
		LKevent=LKEVENT_NONE;
		break;
	case LKEVENT_NEWRUN:
		for (i=0; i<MAXNEARTURNPOINT; i++) {
			for (k=0; k<MAXTURNPOINTNUMPAGES; k++) {
				_stprintf(Buffer1[i][k],_T("------------")); // 12 chars
				_stprintf(Buffer2[i][k],_T("----"));
				_stprintf(Buffer3[i][k],_T("----"));
				_stprintf(Buffer4[i][k],_T("----"));
				_stprintf(Buffer5[i][k],_T("----"));
			}
		}
		break;
	case LKEVENT_NEWPAGE:
		break;
	default:
		LKevent=LKEVENT_NONE;
		break;
  }

  if (INVERTCOLORS)
	  _DrawLine(hdc, PS_SOLID, NIBLSCALE(1), p1, p2, RGB_GREEN, rc);
  else
	  _DrawLine(hdc, PS_SOLID, NIBLSCALE(1), p1, p2, RGB_DARKGREEN, rc);

  SelectObject(hdc, LK8InfoNormalFont); // Heading line

  short cursortbox=SortedMode[MapSpaceMode];

  if ( ScreenSize < (ScreenSize_t)sslandscape ) { // portrait mode
	FillRect(hdc,&s_sortBox[cursortbox], sortbrush); 

	_stprintf(Buffer,TEXT("%d.%d"),ModeIndex,CURTYPE+1);
  	SelectObject(hdc, LK8PanelMediumFont); 
	LKWriteText(hdc, Buffer, LEFTLIMITER, rc.top+TOPLIMITER , 0,  WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);
  	SelectObject(hdc, LK8InfoNormalFont); 

 		_stprintf(Buffer,TEXT("TPS %d/%d"),  curpage+1,TurnpointNumpages); 
	if (cursortbox==0)
 		LKWriteText(hdc, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_BLACK, false);
	else
 		LKWriteText(hdc, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);

	 _stprintf(Buffer,TEXT("Dist")); 
	if (cursortbox==1)
		LKWriteText(hdc, Buffer, Column2, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(hdc, Buffer, Column2, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	_stprintf(Buffer,TEXT("Dir")); 
	if (cursortbox==2)
		LKWriteText(hdc, Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(hdc, Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	_stprintf(Buffer,TEXT("rEff")); 
	if (cursortbox==3)
		LKWriteText(hdc, Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(hdc, Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	_stprintf(Buffer,TEXT("AltA")); 
	if (cursortbox==4)
		LKWriteText(hdc, Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(hdc, Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);


  } else {
	FillRect(hdc,&s_sortBox[cursortbox], sortbrush); 

	if ( (ScreenSize == (ScreenSize_t)ss640x480) || (ScreenSize == (ScreenSize_t)ss320x240)|| ScreenSize==ss896x672 ) {

		_stprintf(Buffer,TEXT("%d.%d"),ModeIndex,CURTYPE+1);
  		SelectObject(hdc, LK8PanelMediumFont); 
		LKWriteText(hdc, Buffer, LEFTLIMITER, rc.top+TOPLIMITER , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);
  		SelectObject(hdc, LK8InfoNormalFont); 

		_stprintf(Buffer,TEXT("TPS %d/%d"), curpage+1, TurnpointNumpages); 
		if (cursortbox==0)
			LKWriteText(hdc, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0,WTMODE_NORMAL, WTALIGN_LEFT, RGB_BLACK, false);
		else
			LKWriteText(hdc, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0,WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);

		_stprintf(Buffer,TEXT("Dist")); 
		if (cursortbox==1)
			LKWriteText(hdc, Buffer, Column2, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(hdc, Buffer, Column2, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

		_stprintf(Buffer,TEXT("Dir")); 
		if (cursortbox==2)
			LKWriteText(hdc, Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(hdc, Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

		_stprintf(Buffer,TEXT("rEff")); 
		if (cursortbox==3)
			LKWriteText(hdc, Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(hdc, Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

		_stprintf(Buffer,TEXT("Arriv")); 
		if (cursortbox==4)
			LKWriteText(hdc, Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(hdc, Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);
	} else {
		_stprintf(Buffer,TEXT("%d.%d"),ModeIndex,CURTYPE+1);
  		SelectObject(hdc, LK8PanelMediumFont); 
		LKWriteText(hdc, Buffer, LEFTLIMITER, rc.top+TOPLIMITER , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);
  		SelectObject(hdc, LK8InfoNormalFont); 

		_stprintf(Buffer,TEXT("TPS %d/%d"),  curpage+1,TurnpointNumpages); 
		if (cursortbox==0)
			LKWriteText(hdc, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0,WTMODE_NORMAL, WTALIGN_LEFT, RGB_BLACK, false);
		else
			LKWriteText(hdc, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0,WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);

		_stprintf(Buffer,TEXT("Distance")); 
		if (cursortbox==1)
			LKWriteText(hdc, Buffer, Column2, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(hdc, Buffer, Column2, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

		_stprintf(Buffer,TEXT("Direction")); 
		if (cursortbox==2)
			LKWriteText(hdc, Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(hdc, Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

		_stprintf(Buffer,TEXT("ReqEff")); 
		if (cursortbox==3)
			LKWriteText(hdc, Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(hdc, Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

		_stprintf(Buffer,TEXT("AltArr")); 
		if (cursortbox==4)
			LKWriteText(hdc, Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(hdc, Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);
	}
	

  } // landscape mode



  SelectObject(hdc, LK8InfoBigFont); // Text font for Nearest

  bool ndr=NearestTurnpointDataReady;
  NearestTurnpointDataReady=false;

  for (i=0, drawn_items_onpage=0; i<TurnpointNumraws; i++) {
	iRaw=TopSize+(s_rawspace*i);
	short curraw=(curpage*TurnpointNumraws)+i;
	if (curraw>=MAXNEARTURNPOINT) break;
	rli=SortedTurnpointIndex[curraw];


	if (!ndr) {
		goto KeepOldValues;
	}
	if ( ValidWayPoint(rli) ) {

		wlen=wcslen(WayPointList[rli].Name);
		if (wlen>s_maxnlname) {
			_tcsncpy(Buffer, WayPointList[rli].Name, s_maxnlname); Buffer[s_maxnlname]='\0';
		}
		else {
			_tcsncpy(Buffer, WayPointList[rli].Name, wlen); Buffer[wlen]='\0';
		}
		ConvToUpper(Buffer); 
		_tcscpy(Buffer1[i][curpage],Buffer); 

		value=WayPointCalc[rli].Distance*DISTANCEMODIFY;
         	_stprintf(Buffer2[i][curpage],TEXT("%0.1lf"),value);


		if (DisplayMode != dmCircling) {
			value = WayPointCalc[rli].Bearing -  GPS_INFO.TrackBearing;

			if (value < -180.0)
				value += 360.0;
			else
				if (value > 180.0)
					value -= 360.0;

#ifndef __MINGW32__
			if (value > 1)
				_stprintf(Buffer3[i][curpage], TEXT("%2.0f°»"), value);
			else
				if (value < -1)
					_stprintf(Buffer3[i][curpage], TEXT("«%2.0f°"), -value);
				else
					_tcscpy(Buffer3[i][curpage], TEXT("«»"));
#else
			if (value > 1)
				_stprintf(Buffer3[i][curpage], TEXT("%2.0fÂ°Â»"), value);
			else
				if (value < -1)
					_stprintf(Buffer3[i][curpage], TEXT("Â«%2.0fÂ°"), -value);
				else
					_tcscpy(Buffer3[i][curpage], TEXT("Â«Â»"));
#endif
		} else
			_stprintf(Buffer3[i][curpage], TEXT("%2.0fÂ°"), WayPointCalc[rli].Bearing); 

		value=WayPointCalc[rli].GR;
		if (value<1 || value>=MAXEFFICIENCYSHOW) 
			_stprintf(Buffer4[i][curpage],_T("---"));
		else {
			if (value>99) sprintf(text,"%.0f",value);
			else sprintf(text,"%.1f",value);
			_stprintf(Buffer4[i][curpage],_T("%S"),text);
		}

		value=ALTITUDEMODIFY*WayPointCalc[rli].AltArriv[AltArrivMode];
		if (value <-9999 ||  value >9999 )
			strcpy(text,"---");
		else
			sprintf(text,"%+.0f",value);
		wsprintf(Buffer5[i][curpage], TEXT("%S"),text);

	} else {
		_stprintf(Buffer1[i][curpage],_T("------------"));
		_stprintf(Buffer2[i][curpage],_T("---"));
		_stprintf(Buffer3[i][curpage],_T("---"));
		_stprintf(Buffer4[i][curpage],_T("---"));
		_stprintf(Buffer5[i][curpage],_T("---"));
	}


KeepOldValues:

	if ( ValidWayPoint(rli) ) {
		drawn_items_onpage++;
		rcolor=RGB_WHITE;
		// Nearest Turnpoint do not have precalculated obstacles, so we cannot use Reachable
		if (WayPointCalc[rli].VGR == 3 ) 
			rcolor=RGB_LIGHTRED;
  		SelectObject(hdc, LK8InfoBigFont); // Text font for Nearest
	} else 
		rcolor=RGB_GREY;

	LKWriteText(hdc, Buffer1[i][curpage], Column1, iRaw , 0, WTMODE_NORMAL, WTALIGN_LEFT, rcolor, false);
	
  	SelectObject(hdc, LK8InfoBigFont); // Text font for Nearest
	LKWriteText(hdc, Buffer2[i][curpage], Column2, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

	LKWriteText(hdc, Buffer3[i][curpage], Column3, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

	LKWriteText(hdc, Buffer4[i][curpage], Column4, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

	LKWriteText(hdc, Buffer5[i][curpage], Column5, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

  }  // for


  if (LKevent==LKEVENT_NEWRUN || LKevent==LKEVENT_NEWPAGE ) {
		LKevent=LKEVENT_NONE;
		#ifndef LKOBJ
  		DeleteObject(sortbrush); 
		#endif
		return;
  }

  if (drawn_items_onpage>0) {

	if (SelectedRaw[MapSpaceMode] <0 || SelectedRaw[MapSpaceMode]>(TurnpointNumraws-1)) {
		DoStatusMessage(_T("ERR-051 Invalid selected raw"));
		#ifndef LKOBJ
  		DeleteObject(sortbrush);
		#endif
  		LKevent=LKEVENT_NONE;
		return;
	}
	if (SelectedRaw[MapSpaceMode] >= drawn_items_onpage) {
		if (LKevent==LKEVENT_DOWN) SelectedRaw[MapSpaceMode]=0;
		else 
		if (LKevent==LKEVENT_UP) SelectedRaw[MapSpaceMode]=drawn_items_onpage-1;
		else {
			DoStatusMessage(_T("ERR-050 Cant find valid raw"));
			SelectedRaw[MapSpaceMode]=0;
		}
	}
	invsel.left=left;
	invsel.right=right;
	invsel.top=TopSize+(s_rawspace*SelectedRaw[MapSpaceMode])+NIBLSCALE(2);
	invsel.bottom=TopSize+(s_rawspace*(SelectedRaw[MapSpaceMode]+1))-NIBLSCALE(1);
	InvertRect(hdc,&invsel);

  } 

  LKevent=LKEVENT_NONE;
  #ifndef LKOBJ
  DeleteObject(sortbrush);
  #endif
  return;
}

