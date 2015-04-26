/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKDrawCommon.cpp,v 1.1 2010/12/11 14:07:24 root Exp root $
*/

#include "externs.h"
#include "LKMapWindow.h"
#include "RGB.h"
#include "DoInits.h"
#include "InputEvents.h"
#include "ScreenGeometry.h"

void MapWindow::DrawCommon(LKSurface& Surface, const RECT& rc) {

  SIZE WPTextSize, DSTextSize, BETextSize, RETextSize, AATextSize, HLTextSize, MITextSize;
  SIZE phdrTextSize;
  TCHAR Buffer[LKSIZEBUFFERLARGE];
  static RECT s_sortBox[6]; 
  static TCHAR Buffer1[MAXCOMMON][MAXCOMMONNUMPAGES][24], Buffer2[MAXCOMMON][MAXCOMMONNUMPAGES][12], Buffer3[MAXCOMMON][MAXCOMMONNUMPAGES][10];
  static TCHAR Buffer4[MAXCOMMON][MAXCOMMONNUMPAGES][12], Buffer5[MAXCOMMON][MAXCOMMONNUMPAGES][12];
  static short maxnlname;
  TCHAR text[LKSIZETEXT];
  short i, j, k, iRaw, wlen, rli=0, curpage, drawn_items_onpage;
  double Value;
  LKColor rcolor;


  static short Column0, Column1, Column2, Column3, Column4, Column5;
  static POINT p1, p2;
  static short rawspace;
  static unsigned short lincr;
  // Printable area for live nearest values
  static short left,right,bottom;
  // one for each mapspace, no matter if 0 and 1 are unused

  // Vertical and horizontal spaces
  #define INTERRAW	1
  #define HEADRAW	NIBLSCALE(6)	
  RECT invsel;

  short curmapspace=MapSpaceMode;

  if (DoInit[MDI_DRAWCOMMON]) {

  // Set screen borders to avoid writing on extreme pixels
  if ( !ScreenLandscape ) {
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
	maxnlname=MAXNLNAME-3; // 9 chars, sized 10 100213
  	_stprintf(Buffer,TEXT("ABCDEFGHMx")); 
  }

  Surface.SelectObject(LK8InfoBigFont); // Text font for Nearest  was LK8Title
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &WPTextSize);

  // Size distance
  _stprintf(Buffer,TEXT("000.0")); 
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &DSTextSize);

  // Bearing
  _stprintf(Buffer,TEXT("<<123")); 
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &BETextSize);

  // reqE
  _stprintf(Buffer,TEXT("5299")); 
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &RETextSize);

  // Altitude Arrival
  _stprintf(Buffer,TEXT("+9999")); 
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &AATextSize);

  Surface.SelectObject(LK8InfoNormalFont);
  _stprintf(Buffer,TEXT("MMMM")); 
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &HLTextSize);

  Surface.SelectObject(LK8PanelMediumFont);
  _stprintf(Buffer,TEXT("4.4"));
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &MITextSize);

  short afterwpname=left+WPTextSize.cx+NIBLSCALE(5);
  short intercolumn=(right-afterwpname- DSTextSize.cx-BETextSize.cx-RETextSize.cx-AATextSize.cx)/3; 

  Column0=MITextSize.cx+LEFTLIMITER+NIBLSCALE(5);
  Column1=left;							// WP align left

  if (ScreenLandscape) {
      Column2=afterwpname+DSTextSize.cx;                // DS align right
      Column3=Column2+intercolumn+BETextSize.cx;        // BE align right
      Column4=Column3+intercolumn+RETextSize.cx;        // RE align right
      Column5=Column4+intercolumn+AATextSize.cx;        // AA align right
  } else {
      Surface.SelectObject(LK8PanelMediumFont);
      Surface.GetTextSize( _T("2.1_APT 3/3_"), 12, &phdrTextSize);
      int s=(rc.right - phdrTextSize.cx) /4;
      Column5= right - NIBLSCALE(2);
      Column4= Column5 - s;
      Column3= Column4 - s;
      Column2= Column3 - s;
  }



  if ( !ScreenLandscape ) {
	lincr=2;
  	TopSize=rc.top+HEADRAW*2+WPTextSize.cy;
  	p1.x=0; p1.y=TopSize; p2.x=rc.right; p2.y=p1.y;
  	TopSize+=HEADRAW;
  	CommonNumraws=(bottom - TopSize) / (WPTextSize.cy+(INTERRAW*2));
  	if (CommonNumraws>MAXCOMMON) CommonNumraws=MAXCOMMON;
  	rawspace=(WPTextSize.cy+INTERRAW);
  } else {
	lincr=1;
  	TopSize=rc.top+HEADRAW*2+HLTextSize.cy;
  	p1.x=0; p1.y=TopSize; p2.x=rc.right; p2.y=p1.y;
  	TopSize+=HEADRAW/2;
  	CommonNumraws=(bottom - TopSize) / (WPTextSize.cy+INTERRAW);
  	if (CommonNumraws>MAXCOMMON) CommonNumraws=MAXCOMMON;
  	rawspace=(WPTextSize.cy+INTERRAW);
  }

#define INTERBOX intercolumn/2

  // Wpname
#ifdef WIN32
  s_sortBox[0].left=0;
  if ( !ScreenLandscape ) s_sortBox[0].right= phdrTextSize.cx;
#else
  s_sortBox[0].left=Column0;
  if ( !ScreenLandscape ) s_sortBox[0].right= phdrTextSize.cx;
#endif

  else s_sortBox[0].right=left+WPTextSize.cx-NIBLSCALE(10);
  s_sortBox[0].top=0;
  s_sortBox[0].bottom=p1.y;
  SortBoxX[MSM_COMMON][0]=s_sortBox[0].right;
  SortBoxX[MSM_RECENT][0]= SortBoxX[MSM_COMMON][0];

  // Distance
  #ifdef WIN32
  if ( !ScreenLandscape ) s_sortBox[1].left=s_sortBox[0].right;
  else s_sortBox[1].left=Column1+afterwpname-INTERBOX-NIBLSCALE(2);
  #else
  s_sortBox[1].left=s_sortBox[0].right;
  #endif
  if (!ScreenLandscape) s_sortBox[1].right=Column2+NIBLSCALE(2);
  else s_sortBox[1].right=Column2+INTERBOX;
  s_sortBox[1].top=0;
  s_sortBox[1].bottom=p1.y;
  SortBoxX[MSM_COMMON][1]=s_sortBox[1].right;
  SortBoxX[MSM_RECENT][1]= SortBoxX[MSM_COMMON][1];

  // Bearing
  if (!ScreenLandscape) {
      s_sortBox[2].left=Column2+NIBLSCALE(2);
      s_sortBox[2].right=Column3+NIBLSCALE(2);;
  } else {
      s_sortBox[2].left=Column2+INTERBOX;
      s_sortBox[2].right=Column3+INTERBOX;
  }
  s_sortBox[2].top=0;
  s_sortBox[2].bottom=p1.y;
  SortBoxX[MSM_COMMON][2]=s_sortBox[2].right;
  SortBoxX[MSM_RECENT][2]= SortBoxX[MSM_COMMON][2];

  // reqE
  if (!ScreenLandscape) {
      s_sortBox[3].left=Column3+NIBLSCALE(2);
      s_sortBox[3].right=Column4+NIBLSCALE(2);
  } else {
      s_sortBox[3].left=Column3+INTERBOX;
      s_sortBox[3].right=Column4+INTERBOX;
  }
  s_sortBox[3].top=0;
  s_sortBox[3].bottom=p1.y;
  SortBoxX[MSM_COMMON][3]=s_sortBox[3].right;
  SortBoxX[MSM_RECENT][3]= SortBoxX[MSM_COMMON][3];

  // AltArr
  if (!ScreenLandscape) {
      s_sortBox[4].left=Column4+NIBLSCALE(2);
      s_sortBox[4].right=rc.right-1;
  } else {
      s_sortBox[4].left=Column4+INTERBOX;
      s_sortBox[4].right=rc.right-1;
  }
  s_sortBox[4].top=0;
  s_sortBox[4].bottom=p1.y;
  SortBoxX[MSM_COMMON][4]=s_sortBox[4].right;
  SortBoxX[MSM_RECENT][4]= SortBoxX[MSM_COMMON][4];

  SortBoxY[MSM_COMMON]=p1.y;
  SortBoxY[MSM_RECENT]=SortBoxY[MSM_COMMON];

  // Caution: could be wrong? no..
  //CommonNumpages=(short)ceil( (float)MAXCOMMON / (float)CommonNumraws );
  CommonNumpages=roundupdivision(MAXCOMMON*lincr, CommonNumraws);
  if (CommonNumpages>MAXCOMMONNUMPAGES) CommonNumpages=MAXCOMMONNUMPAGES; 
  else if (CommonNumpages<1) CommonNumpages=1;

  // set the initial highlighted item to the first, in each MapSpace
  SelectedRaw[MSM_COMMON]=0; 
  SelectedRaw[MSM_RECENT]=0; 
  SelectedPage[MSM_COMMON]=0;
  SelectedPage[MSM_RECENT]=0;

  DoInit[MDI_DRAWCOMMON]=false;
  return;
  } // doinit

  int *pNumber;
  int *pIndex;
  switch(curmapspace) {
	case MSM_COMMON:
			pNumber=&CommonNumber;
			pIndex=CommonIndex;
			break;
	case MSM_RECENT:
	default:
			pNumber=&RecentNumber;
			pIndex=RecentIndex;
			break;
	
  }


  // calculate again real number of pages
  CommonNumpages=roundupdivision(*pNumber * lincr, CommonNumraws);
  if (CommonNumpages>MAXCOMMONNUMPAGES) CommonNumpages=MAXCOMMONNUMPAGES;
  else if (CommonNumpages<1) CommonNumpages=1;
  // current page in use by current mapspacemode
  curpage=SelectedPage[curmapspace];
  if (curpage<0||curpage>=MAXCOMMONNUMPAGES) {
	// DoStatusMessage(_T("ERR-092 current page invalid!")); // selection while waiting for data ready
	// immediate action to resolve this problem, take it back to normality
	SelectedPage[curmapspace]=0;
	LKevent=LKEVENT_NONE;
	return;
  }
  // list changed, and we are now over the real new size> resetting
  if (curpage>=CommonNumpages) curpage=0; 

  // synthetic event handler . Remember to always clear events!
  switch (LKevent) {
	case LKEVENT_NONE:
		break;
	case LKEVENT_ENTER:
		i=pIndex[SelectedRaw[curmapspace] + (curpage*CommonNumraws/lincr)];

		if (!ValidWayPoint(i)) {
			break;
		}
		/*
                 * we can't show dialog from Draw thread
                 * instead, new event is queued, dialog will be popup by main thread 
                 */
                InputEvents::processPopupDetails(InputEvents::PopupWaypoint, i);
		// SetModeType(LKMODE_MAP, MP_MOVING); Experimental OFF 101219
		LKevent=LKEVENT_NONE;
		return;
		break;
	case LKEVENT_DOWN:
		if (++SelectedRaw[curmapspace] >=CommonNumraws) SelectedRaw[curmapspace]=0;
		LastDoCommon=DrawInfo.Time+PAGINGTIMEOUT-1.0; //@ 101003
		// Event to be cleared at the end
		break;
	case LKEVENT_UP:
		if (--SelectedRaw[curmapspace] <0) SelectedRaw[curmapspace]=CommonNumraws-1;
		LastDoCommon=DrawInfo.Time+PAGINGTIMEOUT-1.0; //@ 101003
		break;
	case LKEVENT_PAGEUP:
		LKevent=LKEVENT_NONE;
		break;
	case LKEVENT_PAGEDOWN:
		LKevent=LKEVENT_NONE;
		break;
	case LKEVENT_NEWRUN:
		for (i=0; i<MAXCOMMON; i++) {
			for (k=0; k<MAXCOMMONNUMPAGES; k++) {
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

  // Draw Headline

  if (INVERTCOLORS)
	Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), p1, p2, RGB_GREEN, rc);
  else
	Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), p1, p2, RGB_DARKGREEN, rc);

  Surface.SelectObject(LK8InfoNormalFont); // Heading line

  if ( !ScreenLandscape ) { // portrait mode
	_stprintf(Buffer,TEXT("%d.%d"),ModeIndex,CURTYPE+1);
	Surface.SelectObject(LK8PanelMediumFont);
	LKWriteText(Surface, Buffer, LEFTLIMITER, rc.top+TOPLIMITER , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);
	Surface.SelectObject(LK8InfoNormalFont);

	if (curmapspace == MSM_COMMON )
		// LKTOKEN _@M1309_ "COMN"
  		_stprintf(Buffer,TEXT("%s %d/%d"), gettext(TEXT("_@M1309_")), curpage+1, CommonNumpages); 
	else
		// LKTOKEN _@M1310_ "HIST"
  		_stprintf(Buffer,TEXT("%s %d/%d"), gettext(TEXT("_@M1310_")), curpage+1, CommonNumpages); 
	LKWriteText(Surface, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);


	// LKTOKEN _@M1300_ "Dist"
	 _tcscpy(Buffer,gettext(TEXT("_@M1300_"))); 
	// always sorted manually here!
	LKWriteText(Surface, Buffer, Column2, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	// LKTOKEN _@M1301_ "Dir"
	_tcscpy(Buffer,gettext(TEXT("_@M1301_"))); 
	LKWriteText(Surface, Buffer, Column3, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	// LKTOKEN _@M1302_ "rEff"
	_tcscpy(Buffer,gettext(TEXT("_@M1302_"))); 
	LKWriteText(Surface, Buffer, Column4, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	// LKTOKEN _@M1303_ "AltA"
	_tcscpy(Buffer,gettext(TEXT("_@M1303_"))); 
	LKWriteText(Surface, Buffer, Column5, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);


  } else {

	_stprintf(Buffer,TEXT("%d.%d"),ModeIndex,CURTYPE+1);
	Surface.SelectObject(LK8PanelMediumFont);
	LKWriteText(Surface, Buffer, LEFTLIMITER, rc.top+TOPLIMITER , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);
	Surface.SelectObject(LK8InfoNormalFont);



        bool compact= (ScreenGeometry==SCREEN_GEOMETRY_43);

	if (curmapspace == MSM_COMMON )
		// LKTOKEN _@M1309_ "COMN"
		_stprintf(Buffer,TEXT("%s %d/%d"), gettext(TEXT("_@M1309_")), curpage+1,CommonNumpages); 
	else
		// LKTOKEN _@M1310_ "HIST"
		_stprintf(Buffer,TEXT("%s %d/%d"), gettext(TEXT("_@M1310_")), curpage+1,CommonNumpages); 
	LKWriteText(Surface, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);

	// LKTOKEN _@M1300_ "Dist"
	// LKTOKEN _@M1304_ "Distance"
	_tcscpy(Buffer,gettext(compact?TEXT("_@M1300_"):TEXT("_@M1304_"))); 
	LKWriteText(Surface, Buffer, Column2, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	// LKTOKEN _@M1301_ "Dir"
	// LKTOKEN _@M1305_ "Direction"
	_tcscpy(Buffer,gettext(compact?TEXT("_@M1301_"):TEXT("_@M1305_"))); 
	LKWriteText(Surface, Buffer, Column3, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	// LKTOKEN _@M1302_ "rEff"
	// LKTOKEN _@M1306_ "ReqEff"
	_tcscpy(Buffer,gettext(compact?TEXT("_@M1302_"):TEXT("_@M1306_"))); 
	LKWriteText(Surface, Buffer, Column4, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	// LKTOKEN _@M1308_ "Arriv"
	// LKTOKEN _@M1307_ "AltArr"
	_tcscpy(Buffer,gettext(compact?TEXT("_@M1308_"):TEXT("_@M1307_"))); 
	LKWriteText(Surface, Buffer, Column5, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);
	

  } // landscape mode


  Surface.SelectObject(LK8InfoBigFont); // Text font for Nearest

  // try to reduce conflicts, as task thread could change it while we are using it here.
  // so we copy it and clear it here once forever in this run
  bool ndr;
  switch (curmapspace) {
	case MSM_COMMON:
  		ndr=CommonDataReady;
  		CommonDataReady=false;
		break;
	case MSM_RECENT:
  		ndr=RecentDataReady;
  		RecentDataReady=false;
		break;
	default:
		ndr=false;
		break;
   }

  // numraws always <= MAXNEAREST 
  for (i=0, j=0, drawn_items_onpage=0; i<CommonNumraws; j++, i+=lincr) {
	iRaw=TopSize+(rawspace*i);
	short curraw=(curpage*CommonNumraws);
        if (!ScreenLandscape) curraw/=2;
        curraw+=j;
	if (curraw>=MAXCOMMON) break;
	rli=pIndex[curraw];


	if (!ndr) goto KeepOldValues;
	if ( ValidWayPoint(rli) ) {

		wlen=_tcslen(WayPointList[rli].Name);
                if (!ScreenLandscape) {
                        LK_tcsncpy(Buffer, WayPointList[rli].Name, 12);
                } else {
                        if (wlen>maxnlname) {
                                LK_tcsncpy(Buffer, WayPointList[rli].Name, maxnlname);
                        }
                        else {
                                LK_tcsncpy(Buffer, WayPointList[rli].Name, wlen);
                        }
                }
		CharUpper(Buffer); // 100213 FIX UPPERCASE DRAWNEAREST
		_tcscpy(Buffer1[i][curpage],Buffer); 

		// Distance
		Value=WayPointCalc[rli].Distance*DISTANCEMODIFY;
                if (!ScreenLandscape) 
                    _stprintf(Buffer2[i][curpage],TEXT("%0.1lf %s"),Value,Units::GetDistanceName());
                else
                    _stprintf(Buffer2[i][curpage],TEXT("%0.1lf"),Value);

		// relative bearing

		if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
			Value = WayPointCalc[rli].Bearing -  DrawInfo.TrackBearing;

			if (Value < -180.0)
				Value += 360.0;
			else
				if (Value > 180.0)
					Value -= 360.0;

			if (Value > 1)
				_stprintf(Buffer3[i][curpage], TEXT("%2.0f%s%s"), Value, gettext(_T("_@M2179_")), gettext(_T("_@M2183_")));
			else if (Value < -1)
				_stprintf(Buffer3[i][curpage], TEXT("%s%2.0f%s"), gettext(_T("_@M2182_")), -Value, gettext(_T("_@M2179_")));
			else
				_stprintf(Buffer3[i][curpage], TEXT("%s%s"), gettext(_T("_@M2182_")), gettext(_T("_@M2183_")));
		} else
			_stprintf(Buffer3[i][curpage], TEXT("%2.0f%s"), WayPointCalc[rli].Bearing, gettext(_T("_@M2179_")));

		// Requested GR
		Value=WayPointCalc[rli].GR;
		if (Value<1 || Value>=MAXEFFICIENCYSHOW) 
			_stprintf(Buffer4[i][curpage],_T("---"));
		else {
			if (Value>99) _stprintf(text,_T("%.0f"),Value);
			else _stprintf(text,_T("%.1f"),Value);
			_stprintf(Buffer4[i][curpage],_T("%s"),text);
		}

		// arrival altitude
		Value=ALTITUDEMODIFY*WayPointCalc[rli].AltArriv[AltArrivMode];
		if (Value <-9999 ||  Value >9999 )
			_tcscpy(text,_T("---"));
		else
			_stprintf(text,_T("%+.0f"),Value);
                if (!ScreenLandscape) _stprintf(Buffer5[i][curpage], TEXT("%s %s"),text,Units::GetAltitudeName() );
                else _stprintf(Buffer5[i][curpage], TEXT("%s"),text);

	} else {
		// Invalid waypoint, fill in all empty data and maybe break loop
		_stprintf(Buffer1[i][curpage],_T("------------"));
		_stprintf(Buffer2[i][curpage],_T("---"));
		_stprintf(Buffer3[i][curpage],_T("---"));
		_stprintf(Buffer4[i][curpage],_T("---"));
		_stprintf(Buffer5[i][curpage],_T("---"));
	}


KeepOldValues:

	if ( ValidWayPoint(rli) ) {
  		Surface.SelectObject(LK8InfoBigFont); // Text font for Nearest
		drawn_items_onpage++;
		// Common turnpoints are mixed, so we must be sure that reachable is checked only against a landable
		if ((WayPointCalc[rli].VGR == 3 )|| ( WayPointCalc[rli].IsLandable && !WayPointList[rli].Reachable)) // 091205
			rcolor=RGB_LIGHTRED;
		else
			rcolor=RGB_WHITE;
	} else 
			rcolor=RGB_GREY;
			//TextDisplayMode.AsFlag.Color = TEXTGREY;

	LKWriteText(Surface, Buffer1[i][curpage], Column1, iRaw , 0, WTMODE_NORMAL, WTALIGN_LEFT, rcolor, false);
	
	// set again correct font
  	Surface.SelectObject(LK8InfoBigFont); // Text font for Nearest

        if (ScreenLandscape) {
            LKWriteText(Surface,  Buffer1[i][curpage], Column1, iRaw , 0, WTMODE_NORMAL, WTALIGN_LEFT, rcolor, false);
            LKWriteText(Surface,  Buffer2[i][curpage], Column2, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
            LKWriteText(Surface,  Buffer3[i][curpage], Column3, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
            LKWriteText(Surface,  Buffer4[i][curpage], Column4, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
            LKWriteText(Surface,  Buffer5[i][curpage], Column5, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
        } else {
            LKWriteText(Surface,  Buffer1[i][curpage], Column1, iRaw , 0, WTMODE_NORMAL, WTALIGN_LEFT, rcolor, false);
            LKWriteText(Surface,  Buffer2[i][curpage], Column5, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
            iRaw+=rawspace;
            unsigned int iCol=ScreenSizeX/3;
            LKWriteText(Surface,  Buffer3[i][curpage], iCol, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
            LKWriteText(Surface,  Buffer4[i][curpage], iCol*2, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
            LKWriteText(Surface,  Buffer5[i][curpage], right-IBLSCALE(2), iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
        }




  }  // for


  // clear flag, and don't outbox
  if (LKevent==LKEVENT_NEWRUN || LKevent==LKEVENT_NEWPAGE ) {
		LKevent=LKEVENT_NONE;
		return;
  }

  // BOXOUT SELECTED ITEM
    if (drawn_items_onpage>0) { 

	if (SelectedRaw[curmapspace] <0 || SelectedRaw[curmapspace]>(CommonNumraws-1)) {
		LKevent=LKEVENT_NONE;
		return;
	}
	// avoid boxing and selecting nonexistent items
	// selectedraw starts from 0, drawnitems from 1...
	// In this case we set the first one, or last one, assuming we are rotating forward or backward
	if (SelectedRaw[curmapspace] >= drawn_items_onpage) {
		if (LKevent==LKEVENT_DOWN) SelectedRaw[curmapspace]=0;
		else 
		// up from top to bottom, bottom empty, look for the last valid one (ie first going back from bottom)
		if (LKevent==LKEVENT_UP) SelectedRaw[curmapspace]=drawn_items_onpage-1;
		else {
			// DoStatusMessage(_T("Cant find valid raw")); // no more needed
			SelectedRaw[curmapspace]=0;
		}
	}
	invsel.left=left;
	invsel.right=right;
        if (!ScreenLandscape) invsel.top=TopSize+(rawspace*SelectedRaw[curmapspace]*lincr);
        else invsel.top=TopSize+(rawspace*SelectedRaw[curmapspace]*lincr)+NIBLSCALE(2);
        invsel.bottom=TopSize+(rawspace*(SelectedRaw[curmapspace]*lincr+1))-NIBLSCALE(1);
        if (!ScreenLandscape) invsel.bottom+=rawspace;

	Surface.InvertRect(invsel);
  } 

  LKevent=LKEVENT_NONE;
  return;
}


