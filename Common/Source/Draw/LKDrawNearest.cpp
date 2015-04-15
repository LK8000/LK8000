/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKDrawNearest.cpp,v 1.2 2010/12/24 12:45:49 root Exp root $
*/

#include "externs.h"
#include "LKMapWindow.h"
#include "LKObjects.h"
#include "RGB.h"
#include "DoInits.h"
#include "InputEvents.h"
#include "ScreenGeometry.h"

extern bool CheckLandableReachableTerrainNew(NMEA_INFO *Basic, DERIVED_INFO *Calculated, double LegToGo, double LegBearing);

void MapWindow::DrawNearest(LKSurface& Surface, const RECT& rc) {


  SIZE WPTextSize, DSTextSize, BETextSize, RETextSize, AATextSize, HLTextSize, MITextSize;
  TCHAR Buffer[LKSIZEBUFFERLARGE];
  static RECT s_sortBox[6]; 
  static TCHAR Buffer1[MAXNEAREST][MAXNUMPAGES][24], Buffer2[MAXNEAREST][MAXNUMPAGES][10], Buffer3[MAXNEAREST][MAXNUMPAGES][10];
  static TCHAR Buffer4[MAXNEAREST][MAXNUMPAGES][12], Buffer5[MAXNEAREST][MAXNUMPAGES][12];
  static short s_maxnlname;
  TCHAR text[30];
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

  // we lock to current mapspace for this drawing
  short curmapspace=MapSpaceMode; 

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

  if (DoInit[MDI_DRAWNEAREST]) {

  // Set screen borders to avoid writing on extreme pixels
  if ( !ScreenLandscape ) {
	// Portrait mode has tight horizontal margins...
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
  Surface.GetTextSize( Buffer, _tcslen(Buffer), &WPTextSize);

  _stprintf(Buffer,TEXT("000.0")); 
  Surface.GetTextSize( Buffer, _tcslen(Buffer), &DSTextSize);

  //: Bearing
  _stprintf(Buffer,TEXT("<<123")); 
  Surface.GetTextSize( Buffer, _tcslen(Buffer), &BETextSize);

  //: reqE
  _stprintf(Buffer,TEXT("5299")); 
  Surface.GetTextSize( Buffer, _tcslen(Buffer), &RETextSize);

  //: Altitude Arrival
  _stprintf(Buffer,TEXT("+9999")); 
  Surface.GetTextSize( Buffer, _tcslen(Buffer), &AATextSize);

  Surface.SelectObject(LK8InfoNormalFont);
  _stprintf(Buffer,TEXT("MMMM")); 
  Surface.GetTextSize( Buffer, _tcslen(Buffer), &HLTextSize);

  Surface.SelectObject(LK8PanelMediumFont);
  _stprintf(Buffer,TEXT("4.4")); 
  Surface.GetTextSize( Buffer, _tcslen(Buffer), &MITextSize);

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


  if ( !ScreenLandscape ) {
  	TopSize=rc.top+HEADRAW*2+HLTextSize.cy;
  	p1.x=0; p1.y=TopSize; p2.x=rc.right; p2.y=p1.y;
  	//TopSize+=(WPTextSize.cy);
  	TopSize+=HEADRAW;
  	Numraws=(bottom - TopSize) / (WPTextSize.cy+(INTERRAW*2));
  	if (Numraws>MAXNEAREST) Numraws=MAXNEAREST;
  	s_rawspace=(WPTextSize.cy+INTERRAW);
  } else {
  	TopSize=rc.top+HEADRAW*2+HLTextSize.cy;
  	p1.x=0; p1.y=TopSize; p2.x=rc.right; p2.y=p1.y;
  	TopSize+=HEADRAW/2;
  	Numraws=(bottom - TopSize) / (WPTextSize.cy+INTERRAW);
  	if (Numraws>MAXNEAREST) Numraws=MAXNEAREST;
  	s_rawspace=(WPTextSize.cy+INTERRAW);
  }

#define INTERBOX intercolumn/2

  s_sortBox[0].left=Column0; 
#ifdef WIN32
  if ( !ScreenLandscape ) s_sortBox[0].right=left+WPTextSize.cx-NIBLSCALE(2);
#else
  if ( !ScreenLandscape ) s_sortBox[0].right=left+WPTextSize.cx+NIBLSCALE(3);
#endif
  else s_sortBox[0].right=left+WPTextSize.cx-NIBLSCALE(10);
  s_sortBox[0].top=2;
  s_sortBox[0].bottom=p1.y;
  SortBoxX[MSM_LANDABLE][0]=s_sortBox[0].right;
  SortBoxX[MSM_AIRPORTS][0]=s_sortBox[0].right;
  SortBoxX[MSM_NEARTPS][0]=s_sortBox[0].right;

  #ifdef WIN32
  if ( !ScreenLandscape ) s_sortBox[1].left=Column1+afterwpname-INTERBOX;
  else s_sortBox[1].left=Column1+afterwpname-INTERBOX-NIBLSCALE(2);
  #else
  s_sortBox[1].left=s_sortBox[0].right;
  #endif
  s_sortBox[1].right=Column2+INTERBOX;
  s_sortBox[1].top=2;
  s_sortBox[1].bottom=p1.y;
  SortBoxX[MSM_LANDABLE][1]=s_sortBox[1].right;
  SortBoxX[MSM_AIRPORTS][1]=s_sortBox[1].right;
  SortBoxX[MSM_NEARTPS][1]=s_sortBox[1].right;

  s_sortBox[2].left=Column2+INTERBOX;
  s_sortBox[2].right=Column3+INTERBOX;
  s_sortBox[2].top=2;
  s_sortBox[2].bottom=p1.y;
  SortBoxX[MSM_LANDABLE][2]=s_sortBox[2].right;
  SortBoxX[MSM_AIRPORTS][2]=s_sortBox[2].right;
  SortBoxX[MSM_NEARTPS][2]=s_sortBox[2].right;

  s_sortBox[3].left=Column3+INTERBOX;
  s_sortBox[3].right=Column4+INTERBOX;
  s_sortBox[3].top=2;
  s_sortBox[3].bottom=p1.y;
  SortBoxX[MSM_LANDABLE][3]=s_sortBox[3].right;
  SortBoxX[MSM_AIRPORTS][3]=s_sortBox[3].right;
  SortBoxX[MSM_NEARTPS][3]=s_sortBox[3].right;

  s_sortBox[4].left=Column4+INTERBOX;
  s_sortBox[4].right=rc.right-1;
  s_sortBox[4].top=2;
  s_sortBox[4].bottom=p1.y;
  SortBoxX[MSM_LANDABLE][4]=s_sortBox[4].right;
  SortBoxX[MSM_AIRPORTS][4]=s_sortBox[4].right;
  SortBoxX[MSM_NEARTPS][4]=s_sortBox[4].right;

  SortBoxY[MSM_LANDABLE]=p1.y;
  SortBoxY[MSM_AIRPORTS]=p1.y;
  SortBoxY[MSM_NEARTPS]=p1.y;

  Numpages=roundupdivision(MAXNEAREST, Numraws);
  if (Numpages>MAXNUMPAGES) Numpages=MAXNUMPAGES;
  else if (Numpages<1) Numpages=1;

  SelectedRaw[MSM_LANDABLE]=0; SelectedRaw[MSM_AIRPORTS]=0; SelectedRaw[MSM_NEARTPS]=0;
  SelectedPage[MSM_LANDABLE]=0; SelectedPage[MSM_AIRPORTS]=0; SelectedPage[MSM_NEARTPS]=0;

  DoInit[MDI_DRAWNEAREST]=false;
  return;
  } // doinit

  Numpages=roundupdivision(SortedNumber, Numraws);
  if (Numpages>MAXNUMPAGES) Numpages=MAXNUMPAGES;
  else if (Numpages<1) Numpages=1;

  curpage=SelectedPage[curmapspace];
  if (curpage<0||curpage>=MAXNUMPAGES) { // TODO also >Numpages
	// DoStatusMessage(_T("ERR-091 curpage invalid!"));  // selection while waiting for data ready
	SelectedPage[curmapspace]=0;
	LKevent=LKEVENT_NONE;
	return;
  }

  switch (LKevent) {
	case LKEVENT_NONE:
		break;
	case LKEVENT_ENTER:
		LKevent=LKEVENT_NONE;
		switch(curmapspace) {
			case MSM_LANDABLE:
				i=SortedLandableIndex[SelectedRaw[curmapspace]+(curpage*Numraws)];
				break;
			case MSM_AIRPORTS:
				i=SortedAirportIndex[SelectedRaw[curmapspace] + (curpage*Numraws)];
				break;
			case MSM_NEARTPS:
			default:
				i=SortedTurnpointIndex[SelectedRaw[curmapspace] + (curpage*Numraws)];
				break;
		}

		if ( !ValidWayPoint(i)) {
			#if 0 // selection while waiting for data ready
			if (SortedNumber>0)
				DoStatusMessage(_T("ERR-019 Invalid selection")); 
			#endif
			break;
		}
        /*
         * we can't show dialog from Draw thread
         * instead, new event is queued, dialog will be popup by main thread 
         */
        InputEvents::processPopupDetails(InputEvents::PopupWaypoint, i);
		// SetModeType(LKMODE_MAP,MP_MOVING); EXperimental OFF 101219
		LKevent=LKEVENT_NONE; 
		return;
		break;
	case LKEVENT_DOWN:
		if (++SelectedRaw[curmapspace] >=Numraws) SelectedRaw[curmapspace]=0;
		LastDoNearest=DrawInfo.Time+PAGINGTIMEOUT-1.0; 
		break;
	case LKEVENT_UP:
		if (--SelectedRaw[curmapspace] <0) SelectedRaw[curmapspace]=Numraws-1;
		LastDoNearest=DrawInfo.Time+PAGINGTIMEOUT-1.0; 
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
	  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), p1, p2, RGB_GREEN, rc);
  else
	  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), p1, p2, RGB_DARKGREEN, rc);

  Surface.SelectObject(LK8InfoNormalFont); // Heading line

  short cursortbox=SortedMode[curmapspace];

  if ( !ScreenLandscape ) { // portrait mode
	Surface.FillRect(&s_sortBox[cursortbox], sortbrush);

	_stprintf(Buffer,TEXT("%d.%d"),ModeIndex,CURTYPE+1);
  	Surface.SelectObject(LK8PanelMediumFont);

	LKWriteText(Surface,  Buffer, LEFTLIMITER, rc.top+TOPLIMITER , 0,  WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);
  	Surface.SelectObject(LK8InfoNormalFont);

	switch(curmapspace) {
		case MSM_LANDABLE:
			// LKTOKEN _@M1311_ "LND"
 			_stprintf(Buffer,TEXT("%s %d/%d"), gettext(TEXT("_@M1311_")), curpage+1,Numpages); 
			break;
		case MSM_AIRPORTS:
 	 		// LKTOKEN _@M1313_ "APT"
			_stprintf(Buffer,TEXT("%s %d/%d"), gettext(TEXT("_@M1313_")), curpage+1, Numpages); 
			break;
		case MSM_NEARTPS:
		default:
			// LKTOKEN _@M1315_ "TPS"
 	 		_stprintf(Buffer,TEXT("%s %d/%d"), gettext(TEXT("_@M1315_")), curpage+1, Numpages); 
			break;
	}


	if (cursortbox == 0)
		LKWriteText(Surface,  Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_BLACK, false);
	else
		LKWriteText(Surface,  Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);

	// LKTOKEN _@M1300_ "Dist"
	_tcscpy(Buffer,gettext(TEXT("_@M1300_"))); 
	if (cursortbox==1)
		LKWriteText(Surface,  Buffer, Column2, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(Surface,  Buffer, Column2, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	// LKTOKEN _@M1301_ "Dir"
	_tcscpy(Buffer,gettext(TEXT("_@M1301_"))); 
	if (cursortbox==2)
		LKWriteText(Surface,  Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(Surface,  Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	// LKTOKEN _@M1302_ "rEff"
	_tcscpy(Buffer,gettext(TEXT("_@M1302_"))); 
	if (cursortbox==3)
		LKWriteText(Surface,  Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(Surface,  Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	// LKTOKEN _@M1303_ "AltA"
	_tcscpy(Buffer,gettext(TEXT("_@M1303_"))); 
	if (cursortbox==4)
		LKWriteText(Surface,  Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(Surface,  Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);


  } else {
	Surface.FillRect(&s_sortBox[cursortbox], sortbrush);

	bool compact= (ScreenGeometry==SCREEN_GEOMETRY_43);

	_stprintf(Buffer,TEXT("%d.%d"),ModeIndex,CURTYPE+1);
  	Surface.SelectObject(LK8PanelMediumFont);
	LKWriteText(Surface,  Buffer, LEFTLIMITER, rc.top+TOPLIMITER , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);
  	Surface.SelectObject(LK8InfoNormalFont);

	switch(curmapspace) {
		case MSM_LANDABLE:
			// LKTOKEN _@M1312_ "LNDB"
			_stprintf(Buffer,TEXT("%s %d/%d"), gettext(TEXT("_@M1312_")), curpage+1,Numpages); 
			break;
		case MSM_AIRPORTS:
			// LKTOKEN _@M1314_ "APTS"
			_stprintf(Buffer,TEXT("%s %d/%d"), gettext(TEXT("_@M1314_")), curpage+1, Numpages); 
			break;
		case MSM_NEARTPS:
		default:
			// LKTOKEN _@M1315_ "TPS"
			_stprintf(Buffer,TEXT("%s %d/%d"), gettext(TEXT("_@M1315_")), curpage+1, Numpages); 
			break;
	}
	if (cursortbox==0)
		LKWriteText(Surface,  Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0,WTMODE_NORMAL, WTALIGN_LEFT, RGB_BLACK, false);
	else
		LKWriteText(Surface,  Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0,WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);


		// LKTOKEN _@M1300_ "Dist"
		// LKTOKEN _@M1304_ "Distance"
                _tcscpy(Buffer,gettext(compact?TEXT("_@M1300_"):TEXT("_@M1304_")));
		if (cursortbox==1)
			LKWriteText(Surface,  Buffer, Column2, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(Surface,  Buffer, Column2, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

		// LKTOKEN _@M1301_ "Dir"
		// LKTOKEN _@M1305_ "Direction"
                _tcscpy(Buffer,gettext(compact?TEXT("_@M1301_"):TEXT("_@M1305_")));
		if (cursortbox==2)
			LKWriteText(Surface,  Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(Surface,  Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

		// LKTOKEN _@M1302_ "rEff"
		// LKTOKEN _@M1306_ "ReqEff"
                _tcscpy(Buffer,gettext(compact?TEXT("_@M1302_"):TEXT("_@M1306_")));
		if (cursortbox==3)
			LKWriteText(Surface,  Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(Surface,  Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

		// LKTOKEN _@M1308_ "Arriv"
		// LKTOKEN _@M1307_ "AltArr"
                _tcscpy(Buffer,gettext(compact?TEXT("_@M1308_"):TEXT("_@M1307_")));
		if (cursortbox==4)
			LKWriteText(Surface,  Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(Surface,  Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

  } // landscape mode


  Surface.SelectObject(LK8InfoBigFont); // Text font for Nearest

  bool ndr=NearestDataReady;
  NearestDataReady=false;

  int *psortedindex;
  switch(curmapspace) {
	case MSM_LANDABLE:
		psortedindex=SortedLandableIndex;
		break;
	case MSM_AIRPORTS:
		psortedindex=SortedAirportIndex;
		break;
	case MSM_NEARTPS:
	default:
		psortedindex=SortedTurnpointIndex;
		break;
  }
	
  for (i=0, drawn_items_onpage=0; i<Numraws; i++) {
	iRaw=TopSize+(s_rawspace*i);
	short curraw=(curpage*Numraws)+i;
	if (curraw>=MAXNEAREST) break;

	rli=*(psortedindex+curraw);

	if (!ndr) {
		goto KeepOldValues;
	}
	if ( ValidWayPoint(rli) ) {

		wlen=_tcslen(WayPointList[rli].Name);
		if (wlen>s_maxnlname) {
			LK_tcsncpy(Buffer, WayPointList[rli].Name, s_maxnlname);
		}
		else {
			LK_tcsncpy(Buffer, WayPointList[rli].Name, wlen);
		}
		CharUpper(Buffer);
		_tcscpy(Buffer1[i][curpage],Buffer); 

		value=WayPointCalc[rli].Distance*DISTANCEMODIFY;
         	_stprintf(Buffer2[i][curpage],TEXT("%0.1lf"),value);


		if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
			value = WayPointCalc[rli].Bearing -  DrawInfo.TrackBearing;

			if (value < -180.0)
				value += 360.0;
			else
				if (value > 180.0)
					value -= 360.0;

			if (value > 1)
				_stprintf(Buffer3[i][curpage], TEXT("%2.0f%s%s"), value, gettext(_T("_@M2179_")),gettext(_T("_@M2183_")));
			else
				if (value < -1)
					_stprintf(Buffer3[i][curpage], TEXT("%s%2.0f%s"), gettext(_T("_@M2182_")), -value, gettext(_T("_@M2179_")));
				else
					_stprintf(Buffer3[i][curpage], TEXT("%s%s"), gettext(_T("_@M2182_")),gettext(_T("_@M2183_")));
		} else
			_stprintf(Buffer3[i][curpage], TEXT("%2.0f%s"), WayPointCalc[rli].Bearing, gettext(_T("_@M2179_"))); // 101219

		value=WayPointCalc[rli].GR;
		if (value<1 || value>=MAXEFFICIENCYSHOW) 
			_stprintf(Buffer4[i][curpage],_T("---"));
		else {
			if (value>99) _stprintf(text,_T("%.0f"),value);
			else _stprintf(text,_T("%.1f"),value);
			_stprintf(Buffer4[i][curpage],_T("%s"),text);
		}

		value=ALTITUDEMODIFY*WayPointCalc[rli].AltArriv[AltArrivMode];
		if (value <-9999 ||  value >9999 )
			_tcscpy(text,_T("---"));
		else
			_stprintf(text,_T("%+.0f"),value);
		_stprintf(Buffer5[i][curpage], TEXT("%s"),text);

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

		if (WayPointCalc[rli].IsOutlanding) {
			rcolor=RGB_LIGHTYELLOW;
  			Surface.SelectObject(LK8InfoBigItalicFont);
		} else {
			rcolor=RGB_WHITE;
  			Surface.SelectObject(LK8InfoBigFont);
		}

		// 120601 extend search for tps, missing reachable status
		// If we are listing tps, and the current tp has a positive arrival altitude,
		// then check if it is really unreachable because we dont calculate tps for that.
		// Unless they are in a task, common, alternates, of course.
		if (curmapspace==MSM_NEARTPS) {
			if ( WayPointCalc[rli].AltArriv[AltArrivMode]>0) {
				if (CheckLandableReachableTerrainNew(&DrawInfo, &DerivedDrawInfo, 
					WayPointCalc[rli].Distance, WayPointCalc[rli].Bearing )) {
						rcolor=RGB_WHITE;
				} else {
					rcolor=RGB_LIGHTRED;
				}
			} else {
				rcolor=RGB_LIGHTRED;
			}
		} else  // old stuff as usual
		if ((WayPointCalc[rli].VGR == 3 )|| (!WayPointList[rli].Reachable)) {

			rcolor=RGB_LIGHTRED;
		}


	} else {
		rcolor=RGB_GREY;
	}

	LKWriteText(Surface,  Buffer1[i][curpage], Column1, iRaw , 0, WTMODE_NORMAL, WTALIGN_LEFT, rcolor, false);
	
  	Surface.SelectObject(LK8InfoBigFont); // Text font for Nearest
	LKWriteText(Surface,  Buffer2[i][curpage], Column2, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

	LKWriteText(Surface,  Buffer3[i][curpage], Column3, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

	LKWriteText(Surface,  Buffer4[i][curpage], Column4, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

	LKWriteText(Surface,  Buffer5[i][curpage], Column5, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

  } 


  if (LKevent==LKEVENT_NEWRUN || LKevent==LKEVENT_NEWPAGE ) {
		LKevent=LKEVENT_NONE;
		return;
  }

  if (drawn_items_onpage>0) {

	if (SelectedRaw[curmapspace] <0 || SelectedRaw[curmapspace]>(Numraws-1)) {
		return;
	}
	if (SelectedRaw[curmapspace] >= drawn_items_onpage) {
		if (LKevent==LKEVENT_DOWN) SelectedRaw[curmapspace]=0;
		else 
		if (LKevent==LKEVENT_UP) SelectedRaw[curmapspace]=drawn_items_onpage-1;
		else {
			// Here we are recovering a selection problem caused by a delay while switching.
			// DoStatusMessage(_T("Cant find valid raw")); // not needed anymore
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

