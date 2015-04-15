/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKMapWindow.h"
#include "LKObjects.h"
#include "RGB.h"
#include "Dialogs.h"
#include "DoInits.h"
#include "InputEvents.h"
#include "ScreenGeometry.h"

void MapWindow::DrawThermalHistory(LKSurface& Surface, const RECT& rc) {

  SIZE WPTextSize, DSTextSize, BETextSize, RETextSize, AATextSize, HLTextSize, MITextSize;
  TCHAR Buffer[LKSIZEBUFFERLARGE];
  static RECT s_sortBox[6]; 
  static TCHAR Buffer1[MAXTHISTORY][MAXTHISTORYNUMPAGES][24], Buffer2[MAXTHISTORY][MAXTHISTORYNUMPAGES][10];
  static TCHAR Buffer3[MAXTHISTORY][MAXTHISTORYNUMPAGES][10];
  static TCHAR Buffer4[MAXTHISTORY][MAXTHISTORYNUMPAGES][12], Buffer5[MAXTHISTORY][MAXTHISTORYNUMPAGES][12];
  static short s_maxnlname;
  short i, k, iRaw, wlen, rli=0, curpage, drawn_items_onpage;
  double value;
  LKColor rcolor;

  // column0 starts after writing 1:2 (ModeIndex:CURTYPE+1) with a different font..
  static short Column0;
  static short Column1, Column2, Column3, Column4, Column5;
  static POINT p1, p2;
  static short s_rawspace;
  // Printable area for live nearest values
  static short left,right,bottom;
  // one for each mapspace, no matter if 0 and 1 are unused

  short curmapspace=MapSpaceMode;
  static int thistoryNumraws=0;
  // Vertical and horizontal spaces
  #define INTERRAW	1
  #define HEADRAW	NIBLSCALE(6)	
  BrushReference sortbrush;
  RECT invsel;

  if (INVERTCOLORS) {
  	sortbrush=LKBrush_LightGreen;
  } else {
  	sortbrush=LKBrush_DarkGreen;
  }

  if (DoInit[MDI_DRAWTHERMALHISTORY]) {

  if ( !ScreenLandscape ) {
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


  Surface.SelectObject(LK8InfoBigFont); // Text font for Nearest  was LK8Title
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &WPTextSize);

  _stprintf(Buffer,TEXT("000.0")); 
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &DSTextSize);

  _stprintf(Buffer,TEXT("<<123")); 
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &BETextSize);

  _stprintf(Buffer,TEXT("5299")); 
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &RETextSize);

  _stprintf(Buffer,TEXT("+9999")); 
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &AATextSize);

  Surface.SelectObject(LK8InfoNormalFont); // Heading line  was MapWindow QUI
  _stprintf(Buffer,TEXT("MMMM")); 
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &HLTextSize);

  Surface.SelectObject(LK8PanelMediumFont);
  _stprintf(Buffer,TEXT("4.4")); 
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &MITextSize);

  short afterwpname=left+WPTextSize.cx+NIBLSCALE(5);
  short intercolumn=(right-afterwpname- DSTextSize.cx-BETextSize.cx-RETextSize.cx-AATextSize.cx)/3; 

  Column0=MITextSize.cx+LEFTLIMITER+NIBLSCALE(5);
  Column1=left;							// WP align left
  Column2=afterwpname+DSTextSize.cx;						// DS align right
  Column3=Column2+intercolumn+BETextSize.cx;			// BE align right
  Column4=Column3+intercolumn+RETextSize.cx;			// RE align right
  Column5=Column4+intercolumn+AATextSize.cx;			// AA align right


  if ( !ScreenLandscape ) {
  	TopSize=rc.top+HEADRAW*2+HLTextSize.cy;
  	p1.x=0; p1.y=TopSize; p2.x=rc.right; p2.y=p1.y;
  	TopSize+=HEADRAW;
  	thistoryNumraws=(bottom - TopSize) / (WPTextSize.cy+(INTERRAW*2));
  	if (thistoryNumraws>MAXTHISTORY) thistoryNumraws=MAXTHISTORY;
  	s_rawspace=(WPTextSize.cy+INTERRAW);
  } else {
  	TopSize=rc.top+HEADRAW*2+HLTextSize.cy;
  	p1.x=0; p1.y=TopSize; p2.x=rc.right; p2.y=p1.y;
  	TopSize+=HEADRAW/2;
  	thistoryNumraws=(bottom - TopSize) / (WPTextSize.cy+INTERRAW);
  	if (thistoryNumraws>MAXTHISTORY) thistoryNumraws=MAXTHISTORY;
  	s_rawspace=(WPTextSize.cy+INTERRAW);
  }

#define INTERBOX intercolumn/2

  // Thermal name
  s_sortBox[0].left=Column0; // FIX 090925 era solo 0
  #ifdef WIN32
  if ( !ScreenLandscape ) s_sortBox[0].right=left+WPTextSize.cx-NIBLSCALE(2);
  #else
  if ( !ScreenLandscape ) s_sortBox[0].right=left+WPTextSize.cx+NIBLSCALE(3);
  #endif
  else s_sortBox[0].right=left+WPTextSize.cx-NIBLSCALE(10);
  s_sortBox[0].top=2;
  s_sortBox[0].bottom=p1.y;
  SortBoxX[MSM_THERMALS][0]=s_sortBox[0].right;

  // Distance
  #ifdef WIN32
  if ( !ScreenLandscape ) s_sortBox[1].left=Column1+afterwpname-INTERBOX;
  else s_sortBox[1].left=Column1+afterwpname-INTERBOX-NIBLSCALE(2);
  #else
  s_sortBox[1].left=s_sortBox[0].right;
  #endif
  s_sortBox[1].right=Column2+INTERBOX;
  s_sortBox[1].top=2;
  s_sortBox[1].bottom=p1.y;
  SortBoxX[MSM_THERMALS][1]=s_sortBox[1].right;

  // Bearing
  s_sortBox[2].left=Column2+INTERBOX;
  s_sortBox[2].right=Column3+INTERBOX;
  s_sortBox[2].top=2;
  s_sortBox[2].bottom=p1.y;
  SortBoxX[MSM_THERMALS][2]=s_sortBox[2].right;

  // Vario
  s_sortBox[3].left=Column3+INTERBOX;
  s_sortBox[3].right=Column4+INTERBOX;
  s_sortBox[3].top=2;
  s_sortBox[3].bottom=p1.y;
  SortBoxX[MSM_THERMALS][3]=s_sortBox[3].right;

  // Altitude
  s_sortBox[4].left=Column4+INTERBOX;
  //s_sortBox[4].right=Column5+INTERBOX;
  s_sortBox[4].right=rc.right-1;
  s_sortBox[4].top=2;
  s_sortBox[4].bottom=p1.y;
  SortBoxX[MSM_THERMALS][4]=s_sortBox[4].right;

  SortBoxY[MSM_THERMALS]=p1.y;

  THistoryNumpages=roundupdivision(MAXTHISTORY, thistoryNumraws);
  if (THistoryNumpages>MAXTHISTORYNUMPAGES) THistoryNumpages=MAXTHISTORYNUMPAGES;
  else if (THistoryNumpages<1) THistoryNumpages=1;

  SelectedRaw[MSM_THERMALS]=0;
  SelectedPage[MSM_THERMALS]=0;

  DoInit[MDI_DRAWTHERMALHISTORY]=false;
  return;
  } // doinit

  DoThermalHistory(&DrawInfo,  &DerivedDrawInfo);

  THistoryNumpages=roundupdivision(LKNumThermals, thistoryNumraws);
  if (THistoryNumpages>MAXTHISTORYNUMPAGES) THistoryNumpages=MAXTHISTORYNUMPAGES;
  else if (THistoryNumpages<1) THistoryNumpages=1;

  curpage=SelectedPage[curmapspace];
  if (curpage<0||curpage>=MAXTHISTORYNUMPAGES) {
	#if TESTBENCH
	DoStatusMessage(_T("ERR-041 thermals curpage invalid"));  // selection while waiting for data ready
	#endif
	SelectedPage[curmapspace]=0;
	LKevent=LKEVENT_NONE;
	return;
  }

  switch (LKevent) {
	case LKEVENT_NONE:
		break;
	case LKEVENT_ENTER:
		LKevent=LKEVENT_NONE;
		i=LKSortedThermals[SelectedRaw[curmapspace]+(curpage*thistoryNumraws)];

		if ( (i<0) || (i>=MAXTHISTORY) || (CopyThermalHistory[i].Valid != true) ) {
			#if 0 // selection while waiting for data ready
			if (LKNumThermals>0)
				DoStatusMessage(_T("ERR-045 Invalid selection")); 
			#endif
			break;
		}
		LKevent=LKEVENT_NONE; 
        InputEvents::processPopupDetails(InputEvents::PopupThermal, i);
		break;
	case LKEVENT_DOWN:
		if (++SelectedRaw[curmapspace] >=thistoryNumraws) SelectedRaw[curmapspace]=0;
		// Reset LastDoThermalH so that it wont be updated while selecting an item
		LastDoThermalH=DrawInfo.Time+PAGINGTIMEOUT-1.0;
		break;
	case LKEVENT_UP:
		if (--SelectedRaw[curmapspace] <0) SelectedRaw[curmapspace]=thistoryNumraws-1;
		LastDoThermalH=DrawInfo.Time+PAGINGTIMEOUT-1.0;
		break;
	case LKEVENT_PAGEUP:
		LKevent=LKEVENT_NONE;
		break;
	case LKEVENT_PAGEDOWN:
		LKevent=LKEVENT_NONE;
		break;
	case LKEVENT_NEWRUN:
		for (i=0; i<MAXTHISTORY; i++) {
			for (k=0; k<MAXTHISTORYNUMPAGES; k++) {
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
	  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), p1, p2, RGB_GREEN, rc);
  else
	  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), p1, p2, RGB_DARKGREEN, rc);

  Surface.SelectObject(LK8InfoNormalFont); // Heading line

  short cursortbox=SortedMode[curmapspace];

  if ( !ScreenLandscape ) { // portrait mode
	Surface.FillRect(&s_sortBox[cursortbox], sortbrush);
	_stprintf(Buffer,TEXT("%d.%d"),ModeIndex,CURTYPE+1);
  	Surface.SelectObject(LK8PanelMediumFont);
	LKWriteText(Surface, Buffer, LEFTLIMITER, rc.top+TOPLIMITER , 0,  WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);
  	Surface.SelectObject(LK8InfoNormalFont);

	_stprintf(Buffer,TEXT("%s %d/%d"), gettext(TEXT("_@M1670_")), curpage+1,THistoryNumpages); 
	if (cursortbox==0)
 		LKWriteText(Surface, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_BLACK, false);
	else
 		LKWriteText(Surface, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);

	// LKTOKEN _@M1300_ "Dist"
	_tcscpy(Buffer, gettext(TEXT("_@M1300_"))); 
	if (cursortbox==1)
		LKWriteText(Surface, Buffer, Column2, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(Surface, Buffer, Column2, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	// LKTOKEN _@M1301_ "Dir"
	_tcscpy(Buffer, gettext(TEXT("_@M1301_"))); 
	if (cursortbox==2)
		LKWriteText(Surface, Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(Surface, Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	_tcscpy(Buffer, gettext(TEXT("_@M1673_")));  // Avg
	if (cursortbox==3)
		LKWriteText(Surface, Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(Surface, Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	_tcscpy(Buffer, gettext(TEXT("_@M1307_")));  // AltArr
	if (cursortbox==4)
		LKWriteText(Surface, Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(Surface, Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);


  } else {
	Surface.FillRect(&s_sortBox[cursortbox], sortbrush);

	bool compact= (ScreenGeometry==SCREEN_GEOMETRY_43);


	_stprintf(Buffer,TEXT("%d.%d"),ModeIndex,CURTYPE+1);
  	Surface.SelectObject(LK8PanelMediumFont);
	LKWriteText(Surface, Buffer, LEFTLIMITER, rc.top+TOPLIMITER , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);
  	Surface.SelectObject(LK8InfoNormalFont);

 	// LKTOKEN _@M1670_ "THE"
	_stprintf(Buffer,TEXT("%s %d/%d"), gettext(TEXT("_@M1670_")), curpage+1,THistoryNumpages);  
	if (cursortbox==0)
		LKWriteText(Surface, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0,WTMODE_NORMAL, WTALIGN_LEFT, RGB_BLACK, false);
	else
		LKWriteText(Surface, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0,WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);


	// LKTOKEN _@M1300_ "Dist"
	// LKTOKEN _@M1304_ "Distance"
	_tcscpy(Buffer,gettext(compact?TEXT("_@M1300_"):TEXT("_@M1304_")));
	if (cursortbox==1)
		LKWriteText(Surface, Buffer, Column2, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(Surface, Buffer, Column2, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);


	// LKTOKEN _@M1301_ "Dir"
	// LKTOKEN _@M1305_ "Direction"
	_tcscpy(Buffer,gettext(compact?TEXT("_@M1301_"):TEXT("_@M1305_")));
	if (cursortbox==2)
		LKWriteText(Surface, Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(Surface, Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);


	_tcscpy(Buffer, gettext(TEXT("_@M1673_")));  // Avg
	if (cursortbox==3)
		LKWriteText(Surface, Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(Surface, Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);


	_tcscpy(Buffer, gettext(TEXT("_@M1307_")));  // AltArr
	if (cursortbox==4)
		LKWriteText(Surface, Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(Surface, Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);
	

  } // landscape mode


  Surface.SelectObject(LK8InfoBigFont); // Text font for Nearest

  #ifdef DEBUG_LKT_DRAWTHISTORY
  TCHAR v2buf[100]; 
  _stprintf(v2buf,_T("MAXTHISTORY=%d LKNumTherm=%d / thistoryNumraws=%d THistoryNumpages=%d calc=%d\n"),MAXTHISTORY, LKNumThermals,thistoryNumraws, THistoryNumpages, (short)(ceil(MAXTHISTORY/thistoryNumraws)));
  StartupStore(v2buf);
  #endif

  for (i=0, drawn_items_onpage=0; i<thistoryNumraws; i++) {
	iRaw=TopSize+(s_rawspace*i);
	short curraw=(curpage*thistoryNumraws)+i;
	if (curraw>=MAXTHISTORY) break;
	rli=LKSortedThermals[curraw];

	#ifdef DEBUG_LKT_DRAWTHISTORY
	StartupStore(_T("..LKDrawThistory page=%d curraw=%d rli=%d \n"),
		curpage,curraw,rli);
	#endif

	if ( (rli>=0) && (CopyThermalHistory[rli].Valid==true) ) {

		// Thermal name
		wlen=_tcslen(CopyThermalHistory[rli].Name);

                if (wlen>s_maxnlname) {
                        LK_tcsncpy(Buffer, CopyThermalHistory[rli].Name, s_maxnlname);
                }
                else {
                        LK_tcsncpy(Buffer, CopyThermalHistory[rli].Name, wlen);
                }
		if (IsThermalMultitarget(rli)) {
			TCHAR buffer2[40];
			_stprintf(buffer2,_T(">%s"),Buffer);
			_tcscpy(Buffer,buffer2);
		}
        CharUpper(Buffer);

		_tcscpy(Buffer1[i][curpage],Buffer); 

		// Distance
		value=CopyThermalHistory[rli].Distance*DISTANCEMODIFY;
         	_stprintf(Buffer2[i][curpage],TEXT("%0.1lf"),value);

		// relative bearing

		if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
			value = CopyThermalHistory[rli].Bearing -  DrawInfo.TrackBearing;

			if (value < -180.0)
				value += 360.0;
			else
				if (value > 180.0)
					value -= 360.0;

			if (value > 1)
				_stprintf(Buffer3[i][curpage], TEXT("%2.0f%s%s"), value, gettext(_T("_@M2179_")),gettext(_T("_@M2183_")));
			else
				if (value < -1)
					_stprintf(Buffer3[i][curpage], TEXT("%s%2.0f%s"),gettext(_T("_@M2182_")), -value, gettext(_T("_@M2179_")));
				else
					_stprintf(Buffer3[i][curpage], TEXT("%s%s"), gettext(_T("_@M2182_")), gettext(_T("_@M2183_")));
		} else {
			_stprintf(Buffer3[i][curpage], _T("%2.0f%s"), CopyThermalHistory[rli].Bearing, gettext(_T("_@M2179_")));
		}
			

		// Average lift
		value=LIFTMODIFY*CopyThermalHistory[rli].Lift;
		if (value<-99 || value>99) 
			_stprintf(Buffer4[i][curpage],_T("---"));
		else {
			_stprintf(Buffer4[i][curpage],_T("%+.1f"),value);
		}

		// Altitude
		value=ALTITUDEMODIFY*CopyThermalHistory[rli].Arrival;
		if (value<-1000 || value >45000 )
			_stprintf(Buffer5[i][curpage],_T("---"));
		else
			_stprintf(Buffer5[i][curpage],_T("%.0f"),value);
		

	} else {
		// Empty thermals, fill in all empty data and maybe break loop
		_stprintf(Buffer1[i][curpage],_T("------------"));
		_stprintf(Buffer2[i][curpage],_T("---"));
		_stprintf(Buffer3[i][curpage],_T("---"));
		_stprintf(Buffer4[i][curpage],_T("---"));
		_stprintf(Buffer5[i][curpage],_T("---"));
	}


	if ((rli>=0) && (CopyThermalHistory[rli].Valid==true)) {
		drawn_items_onpage++;

		if (CopyThermalHistory[rli].Arrival >=0) {
			rcolor=RGB_WHITE;
  			Surface.SelectObject(LK8InfoBigFont);
		} else {
			rcolor=RGB_LIGHTRED;
  			Surface.SelectObject(LK8InfoBigItalicFont);
		}

	} else 
		rcolor=RGB_GREY;

	LKWriteText(Surface, Buffer1[i][curpage], Column1, iRaw , 0, WTMODE_NORMAL, WTALIGN_LEFT, rcolor, false);
	
  	Surface.SelectObject(LK8InfoBigFont); // Text font for Nearest
	LKWriteText(Surface, Buffer2[i][curpage], Column2, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

	LKWriteText(Surface, Buffer3[i][curpage], Column3, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

	LKWriteText(Surface, Buffer4[i][curpage], Column4, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

	LKWriteText(Surface, Buffer5[i][curpage], Column5, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

  }  // for


  if (LKevent==LKEVENT_NEWRUN || LKevent==LKEVENT_NEWPAGE ) {
		LKevent=LKEVENT_NONE;
		return;
  }

  if (drawn_items_onpage>0) {

	if (SelectedRaw[curmapspace] <0 || SelectedRaw[curmapspace]>(thistoryNumraws-1)) {
  		LKevent=LKEVENT_NONE; // 100328
		return;
	}
	if (SelectedRaw[curmapspace] >= drawn_items_onpage) {
		if (LKevent==LKEVENT_DOWN) SelectedRaw[curmapspace]=0;
		else 
		if (LKevent==LKEVENT_UP) SelectedRaw[curmapspace]=drawn_items_onpage-1;
		else {
			SelectedRaw[curmapspace]=0;
		}
	}
	invsel.left=left;
	invsel.right=right;
	invsel.top=TopSize+(s_rawspace*SelectedRaw[curmapspace])+NIBLSCALE(2);
	invsel.bottom=TopSize+(s_rawspace*(SelectedRaw[curmapspace]+1))-NIBLSCALE(1);
	Surface.InvertRect(invsel);

  } 

  LKevent=LKEVENT_NONE;
  return;
}

