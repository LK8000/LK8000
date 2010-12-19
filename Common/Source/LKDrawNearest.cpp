/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKDrawNearest.cpp,v 1.1 2010/12/11 14:56:01 root Exp root $
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
#include "LKObjects.h"

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
  bool isLandable; // VENTA5
  bool isAirport; // VENTA5
  bool isExcluded;
  int  index;
}MapWaypointLabel_t;


extern MapWaypointLabel_t MapWaypointLabelList[];

void MapWindow::DrawNearest(HDC hdc, RECT rc) {


  SIZE WPTextSize, DSTextSize, BETextSize, RETextSize, AATextSize, HLTextSize, MITextSize;
  TCHAR Buffer[LKSIZEBUFFERLARGE];
  static RECT s_sortBox[6]; 
  static TCHAR Buffer1[MAXNEAREST][MAXNUMPAGES][24], Buffer2[MAXNEAREST][MAXNUMPAGES][10], Buffer3[MAXNEAREST][MAXNUMPAGES][10];
  static TCHAR Buffer4[MAXNEAREST][MAXNUMPAGES][12], Buffer5[MAXNEAREST][MAXNUMPAGES][12];
  static short maxnlname;
  char text[30];
  short i, k, iRaw, wlen, rli=0, curpage, drawn_items_onpage;
  double Value;
  COLORREF rcolor;

  // column0 starts after writing 1:2 (ModeIndex:CURTYPE+1) with a different font..
  static short Column0;
  static short Column1, Column2, Column3, Column4, Column5;
  static POINT p1, p2;
  static short rawspace;
  // Printable area for live nearest values
  static short left,right,bottom;
  // one for each mapspace, no matter if 0 and 1 are unused

  // Vertical and horizontal spaces
  #define INTERRAW	1
  #define HEADRAW	NIBLSCALE(6)	
  HBRUSH sortbrush;
  RECT invsel;

  static bool doinit=true;

  if (INVERTCOLORS) {
	#if LKOBJ
  	sortbrush=LKBrush_LightGreen;
  } else {
  	sortbrush=LKBrush_DarkGreen;
	#else
  	if ( (sortbrush=CreateSolidBrush(RGB_LIGHTGREEN)) == NULL) {
		DoStatusMessage(_T("ERR-011 Brush DrawNearest failed"));
  		sortbrush=CreateSolidBrush(RGB_WHITE);
	}
  } else {
  	sortbrush=CreateSolidBrush(RGB_DARKGREEN);
	#endif
  }

  if (doinit) {

  // Set screen borders to avoid writing on extreme pixels
  if ( ScreenSize < (ScreenSize_t)sslandscape ) {
	// Portrait mode has tight horizontal margins...
	left=rc.left+NIBLSCALE(1);
	right=rc.right-NIBLSCALE(1);
  	bottom=rc.bottom-BottomSize-NIBLSCALE(2);
	maxnlname=MAXNLNAME-5; // 7 chars max, 8 sized
  	_stprintf(Buffer,TEXT("MAKSJSMM"));  
  } else {
	left=rc.left+NIBLSCALE(5);
	right=rc.right-NIBLSCALE(5);
  	bottom=rc.bottom-BottomSize;
	maxnlname=MAXNLNAME-3; // 9 chars, sized 10
  	_stprintf(Buffer,TEXT("ABCDEFGHMx")); 
  }


  SelectObject(hdc, LK8InfoBigFont); // Text font for Nearest  was LK8Title
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &WPTextSize);

  _stprintf(Buffer,TEXT("000.0")); 
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &DSTextSize);

  //: Bearing
  _stprintf(Buffer,TEXT("<<123")); 
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &BETextSize);

  //: reqE
  _stprintf(Buffer,TEXT("5299")); 
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &RETextSize);

  //: Altitude Arrival
  _stprintf(Buffer,TEXT("+9999")); 
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &AATextSize);

  SelectObject(hdc, LK8InfoNormalFont);
  _stprintf(Buffer,TEXT("MMMM")); 
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &HLTextSize);

  SelectObject(hdc, LK8PanelMediumFont);  
  _stprintf(Buffer,TEXT("1.1")); 
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &MITextSize);

  //short intercolumn=(right-left-WPTextSize.cx-DSTextSize.cx-BETextSize.cx-RETextSize.cx-AATextSize.cx)/3; // era 4 FIX
  short afterwpname=left+WPTextSize.cx+NIBLSCALE(5);
  short intercolumn=(right-afterwpname- DSTextSize.cx-BETextSize.cx-RETextSize.cx-AATextSize.cx)/3; 

  // Col0 is where APTS 1/3 can be written, after ModeIndex:Curtype
  Column0=MITextSize.cx+LEFTLIMITER+NIBLSCALE(5);
  Column1=left;							// WP align left
  //Column2=Column1+WPTextSize.cx+intercolumn/2;	// DS align right
  Column2=afterwpname+DSTextSize.cx;						// DS align right
  Column3=Column2+intercolumn+BETextSize.cx;			// BE align right
  Column4=Column3+intercolumn+RETextSize.cx;			// RE align right
  Column5=Column4+intercolumn+AATextSize.cx;			// AA align right


  if ( ScreenSize < (ScreenSize_t)sslandscape ) {
  	TopSize=rc.top+HEADRAW*2+HLTextSize.cy;
  	p1.x=0; p1.y=TopSize; p2.x=rc.right; p2.y=p1.y;
  	//TopSize+=(WPTextSize.cy);
  	TopSize+=HEADRAW;
  	Numraws=(bottom - TopSize) / (WPTextSize.cy+(INTERRAW*2));
  	if (Numraws>MAXNEAREST) Numraws=MAXNEAREST;
  	rawspace=(WPTextSize.cy+INTERRAW);
  } else {
  	//TopSize=rc.top+HEADRAW+HLTextSize.cy;
  	TopSize=rc.top+HEADRAW*2+HLTextSize.cy;
  	p1.x=0; p1.y=TopSize; p2.x=rc.right; p2.y=p1.y;
//  	TopSize+=(WPTextSize.cy/4);
  	TopSize+=HEADRAW/2;
  	Numraws=(bottom - TopSize) / (WPTextSize.cy+INTERRAW);
  	if (Numraws>MAXNEAREST) Numraws=MAXNEAREST;
  	rawspace=(WPTextSize.cy+INTERRAW);
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
  //s_sortBox[4].right=Column5+INTERBOX;
  s_sortBox[4].right=rc.right-1;
  s_sortBox[4].top=2;
  s_sortBox[4].bottom=p1.y;
  SortBoxX[4]=s_sortBox[4].right;

  SortBoxY=p1.y;

  Numpages=roundupdivision(MAXNEAREST, Numraws);
  if (Numpages>MAXNUMPAGES) Numpages=MAXNUMPAGES;
  else if (Numpages<1) Numpages=1;

  SelectedRaw[MSM_LANDABLE]=0; SelectedRaw[MSM_AIRPORTS]=0;
  SelectedPage[MSM_LANDABLE]=0; SelectedPage[MSM_AIRPORTS]=0;

  doinit=false;
  #ifndef LKOBJ
  DeleteObject(sortbrush);
  #endif
  return;
  } // doinit

  Numpages=roundupdivision(SortedNumber, Numraws);
  if (Numpages>MAXNUMPAGES) Numpages=MAXNUMPAGES;
  else if (Numpages<1) Numpages=1;

  curpage=SelectedPage[MapSpaceMode];
  if (curpage<0||curpage>=MAXNUMPAGES) { // TODO also >Numpages
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
		if (MapSpaceMode==MSM_LANDABLE) 
			i=SortedLandableIndex[SelectedRaw[MapSpaceMode]+(curpage*Numraws)];
		else
			i=SortedAirportIndex[SelectedRaw[MapSpaceMode] + (curpage*Numraws)];

		if ( !ValidWayPoint(i)) {
			if (SortedNumber>0)
				DoStatusMessage(_T("ERR-019 Invalid selection")); 
			break;
		}
		SelectedWaypoint=i;
		LastDoNearest = GPS_INFO.Time+NEARESTONHOLD; //@ 101003
		PopupWaypointDetails();
		LastDoNearest = 0; //@ 101003
		// SetModeType(LKMODE_MAP,MP_MOVING); EXperimental OFF 101219
		LKevent=LKEVENT_NONE; 
		#ifndef LKOBJ
  		DeleteObject(sortbrush);
		#endif
		return;
		break;
	case LKEVENT_DOWN:
		if (++SelectedRaw[MapSpaceMode] >=Numraws) SelectedRaw[MapSpaceMode]=0;
		LastDoNearest=GPS_INFO.Time+PAGINGTIMEOUT-1.0; //@ 101003
		break;
	case LKEVENT_UP:
		if (--SelectedRaw[MapSpaceMode] <0) SelectedRaw[MapSpaceMode]=Numraws-1;
		LastDoNearest=GPS_INFO.Time+PAGINGTIMEOUT-1.0; //@ 101003
		break;
	case LKEVENT_PAGEUP:
		LKevent=LKEVENT_NONE;
		break;
	case LKEVENT_PAGEDOWN:
		LKevent=LKEVENT_NONE;
		break;
	case LKEVENT_NEWRUN:
		for (i=0; i<MAXNEAREST; i++) {
			for (k=0; k<MAXNUMPAGES; k++) {
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

	if (MapSpaceMode==MSM_LANDABLE) 
 		_stprintf(Buffer,TEXT("LND %d/%d"),  curpage+1,Numpages); 
 	else
 	 	_stprintf(Buffer,TEXT("APT %d/%d"),  curpage+1, Numpages); 
	if (cursortbox == 0)
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
	//oldpen=(HPEN)SelectObject(hdc,hpen);
	//Rectangle(hdc,s_sortBox[cursortbox].left, s_sortBox[cursortbox].top, s_sortBox[cursortbox].right, s_sortBox[cursortbox].bottom); 

	if ( (ScreenSize == (ScreenSize_t)ss640x480) || (ScreenSize == (ScreenSize_t)ss320x240) || (ScreenSize == ss896x672) ) {

		_stprintf(Buffer,TEXT("%d.%d"),ModeIndex,CURTYPE+1);
  		SelectObject(hdc, LK8PanelMediumFont); 
		LKWriteText(hdc, Buffer, LEFTLIMITER, rc.top+TOPLIMITER , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);
  		SelectObject(hdc, LK8InfoNormalFont); 

		if (MapSpaceMode==MSM_LANDABLE) 
			_stprintf(Buffer,TEXT("LNDB %d/%d"), curpage+1,Numpages); 
		else
			_stprintf(Buffer,TEXT("APTS %d/%d"), curpage+1, Numpages); 
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

		if (MapSpaceMode==MSM_LANDABLE) 
			_stprintf(Buffer,TEXT("LNDB %d/%d"),  curpage+1,Numpages); 
		else
			_stprintf(Buffer,TEXT("APTS %d/%d"), curpage+1, Numpages); 
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

  bool ndr=NearestDataReady;
  NearestDataReady=false;

  for (i=0, drawn_items_onpage=0; i<Numraws; i++) {
	iRaw=TopSize+(rawspace*i);
	short curraw=(curpage*Numraws)+i;
	if (curraw>=MAXNEAREST) break;
	if (MapSpaceMode==MSM_LANDABLE) {
		rli=SortedLandableIndex[curraw];
	} else {
		rli=SortedAirportIndex[curraw];
	}


	if (!ndr) {
		goto KeepOldValues;
	}
	if ( ValidWayPoint(rli) ) {

		wlen=wcslen(WayPointList[rli].Name);
		if (wlen>maxnlname) {
			_tcsncpy(Buffer, WayPointList[rli].Name, maxnlname); Buffer[maxnlname]='\0';
		}
		else {
			_tcsncpy(Buffer, WayPointList[rli].Name, wlen); Buffer[wlen]='\0';
		}
		ConvToUpper(Buffer); // 100213 FIX UPPERCASE DRAWNEAREST
		_tcscpy(Buffer1[i][curpage],Buffer); 

		Value=WayPointCalc[rli].Distance*DISTANCEMODIFY;
         	_stprintf(Buffer2[i][curpage],TEXT("%0.1lf"),Value);


		if (DisplayMode != dmCircling) {
			Value = WayPointCalc[rli].Bearing -  GPS_INFO.TrackBearing;

			if (Value < -180.0)
				Value += 360.0;
			else
				if (Value > 180.0)
					Value -= 360.0;

#ifndef __MINGW32__
			if (Value > 1)
				_stprintf(Buffer3[i][curpage], TEXT("%2.0f°»"), Value);
			else
				if (Value < -1)
					_stprintf(Buffer3[i][curpage], TEXT("«%2.0f°"), -Value);
				else
					_tcscpy(Buffer3[i][curpage], TEXT("«»"));
#else
			if (Value > 1)
				_stprintf(Buffer3[i][curpage], TEXT("%2.0fÂ°Â»"), Value);
			else
				if (Value < -1)
					_stprintf(Buffer3[i][curpage], TEXT("Â«%2.0fÂ°"), -Value);
				else
					_tcscpy(Buffer3[i][curpage], TEXT("Â«Â»"));
#endif
		} else
			_stprintf(Buffer3[i][curpage], TEXT("%2.0fÂ°"), WayPointCalc[rli].Bearing); // 101219

		Value=WayPointCalc[rli].GR;
		if (Value<1 || Value>=MAXEFFICIENCYSHOW) 
			_stprintf(Buffer4[i][curpage],_T("---"));
		else {
			if (Value>99) sprintf(text,"%.0f",Value);
			else sprintf(text,"%.1f",Value);
			_stprintf(Buffer4[i][curpage],_T("%S"),text);
		}

//* TESTFIX 091005 REMOVE
		Value=ALTITUDEMODIFY*WayPointCalc[rli].AltArriv[AltArrivMode];
		if (Value <-9999 ||  Value >9999 )
			strcpy(text,"---");
		else
			sprintf(text,"%+.0f",Value);
		wsprintf(Buffer5[i][curpage], TEXT("%S"),text);
//*/

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

		if ( ((WayPointList[rli].Flags & AIRPORT) == AIRPORT) ) // BUGFIX old  rli and not i
		{
			rcolor=RGB_WHITE;
  			SelectObject(hdc, LK8InfoBigFont); // Text font for Nearest
		} else {
			rcolor=RGB_LIGHTYELLOW;
  			SelectObject(hdc, LK8InfoBigItalicFont); // Text font for Nearest
		}
		if ((WayPointCalc[rli].VGR == 3 )|| (!WayPointList[rli].Reachable)) // 091205
			rcolor=RGB_LIGHTRED;
	} else 
			rcolor=RGB_GREY;

	LKWriteText(hdc, Buffer1[i][curpage], Column1, iRaw , 0, WTMODE_NORMAL, WTALIGN_LEFT, rcolor, false);
	
  	SelectObject(hdc, LK8InfoBigFont); // Text font for Nearest
	LKWriteText(hdc, Buffer2[i][curpage], Column2, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

	LKWriteText(hdc, Buffer3[i][curpage], Column3, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

	LKWriteText(hdc, Buffer4[i][curpage], Column4, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

	LKWriteText(hdc, Buffer5[i][curpage], Column5, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

  } 


  if (LKevent==LKEVENT_NEWRUN || LKevent==LKEVENT_NEWPAGE ) {
		LKevent=LKEVENT_NONE;
		#ifndef LKOBJ
  		DeleteObject(sortbrush);
		#endif
		return;
  }

  if (drawn_items_onpage>0) {

	if (SelectedRaw[MapSpaceMode] <0 || SelectedRaw[MapSpaceMode]>(Numraws-1)) {
		#ifndef LKOBJ
  		DeleteObject(sortbrush); 
		#endif
		return;
	}
	if (SelectedRaw[MapSpaceMode] >= drawn_items_onpage) {
		if (LKevent==LKEVENT_DOWN) SelectedRaw[MapSpaceMode]=0;
		else 
		if (LKevent==LKEVENT_UP) SelectedRaw[MapSpaceMode]=drawn_items_onpage-1;
		else {
			DoStatusMessage(_T("Cant find valid raw"));
			SelectedRaw[MapSpaceMode]=0;
		}
	}

	
	invsel.left=left;
	invsel.right=right;
	invsel.top=TopSize+(rawspace*SelectedRaw[MapSpaceMode])+NIBLSCALE(2);
	invsel.bottom=TopSize+(rawspace*(SelectedRaw[MapSpaceMode]+1))-NIBLSCALE(1);
	InvertRect(hdc,&invsel);

  } 

  LKevent=LKEVENT_NONE;
  #ifndef LKOBJ
  DeleteObject(sortbrush);
  #endif
  return;
}

