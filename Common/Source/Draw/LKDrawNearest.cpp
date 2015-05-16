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


  static TCHAR Buffer1[MAXNEAREST][MAXNUMPAGES][24];
  static TCHAR Buffer2[MAXNEAREST][MAXNUMPAGES][12];
  static TCHAR Buffer3[MAXNEAREST][MAXNUMPAGES][12];
  static TCHAR Buffer4[MAXNEAREST][MAXNUMPAGES][12];
  static TCHAR Buffer5[MAXNEAREST][MAXNUMPAGES][12];

  static unsigned short Column0, Column1, Column2, Column3, Column4, Column5;
  static unsigned short hColumn2, hColumn3, hColumn4, hColumn5;
  static RECT s_sortBox[6]; 
  static POINT p1, p2;
  static unsigned short s_rawspace, s_maxnlname, lincr, left, right, bottom;
  static FontReference bigFont, bigItalicFont;
  static bool usetwolines=0;

  // Vertical and horizontal spaces
  #define INTERRAW	1
  #define HEADRAW	NIBLSCALE(6)	

  RECT invsel;
  TCHAR Buffer[LKSIZEBUFFERLARGE];
  TCHAR text[30];
  short i, j, k, iRaw, rli=0, curpage, drawn_items_onpage;
  double value;
  LKColor rcolor;
  short curmapspace=MapSpaceMode; 


  if (DoInit[MDI_DRAWNEAREST]) {

  SIZE InfoTextSize, InfoNumberSize;
  SIZE K1TextSize, K2TextSize, K3TextSize, K4TextSize;
  SIZE phdrTextSize;

  usetwolines=(ScreenLandscape?false:UseTwoLines);

  bigFont=(usetwolines?LK8InfoBig2LFont:LK8InfoBigFont);

  bigItalicFont=(usetwolines?LK8InfoBigItalic2LFont:LK8InfoBigItalicFont);

  // Set screen borders to avoid writing on extreme pixels
  if ( !ScreenLandscape ) {
	left=rc.left+NIBLSCALE(1);
	right=rc.right-NIBLSCALE(1);
  	bottom=rc.bottom-BottomSize-NIBLSCALE(2);
  } else {
	left=rc.left+NIBLSCALE(5);
	right=rc.right-NIBLSCALE(5);
  	bottom=rc.bottom-BottomSize;
  }


  Surface.SelectObject(bigFont); 

  // We want an average size of an alphabet letter and number
  Surface.GetTextSize( _T("ALPHAROMEO"),10, &InfoTextSize); 
  InfoTextSize.cx /= 10;

  Surface.GetTextSize( _T("0123456789"), 10, &InfoNumberSize);
  InfoNumberSize.cx /= 10;


  _stprintf(Buffer,TEXT("555.5")); 
  if (usetwolines) {
      _tcscat(Buffer,_T(" "));
      _tcscat(Buffer,Units::GetDistanceName());
  }
  Surface.GetTextSize( Buffer, _tcslen(Buffer), &K1TextSize);

  _stprintf(Buffer,TEXT("<325")); 
  Surface.GetTextSize( Buffer, _tcslen(Buffer), &K2TextSize);

  _stprintf(Buffer,TEXT("555")); 
  Surface.GetTextSize( Buffer, _tcslen(Buffer), &K3TextSize);

  if (Units::GetUserAltitudeUnit() == unFeet)
      _stprintf(Buffer,TEXT("-99999"));
  else
      _stprintf(Buffer,TEXT("-9999"));
  if (usetwolines) {
      _tcscat(Buffer,_T(" "));
      _tcscat(Buffer,Units::GetAltitudeName());
  }
  Surface.GetTextSize( Buffer, _tcslen(Buffer), &K4TextSize);

  Surface.SelectObject(LK8PanelMediumFont);
  _stprintf(Buffer,TEXT("4.4")); 
  Surface.GetTextSize( Buffer, _tcslen(Buffer), &phdrTextSize);

  // Col0 is where APTS 1/3 can be written, after ModeIndex:Curtype
  Column0= phdrTextSize.cx+LEFTLIMITER+NIBLSCALE(5);
  Column1= left;
  Column5= right;

  if (usetwolines) {
      Column4= right;
      Column3= Column4 - K4TextSize.cx - InfoNumberSize.cx;
      Column2= Column3 - K3TextSize.cx - InfoNumberSize.cx;
      int spare = (Column2 - K2TextSize.cx -InfoNumberSize.cx - left)/3;
      if (spare>0) {
          Column4 -= spare;
          Column3 -= (spare*2);
          Column2 -= (spare*3);
      } 
      s_maxnlname = MAXNLNAME; // 12
      lincr=2;
  } else {
      Column5= right;
      Column4= Column5 - K4TextSize.cx - InfoNumberSize.cx;
      Column3= Column4 - K3TextSize.cx - InfoNumberSize.cx;
      Column2= Column3 - K2TextSize.cx - InfoNumberSize.cx;

      int spare= (Column2 - K1TextSize.cx - left - InfoNumberSize.cx); 
      #if TESTBENCH
      if (spare<1) StartupStore(_T("... NEAREST: spare invalid, =%d%s"),spare,NEWLINE);
      #endif
      if (spare<0) spare=0; // no problem, the user chose a font too big
      int charspace= ceil(spare/(float)InfoTextSize.cx);
      unsigned short charname=0;

      short minchars=ScreenLandscape?7:3;
      if (charspace<=minchars) {
          //  0 or 1 chars do not make a name. We force 2 and good luck with visibility.
          if (charspace<2) {
              #if TESTBENCH
              StartupStore(_T("... NEAREST: not enough space=%d for item names%s"), charspace,NEWLINE);
              #endif
              charname=2;
          } else {
              charname=charspace;
          }
      } else {
          charspace-=minchars; charname=minchars;
          short ratio=2;
          while (charspace>ratio) {
              charspace-=(ratio+1);
              charname++;
              if (charname==9)ratio=3;
              if (charname==11)ratio=4;
          }
      }
      s_maxnlname = charname>MAXNLNAME?MAXNLNAME:charname;
      int advance = spare- (charname*InfoTextSize.cx); 
      if (advance>3) {
          float fadv=advance/4.0;
          Column4 -= ceil(fadv);
          Column3 -= ceil(fadv*2);
          Column2 -= ceil(fadv*3); 
      } 
      lincr=1;
  }

  // Vertical alignement

  Surface.SelectObject(LK8InfoNearestFont);
  Surface.GetTextSize( _T("M"), 1, &phdrTextSize);

  TopSize=rc.top+HEADRAW*2+phdrTextSize.cy;
  p1.x=0; p1.y=TopSize; p2.x=rc.right; p2.y=p1.y;
  if ( !ScreenLandscape ) {
  	TopSize+=HEADRAW;
  	Numraws=(bottom - TopSize) / (InfoTextSize.cy+(INTERRAW*2));
  } else {
  	TopSize+=HEADRAW/2;
  	Numraws=(bottom - TopSize) / (InfoTextSize.cy+INTERRAW);
  }
  if (usetwolines) {; Numraws /=2; Numraws*=2; } // make it even number
  if (Numraws>MAXNEAREST) Numraws=MAXNEAREST;

  s_rawspace=(InfoTextSize.cy+(ScreenLandscape?INTERRAW:INTERRAW*2));

  // center in available vertical space
  int cs = (bottom-TopSize) - (Numraws*(InfoTextSize.cy+(ScreenLandscape?INTERRAW:INTERRAW*2)) +
      (ScreenLandscape?INTERRAW:INTERRAW*2) );

  if ( (cs>(Numraws-1) && (Numraws>1)) ) {
      s_rawspace+= cs/(Numraws-1);
      s_rawspace-= NIBLSCALE(1); // adjust rounding errors
      TopSize += (cs - ((cs/(Numraws-1))*(Numraws-1)))/2 -1;
  } else {
      TopSize += cs/2 -1;
  }

#define HMARGIN NIBLSCALE(2) // left and right margins for header selection 

  //
  // HEADER SORTBOXES
  //
  Surface.SelectObject(LK8InfoNearestFont);
  if (ScreenLandscape)
      Surface.GetTextSize( _T("MMMM 3/3"), 8, &phdrTextSize);
  else
      Surface.GetTextSize( _T("MMM 3/3"), 7, &phdrTextSize);

  s_sortBox[0].left=Column0-NIBLSCALE(1); 
  s_sortBox[0].right=Column0 + phdrTextSize.cx;
  s_sortBox[0].top=2;
  s_sortBox[0].bottom=p1.y-NIBLSCALE(1);;
  SortBoxX[MSM_LANDABLE][0]=s_sortBox[0].right;
  SortBoxX[MSM_AIRPORTS][0]=s_sortBox[0].right;
  SortBoxX[MSM_NEARTPS][0]=s_sortBox[0].right;

  int headerspacing= (right-s_sortBox[0].right)/4; // used only for monoline portrait

  s_sortBox[1].left=s_sortBox[0].right+HMARGIN;
  s_sortBox[1].right=s_sortBox[0].right+headerspacing-HMARGIN; 
  s_sortBox[1].top=2;
  s_sortBox[1].bottom=p1.y-NIBLSCALE(1);
  SortBoxX[MSM_LANDABLE][1]=s_sortBox[1].right;
  SortBoxX[MSM_AIRPORTS][1]=s_sortBox[1].right;
  SortBoxX[MSM_NEARTPS][1]=s_sortBox[1].right;

  s_sortBox[2].left=s_sortBox[1].right+HMARGIN; 
  s_sortBox[2].right=s_sortBox[1].right+headerspacing;
  s_sortBox[2].top=2;
  s_sortBox[2].bottom=p1.y-NIBLSCALE(1);
  SortBoxX[MSM_LANDABLE][2]=s_sortBox[2].right;
  SortBoxX[MSM_AIRPORTS][2]=s_sortBox[2].right;
  SortBoxX[MSM_NEARTPS][2]=s_sortBox[2].right;

  s_sortBox[3].left=s_sortBox[2].right+HMARGIN;
  s_sortBox[3].right=s_sortBox[2].right+headerspacing;
  s_sortBox[3].top=2;
  s_sortBox[3].bottom=p1.y-NIBLSCALE(1);
  SortBoxX[MSM_LANDABLE][3]=s_sortBox[3].right;
  SortBoxX[MSM_AIRPORTS][3]=s_sortBox[3].right;
  SortBoxX[MSM_NEARTPS][3]=s_sortBox[3].right;

  s_sortBox[4].left=s_sortBox[3].right+HMARGIN;
  s_sortBox[4].right=right;
  s_sortBox[4].top=2;
  s_sortBox[4].bottom=p1.y-NIBLSCALE(1);
  SortBoxX[MSM_LANDABLE][4]=s_sortBox[4].right;
  SortBoxX[MSM_AIRPORTS][4]=s_sortBox[4].right;
  SortBoxX[MSM_NEARTPS][4]=s_sortBox[4].right;

  SortBoxY[MSM_LANDABLE]=p1.y;
  SortBoxY[MSM_AIRPORTS]=p1.y;
  SortBoxY[MSM_NEARTPS]=p1.y;

  Numpages=roundupdivision(MAXNEAREST*lincr, Numraws);

  if (Numpages>MAXNUMPAGES) Numpages=MAXNUMPAGES;
  else if (Numpages<1) Numpages=1;

  SelectedRaw[MSM_LANDABLE]=0; SelectedRaw[MSM_AIRPORTS]=0; SelectedRaw[MSM_NEARTPS]=0;
  SelectedPage[MSM_LANDABLE]=0; SelectedPage[MSM_AIRPORTS]=0; SelectedPage[MSM_NEARTPS]=0;

  hColumn2=s_sortBox[1].right-NIBLSCALE(2); 
  hColumn3=s_sortBox[2].right-NIBLSCALE(2);
  hColumn4=s_sortBox[3].right-NIBLSCALE(2);
  hColumn5=s_sortBox[4].right-NIBLSCALE(2);

  DoInit[MDI_DRAWNEAREST]=false;
  return;
  } // doinit



  Numpages=roundupdivision(SortedNumber*lincr, Numraws);
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
				i=SortedLandableIndex[SelectedRaw[curmapspace]+(curpage*Numraws/lincr)];
				break;
			case MSM_AIRPORTS:
				i=SortedAirportIndex[SelectedRaw[curmapspace] + (curpage*Numraws/lincr)];
				break;
			case MSM_NEARTPS:
			default:
				i=SortedTurnpointIndex[SelectedRaw[curmapspace] + (curpage*Numraws/lincr)];
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
		SelectedRaw[curmapspace] += 1;
		if (SelectedRaw[curmapspace] >=Numraws) SelectedRaw[curmapspace]=0;
		LastDoNearest=DrawInfo.Time+PAGINGTIMEOUT-1.0; 
		break;
	case LKEVENT_UP:
		SelectedRaw[curmapspace] -= 1;//
		if (SelectedRaw[curmapspace] <0) SelectedRaw[curmapspace]=Numraws-1;
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

  Surface.SelectObject(LK8InfoNearestFont); // Heading line

  short cursortbox=SortedMode[curmapspace];

  //
  // DRAW HEADER ROW WITH SORTBOXES
  // 

  Surface.FillRect(&s_sortBox[cursortbox], INVERTCOLORS?LKBrush_LightGreen:LKBrush_DarkGreen);

  // PAGE INDEX, example: 2.1
  //
  Surface.SelectObject(LK8PanelMediumFont);
  _stprintf(Buffer,TEXT("%d.%d"),ModeIndex,CURTYPE+1);
  LKWriteText(Surface,  Buffer, LEFTLIMITER, rc.top+TOPLIMITER , 0,  WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);


  unsigned int tmptoken;
  LKColor  tmpcolor;
  bool compact= (ScreenGeometry==SCREEN_GEOMETRY_43)||!ScreenLandscape;

  Surface.SelectObject(LK8InfoNearestFont);


  // PAGE NAME, example: APT 
  //
  switch(curmapspace) {
      case MSM_LANDABLE:
          tmptoken= ScreenLandscape? 1312:1311; // LKTOKEN _@M1311_ "LND" _@M1312_ "LNDB"
          break;
      case MSM_AIRPORTS:
          tmptoken= ScreenLandscape? 1314:1313; // LKTOKEN _@M1313_ "APT"  _@M1314_ "APTS"
          break;
      case MSM_NEARTPS:
          tmptoken= 1315; // LKTOKEN _@M1315_ "TPS"
          break;
      default:
          _stprintf(Buffer,TEXT("%s %d/%d"), MsgToken(1315), curpage+1, Numpages); 
          // LKTOKEN _@M266_ "Error"
          tmptoken= 266;
          break;
  }
  _stprintf(Buffer,TEXT("%s %d/%d"), MsgToken(tmptoken), curpage+1, Numpages); 

  tmpcolor= cursortbox==0?RGB_BLACK:RGB_LIGHTGREEN;
  LKWriteText(Surface,  Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0, WTMODE_NORMAL, WTALIGN_LEFT, tmpcolor, false);

  tmptoken=compact?1300:1304; // LKTOKEN _@M1300_ "Dist"  _@M1304_ "Distance"
  _tcscpy(Buffer,MsgToken(tmptoken)); 
  tmpcolor= cursortbox==1?RGB_BLACK:RGB_WHITE;
  LKWriteText(Surface,  Buffer, hColumn2, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, tmpcolor, false);

  tmptoken=compact?1301:1305; // LKTOKEN _@M1301_ "Dir" LKTOKEN _@M1305_ "Direction"
  _tcscpy(Buffer,MsgToken(tmptoken)); 
  tmpcolor= cursortbox==2?RGB_BLACK:RGB_WHITE;
  LKWriteText(Surface,  Buffer, hColumn3, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, tmpcolor, false);

  tmptoken=compact?1302:1306; // LKTOKEN _@M1302_ "rEff"  LKTOKEN _@M1306_ "ReqEff"
  _tcscpy(Buffer,MsgToken(tmptoken)); 
  tmpcolor= cursortbox==3?RGB_BLACK:RGB_WHITE;
  LKWriteText(Surface,  Buffer, hColumn4, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, tmpcolor, false);

  // In v5 using 1308 for landscape compact. Now simplified because compact is also for portrait
  // LKTOKEN _@M1303_ "AltA"
  // LKTOKEN _@M1307_ "AltArr"
  // LKTOKEN _@M1308_ "Arriv"
  tmptoken=compact?1303:1307; 
  _tcscpy(Buffer,MsgToken(tmptoken)); 
  tmpcolor= cursortbox==4?RGB_BLACK:RGB_WHITE;
  LKWriteText(Surface,  Buffer, hColumn5, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, tmpcolor, false);


  Surface.SelectObject(bigFont); // Text font for Nearest

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
	
  for (i=0, j=0, drawn_items_onpage=0; i<Numraws; j++, i+=lincr) {
	iRaw=TopSize+(s_rawspace*i);
	short curraw=(curpage*Numraws);
        if (usetwolines) {
            curraw/=2;
            curraw+=j;
        } else {
            curraw+=i;
        }
	if (curraw>=MAXNEAREST) break;

	rli=*(psortedindex+curraw);

	if (!ndr) {
		goto KeepOldValues;
	}
	if ( ValidWayPoint(rli) ) {

		LK_tcsncpy(Buffer, WayPointList[rli].Name, s_maxnlname);
		CharUpper(Buffer);
		_tcscpy(Buffer1[i][curpage],Buffer); 

		value=WayPointCalc[rli].Distance*DISTANCEMODIFY;
		if (usetwolines) 
         	    _stprintf(Buffer2[i][curpage],TEXT("%0.1lf %s"),value,Units::GetDistanceName());
		else
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
		if (usetwolines) _stprintf(Buffer5[i][curpage], TEXT("%s %s"),text,Units::GetAltitudeName() );
		else _stprintf(Buffer5[i][curpage], TEXT("%s"),text);

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
  			Surface.SelectObject(bigItalicFont);
		} else {
			rcolor=RGB_WHITE;
  			Surface.SelectObject(bigFont);
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

  	Surface.SelectObject(bigFont); 
        if (!usetwolines) {
	    LKWriteText(Surface,  Buffer1[i][curpage], Column1, iRaw , 0, WTMODE_NORMAL, WTALIGN_LEFT, rcolor, false);
	    LKWriteText(Surface,  Buffer2[i][curpage], Column2, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
	    LKWriteText(Surface,  Buffer3[i][curpage], Column3, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
	    LKWriteText(Surface,  Buffer4[i][curpage], Column4, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
	    LKWriteText(Surface,  Buffer5[i][curpage], Column5, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
	} else {
	    LKWriteText(Surface,  Buffer1[i][curpage], Column1, iRaw , 0, WTMODE_NORMAL, WTALIGN_LEFT, rcolor, false);
	    LKWriteText(Surface,  Buffer2[i][curpage], Column5, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
	    iRaw+=s_rawspace;
	    LKWriteText(Surface,  Buffer3[i][curpage], Column2, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
	    LKWriteText(Surface,  Buffer4[i][curpage], Column3, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
	    LKWriteText(Surface,  Buffer5[i][curpage], Column4, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
	}

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
        if (usetwolines) {
            invsel.top=TopSize+(s_rawspace*SelectedRaw[curmapspace]*lincr);
        } else {
            invsel.top=TopSize+(s_rawspace*SelectedRaw[curmapspace])+NIBLSCALE(2);
        }
        invsel.bottom=TopSize+(s_rawspace*(SelectedRaw[curmapspace]*lincr+1))-NIBLSCALE(1);
        if (usetwolines) invsel.bottom+=s_rawspace;

        #ifdef __linux__  
        invsel.top -= (HEADRAW/2 - NIBLSCALE(2)); 
        invsel.bottom -= NIBLSCALE(1);  // interline
        #endif

	Surface.InvertRect(invsel);

  } 

  LKevent=LKEVENT_NONE;
  return;
}

