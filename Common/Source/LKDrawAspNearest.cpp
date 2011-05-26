/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
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
#include "LKObjects.h"
#include "LKAirspace.h"

#if (WINDOWSPC>0)
#include <wingdi.h>
#endif

#include "utils/heapcheck.h"

extern void ConvToUpper(TCHAR *str);
extern bool ValidAirspace(int i);

void MapWindow::DrawAspNearest(HDC hdc, RECT rc) {


  SIZE ASPTextSize, DSTextSize, BETextSize, TYTextSize, ACTextSize, HLTextSize, MITextSize;
  TCHAR Buffer[LKSIZEBUFFERLARGE];
  static RECT s_sortBox[6]; 
  static TCHAR Buffer1[MAXNEARAIRSPACES][MAXAIRSPACENUMPAGES][30], Buffer2[MAXNEARAIRSPACES][MAXAIRSPACENUMPAGES][10];
  static TCHAR Buffer3[MAXNEARAIRSPACES][MAXAIRSPACENUMPAGES][10];
  static TCHAR Buffer4[MAXNEARAIRSPACES][MAXAIRSPACENUMPAGES][12], Buffer5[MAXNEARAIRSPACES][MAXAIRSPACENUMPAGES][12];
  static short s_maxnlname;
  short i, k, iRaw, wlen, rli=0, curpage, drawn_items_onpage;
  double value;
  COLORREF rcolor;

  // column0 starts after writing 1:2 (ModeIndex:CURTYPE+1) with a different font..
  static short Column0;
  static short Column1, Column2, Column3, Column4, Column5;
  static POINT p1, p2;
  static short s_rawspace;
  // Printable area for live nearest values
  static short left,right,bottom;
  // one for each mapspace, no matter if 0 and 1 are unused

  // we lock to current mapspace for this drawing
  short curmapspace=MapSpaceMode; 

  static int AspNumraws=0;


  // Vertical and horizontal spaces
  #define INTERRAW	1
  #define HEADRAW	NIBLSCALE(6)	
  HBRUSH sortbrush;
  RECT invsel;

  static bool doinit=true;

  if (INVERTCOLORS) {
  	sortbrush=LKBrush_LightGreen;
  } else {
  	sortbrush=LKBrush_DarkGreen;
  }

  if (doinit) {

  // Set screen borders to avoid writing on extreme pixels
  if ( ScreenSize < (ScreenSize_t)sslandscape ) {
	// Portrait mode has tight horizontal margins...
	left=rc.left+NIBLSCALE(1);
	right=rc.right-NIBLSCALE(1);
  	bottom=rc.bottom-BottomSize-NIBLSCALE(2);
	s_maxnlname=9; 
  	_stprintf(Buffer,TEXT("AKSJSMMMM"));  
  } else {
	left=rc.left+NIBLSCALE(5);
	right=rc.right-NIBLSCALE(5);
  	bottom=rc.bottom-BottomSize;
	s_maxnlname=15; 
  	_stprintf(Buffer,TEXT("MAKSJSMMMMMMMMM"));  
  }


  /// WPT is now AIRSPACE name
  SelectObject(hdc, LK8InfoBigFont); // Text font for Nearest  was LK8Title
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &ASPTextSize);

  // DST is always distance
  _stprintf(Buffer,TEXT("000.0")); 
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &DSTextSize);

  // Bearing
  _stprintf(Buffer,TEXT("<<123")); 
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &BETextSize);

  // TYPE, 3 letters printed
  _stprintf(Buffer,TEXT("CTRA")); 
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &TYTextSize);

  // Active can only be 0 or 1  
  _stprintf(Buffer,TEXT("00")); 
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &ACTextSize);

  SelectObject(hdc, LK8InfoNormalFont);
  _stprintf(Buffer,TEXT("MMMM")); 
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &HLTextSize);

  SelectObject(hdc, LK8PanelMediumFont);  
  _stprintf(Buffer,TEXT("1.1")); 
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &MITextSize);

  short afterwpname=left+ASPTextSize.cx+NIBLSCALE(5);
  short intercolumn=(right-afterwpname- DSTextSize.cx-BETextSize.cx-TYTextSize.cx-ACTextSize.cx)/3; 

  // Col0 is where ASP 1/3 can be written, after ModeIndex:Curtype
  Column0=MITextSize.cx+LEFTLIMITER+NIBLSCALE(5);
  Column1=left;							// WP align left
  Column2=afterwpname+TYTextSize.cx;				// TY align right
  Column3=Column2+intercolumn+BETextSize.cx;			// BE align right
  Column4=Column3+intercolumn+DSTextSize.cx;			// DS align right
  Column5=Column4+intercolumn+ACTextSize.cx;			// AC align right


  if ( ScreenSize < (ScreenSize_t)sslandscape ) {
  	TopSize=rc.top+HEADRAW*2+HLTextSize.cy;
  	p1.x=0; p1.y=TopSize; p2.x=rc.right; p2.y=p1.y;
  	TopSize+=HEADRAW;
  	AspNumraws=(bottom - TopSize) / (ASPTextSize.cy+(INTERRAW*2));
  	if (AspNumraws>MAXNEARAIRSPACES) AspNumraws=MAXNEARAIRSPACES;
  	s_rawspace=(ASPTextSize.cy+INTERRAW);
  } else {
  	TopSize=rc.top+HEADRAW*2+HLTextSize.cy;
  	p1.x=0; p1.y=TopSize; p2.x=rc.right; p2.y=p1.y;
  	TopSize+=HEADRAW/2;
  	AspNumraws=(bottom - TopSize) / (ASPTextSize.cy+INTERRAW);
  	if (AspNumraws>MAXNEARAIRSPACES) AspNumraws=MAXNEARAIRSPACES;
  	s_rawspace=(ASPTextSize.cy+INTERRAW);
  }

#define INTERBOX intercolumn/2

  s_sortBox[0].left=Column0; 
  if ( ScreenSize < (ScreenSize_t)sslandscape ) s_sortBox[0].right=left+ASPTextSize.cx-NIBLSCALE(2);
  else s_sortBox[0].right=left+ASPTextSize.cx-NIBLSCALE(10);
  s_sortBox[0].top=2;
  s_sortBox[0].bottom=p1.y;
  SortBoxX[MSM_AIRSPACES][0]=s_sortBox[0].right;

  if ( ScreenSize < (ScreenSize_t)sslandscape ) s_sortBox[1].left=Column1+afterwpname-INTERBOX;
  else s_sortBox[1].left=Column1+afterwpname-INTERBOX-NIBLSCALE(2);
  s_sortBox[1].right=Column2+INTERBOX;
  s_sortBox[1].top=2;
  s_sortBox[1].bottom=p1.y;
  SortBoxX[MSM_AIRSPACES][1]=s_sortBox[1].right;

  s_sortBox[2].left=Column2+INTERBOX;
  s_sortBox[2].right=Column3+INTERBOX;
  s_sortBox[2].top=2;
  s_sortBox[2].bottom=p1.y;
  SortBoxX[MSM_AIRSPACES][2]=s_sortBox[2].right;

  s_sortBox[3].left=Column3+INTERBOX;
  s_sortBox[3].right=Column4+INTERBOX;
  s_sortBox[3].top=2;
  s_sortBox[3].bottom=p1.y;
  SortBoxX[MSM_AIRSPACES][3]=s_sortBox[3].right;

  s_sortBox[4].left=Column4+INTERBOX;
  s_sortBox[4].right=rc.right-1;
  s_sortBox[4].top=2;
  s_sortBox[4].bottom=p1.y;
  SortBoxX[MSM_AIRSPACES][4]=s_sortBox[4].right;

  SortBoxY[MSM_AIRSPACES]=p1.y;

  AspNumpages=roundupdivision(MAXNEARAIRSPACES, AspNumraws);
  if (AspNumpages>MAXAIRSPACENUMPAGES) AspNumpages=MAXAIRSPACENUMPAGES;
  else if (AspNumpages<1) AspNumpages=1;

  SelectedRaw[MSM_AIRSPACES]=0;
  SelectedPage[MSM_AIRSPACES]=0;

  doinit=false;
  return;
  } // doinit

  bool ndr;
  ndr=DoAirspaces(&GPS_INFO,  &CALCULATED_INFO);

  AspNumpages=roundupdivision(LKNumAirspaces, AspNumraws);
  if (AspNumpages>MAXAIRSPACENUMPAGES) AspNumpages=MAXAIRSPACENUMPAGES;
  else if (AspNumpages<1) AspNumpages=1;

  curpage=SelectedPage[curmapspace];
  if (curpage<0||curpage>=MAXAIRSPACENUMPAGES) { // TODO also >Numpages
	SelectedPage[curmapspace]=0;
	LKevent=LKEVENT_NONE;
	return;
  }

  switch (LKevent) {
	case LKEVENT_NONE:
		break;
	case LKEVENT_ENTER:
		LKevent=LKEVENT_NONE;
		i=LKSortedAirspaces[SelectedRaw[curmapspace] + (curpage*AspNumraws)];

		if ( !ValidAirspace(i)) {
			// todo only if numairspace>0
			DoStatusMessage(_T("ERR-039 Invalid ASP selection")); 
			break;
		}
		LastDoAirspaces = GPS_INFO.Time+NEARESTONHOLD; 
		dlgAirspaceDetails( LKAirspaces[i].Pointer );
		LastDoAirspaces = 0; 
		LKevent=LKEVENT_NONE; 
		return;
		break;
	case LKEVENT_DOWN:
		if (++SelectedRaw[curmapspace] >=AspNumraws) SelectedRaw[curmapspace]=0;
		LastDoAirspaces=GPS_INFO.Time+PAGINGTIMEOUT-1.0; 
		break;
	case LKEVENT_UP:
		if (--SelectedRaw[curmapspace] <0) SelectedRaw[curmapspace]=AspNumraws-1;
		LastDoAirspaces=GPS_INFO.Time+PAGINGTIMEOUT-1.0; 
		break;
	case LKEVENT_PAGEUP:
		LKevent=LKEVENT_NONE;
		break;
	case LKEVENT_PAGEDOWN:
		LKevent=LKEVENT_NONE;
		break;
	case LKEVENT_NEWRUN:
		for (i=0; i<MAXNEARAIRSPACES; i++) {
			for (k=0; k<MAXAIRSPACENUMPAGES; k++) {
				if ( ScreenSize < (ScreenSize_t)sslandscape ) 
					_stprintf(Buffer1[i][k],_T("---------------")); // 15 chars
				else
					_stprintf(Buffer1[i][k],_T("-----------------------")); // 23 chars
				_stprintf(Buffer2[i][k],_T("----"));
				_stprintf(Buffer3[i][k],_T("----"));
				_stprintf(Buffer4[i][k],_T("----"));
				_stprintf(Buffer5[i][k],_T("  "));
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

  short cursortbox=SortedMode[curmapspace];

  if ( ScreenSize < (ScreenSize_t)sslandscape ) { // portrait mode
	FillRect(hdc,&s_sortBox[cursortbox], sortbrush); 

	_stprintf(Buffer,TEXT("%d.%d"),ModeIndex,CURTYPE+1);
  	SelectObject(hdc, LK8PanelMediumFont); 

	LKWriteText(hdc, Buffer, LEFTLIMITER, rc.top+TOPLIMITER , 0,  WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);
  	SelectObject(hdc, LK8InfoNormalFont); 

	_stprintf(Buffer,TEXT("%s %d/%d"), gettext(TEXT("_@M1642_")), curpage+1, AspNumpages);  // ASP

	if (cursortbox == 0)
		LKWriteText(hdc, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_BLACK, false);
	else
		LKWriteText(hdc, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);

	_stprintf(Buffer,gettext(TEXT("_@M752_"))); // Type
	if (cursortbox==1)
		LKWriteText(hdc, Buffer, Column2, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(hdc, Buffer, Column2, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	_stprintf(Buffer,gettext(TEXT("_@M1300_")));  // Dist
	if (cursortbox==2)
		LKWriteText(hdc, Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(hdc, Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	_stprintf(Buffer,gettext(TEXT("_@M1301_")));  // Dir
	if (cursortbox==3)
		LKWriteText(hdc, Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(hdc, Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	// Active mode
	_stprintf(Buffer,TEXT("*")); 
	if (cursortbox==4)
		LKWriteText(hdc, Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(hdc, Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);


  } else {
	FillRect(hdc,&s_sortBox[cursortbox], sortbrush);

	if ( (ScreenSize == (ScreenSize_t)ss640x480) || (ScreenSize == (ScreenSize_t)ss320x240) || (ScreenSize == ss896x672) ) {

		_stprintf(Buffer,TEXT("%d.%d"),ModeIndex,CURTYPE+1);
  		SelectObject(hdc, LK8PanelMediumFont); 
		LKWriteText(hdc, Buffer, LEFTLIMITER, rc.top+TOPLIMITER , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);
  		SelectObject(hdc, LK8InfoNormalFont); 

		_stprintf(Buffer,TEXT("%s %d/%d"), gettext(TEXT("_@M1642_")), curpage+1, AspNumpages);  // ASP

		if (cursortbox==0)
			LKWriteText(hdc, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0,WTMODE_NORMAL, WTALIGN_LEFT, RGB_BLACK, false);
		else
			LKWriteText(hdc, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0,WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);

		_stprintf(Buffer,gettext(TEXT("_@M752_"))); // Type
		if (cursortbox==1)
			LKWriteText(hdc, Buffer, Column2, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(hdc, Buffer, Column2, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

		_stprintf(Buffer,gettext(TEXT("_@M1300_")));  // Dist
		if (cursortbox==2)
			LKWriteText(hdc, Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(hdc, Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

		_stprintf(Buffer,gettext(TEXT("_@M1301_")));  // dir
		if (cursortbox==3)
			LKWriteText(hdc, Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(hdc, Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

		_stprintf(Buffer,TEXT("*"));
		if (cursortbox==4)
			LKWriteText(hdc, Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(hdc, Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);
	} else {
		_stprintf(Buffer,TEXT("%d.%d"),ModeIndex,CURTYPE+1);
  		SelectObject(hdc, LK8PanelMediumFont); 
		LKWriteText(hdc, Buffer, LEFTLIMITER, rc.top+TOPLIMITER , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);
  		SelectObject(hdc, LK8InfoNormalFont); 

		_stprintf(Buffer,TEXT("%s %d/%d"), gettext(TEXT("_@M1642_")), curpage+1, AspNumpages);  // ASP

		if (cursortbox==0)
			LKWriteText(hdc, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0,WTMODE_NORMAL, WTALIGN_LEFT, RGB_BLACK, false);
		else
			LKWriteText(hdc, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0,WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);

		_stprintf(Buffer,gettext(TEXT("_@M752_")));  // Type
		if (cursortbox==1)
			LKWriteText(hdc, Buffer, Column2, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(hdc, Buffer, Column2, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

		_stprintf(Buffer,gettext(TEXT("_@M1300_")));  // Dist
		if (cursortbox==2)
			LKWriteText(hdc, Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(hdc, Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

		_stprintf(Buffer,gettext(TEXT("_@M1301_")));  // Dir
		if (cursortbox==3)
			LKWriteText(hdc, Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(hdc, Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

		_stprintf(Buffer,TEXT("*")); 
		if (cursortbox==4)
			LKWriteText(hdc, Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(hdc, Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);
	}
	

  } // landscape mode


  SelectObject(hdc, LK8InfoBigFont); // Text font for Nearest


  int *psortedindex;
  psortedindex=LKSortedAirspaces;

  for (i=0, drawn_items_onpage=0; i<AspNumraws; i++) {
	iRaw=TopSize+(s_rawspace*i);
	short curraw=(curpage*AspNumraws)+i;
	if (curraw>=MAXNEARAIRSPACES) break;

	rli=*(psortedindex+curraw);

	if (!ndr) {
		goto KeepOldValues;
	}
	if ( ValidAirspace(rli) ) {

		//
		// AIRSPACE NAME
		//
		wlen=wcslen(LKAirspaces[rli].Name);
		if (wlen>s_maxnlname) {
			_tcsncpy(Buffer, LKAirspaces[rli].Name, s_maxnlname); Buffer[s_maxnlname]='\0';
		}
		else {
			_tcsncpy(Buffer, LKAirspaces[rli].Name, wlen); Buffer[wlen]='\0';
		}
		ConvToUpper(Buffer);
		_tcscpy(Buffer1[i][curpage],Buffer); 


		//
		// AIRSPACE TYPE
		//
		wlen=wcslen(LKAirspaces[rli].Type);
		#define LKASP_TYPE_LEN	4
		if (wlen>LKASP_TYPE_LEN) {
			_tcsncpy(Buffer, LKAirspaces[rli].Type, LKASP_TYPE_LEN); Buffer[LKASP_TYPE_LEN]='\0';
		}
		else {
			_tcsncpy(Buffer, LKAirspaces[rli].Type, wlen); Buffer[wlen]='\0';
		}
		ConvToUpper(Buffer);
		_tcscpy(Buffer2[i][curpage],Buffer); 

		
		//
		// AIRSPACE DISTANCE
		//
		value=LKAirspaces[rli].Distance*DISTANCEMODIFY;
         	_stprintf(Buffer3[i][curpage],TEXT("%0.1lf"),value);

/* -------------
		if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
			value = WayPointCalc[rli].Bearing -  GPS_INFO.TrackBearing;

			if (value < -180.0)
				value += 360.0;
			else
				if (value > 180.0)
					value -= 360.0;

#ifndef __MINGW32__
			if (value > 1)
				_stprintf(Buffer3[i][curpage], TEXT("%2.0f��"), value);
			else
				if (value < -1)
					_stprintf(Buffer3[i][curpage], TEXT("�%2.0f�"), -value);
				else
					_tcscpy(Buffer3[i][curpage], TEXT("��"));
#else
			if (value > 1)
				_stprintf(Buffer3[i][curpage], TEXT("%2.0f°»"), value);
			else
				if (value < -1)
					_stprintf(Buffer3[i][curpage], TEXT("«%2.0f°"), -value);
				else
					_tcscpy(Buffer3[i][curpage], TEXT("«»"));
#endif
		} else
			_stprintf(Buffer3[i][curpage], TEXT("%2.0f°"), WayPointCalc[rli].Bearing); // 101219
 ----------- */

		//
		// AIRSPACE BEARING DIFFERENCE, OR BEARING IF CIRCLING
		//
		_stprintf(Buffer4[i][curpage], TEXT("%2.0f°"), LKAirspaces[rli].Bearing);

/*
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
*/

		//
		// AIRSPACE ACTIVE OR NOT
		//
		_stprintf(Buffer5[i][curpage], TEXT("%s"), LKAirspaces[rli].Enabled ? _T("+") : _T("-"));

	} else {
		if ( ScreenSize < (ScreenSize_t)sslandscape ) 
			_stprintf(Buffer1[i][curpage], _T("---------------")); // 15 chars
		else
			_stprintf(Buffer1[i][curpage], _T("-----------------------")); // 23 chars
		_stprintf(Buffer2[i][curpage],_T("----"));
		_stprintf(Buffer3[i][curpage],_T("----"));
		_stprintf(Buffer4[i][curpage],_T("----"));
		_stprintf(Buffer5[i][curpage],_T("  "));
	}


KeepOldValues:

	if ( ValidAirspace(rli) ) {

		drawn_items_onpage++;

		if (!LKAirspaces[rli].Enabled) {
			rcolor=RGB_LIGHTRED;
  			SelectObject(hdc, LK8InfoBigItalicFont); 
		} else {
			rcolor=RGB_WHITE;
  			SelectObject(hdc, LK8InfoBigFont); 
		}
	} else {
		rcolor=RGB_GREY;
	}

	LKWriteText(hdc, Buffer1[i][curpage], Column1, iRaw , 0, WTMODE_NORMAL, WTALIGN_LEFT, rcolor, false);
	
  	SelectObject(hdc, LK8InfoBigFont); // Text font for Nearest
	LKWriteText(hdc, Buffer2[i][curpage], Column2, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

	LKWriteText(hdc, Buffer3[i][curpage], Column3, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

	LKWriteText(hdc, Buffer4[i][curpage], Column4, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

	LKWriteText(hdc, Buffer5[i][curpage], Column5, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

  } 


  if (LKevent==LKEVENT_NEWRUN || LKevent==LKEVENT_NEWPAGE ) {
		LKevent=LKEVENT_NONE;
		return;
  }

  if (drawn_items_onpage>0) {

	if (SelectedRaw[curmapspace] <0 || SelectedRaw[curmapspace]>(AspNumraws-1)) {
		return;
	}
	if (SelectedRaw[curmapspace] >= drawn_items_onpage) {
		if (LKevent==LKEVENT_DOWN) SelectedRaw[curmapspace]=0;
		else 
		if (LKevent==LKEVENT_UP) SelectedRaw[curmapspace]=drawn_items_onpage-1;
		else {
			DoStatusMessage(_T("Cant find valid raw"));
			SelectedRaw[curmapspace]=0;
		}
	}

	
	invsel.left=left;
	invsel.right=right;
	invsel.top=TopSize+(s_rawspace*SelectedRaw[curmapspace])+NIBLSCALE(2);
	invsel.bottom=TopSize+(s_rawspace*(SelectedRaw[curmapspace]+1))-NIBLSCALE(1);
	InvertRect(hdc,&invsel);

  } 

  LKevent=LKEVENT_NONE;
  return;
}

// True if the i airspace is existing and valid
bool ValidAirspace(int i) {

	return true;
}
