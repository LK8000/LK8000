/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKInterface.h"
#include "Logger.h"
#include "LKProcess.h"
#include "LKObjects.h"
#include "RGB.h"
#include "DoInits.h"
#include "McReady.h"


extern NMEAParser nmeaParser1;
extern NMEAParser nmeaParser2;

void MapWindow::DrawBottomBar(HDC hdc,  RECT rc )
{

  TCHAR BufferValue[LKSIZEBUFFERVALUE];
  TCHAR BufferUnit[LKSIZEBUFFERUNIT];
  TCHAR BufferTitle[LKSIZEBUFFERTITLE];

  char text[LKSIZETEXT];
  int index=-1;
  double Value;
  short rcx, rcy;

  SIZE TextSize;


  static bool wascircling=false; // init not circling of course
  static short OldBottomMode=BM_FIRST;

  COLORREF barTextColor=RGB_WHITE; // default bottom bar text color, reversable

  // position Y of text in navboxes
  static short yRow2Title=0;	// higher row in portrait, unused in landscape
  static short yRow2Value=0;
  static short yRow2Unit=0;
  static short yRow1Title=0;	// lower row in portrait, the only one in landscape
  static short yRow1Value=0;
  static short yRow1Unit=0;

  static int splitoffset;
  static int splitoffset2; // second raw, which really is the first from top!

  HBRUSH brush_bar;
  if (INVERTCOLORS) {
    brush_bar = LKBrush_Black;
  } else {
    brush_bar = LKBrush_Nlight;
  }


  if (DoInit[MDI_DRAWBOTTOMBAR]) {

	wascircling=false;
	OldBottomMode=BM_FIRST;

	TCHAR Tdummy[]=_T("T");
	int iconsize;

	// All these values are fine tuned for font/resolution/screenmode.
	// there is no speed issue inside doinit. take your time.
	SelectObject(hdc, LK8TitleNavboxFont); 
	GetTextExtentPoint(hdc, Tdummy, _tcslen(Tdummy), &TextSize);
	int syTitle = TextSize.cy;
	SelectObject(hdc, LK8ValueFont); 
	GetTextExtentPoint(hdc, Tdummy, _tcslen(Tdummy), &TextSize);
	int syValue = TextSize.cy;

	switch (ScreenSize) {
		// Row1 is the lower, Row2 is the top, for portrait
		// WARNING, algos are wrong, need to check and recalculate for each resolution!!
		// Changing font size in Utils2 does require checking and fixing here.
		case ss480x640:
		case ss480x800:
		case ss240x320:
		case ss272x480:
			yRow2Value =  rc.bottom-(syValue*2);
			yRow2Unit  =  yRow2Value;
			yRow2Title =  yRow2Value - (syValue/2) - (syTitle/2) + NIBLSCALE(2);
			yRow1Value =  rc.bottom-(syValue/2);
			yRow1Unit  =  yRow1Value;
			yRow1Title =  yRow1Value - (syValue/2) - (syTitle/2) + NIBLSCALE(2);
			break;
		
		case ss800x480:
		case ss640x480:
		case ss400x240:
			yRow2Value =  rc.bottom-(syValue*2);
			yRow2Unit  =  rc.bottom-(syValue*2) - NIBLSCALE(2);
			yRow2Title =  rc.bottom-(syValue*2) - syTitle;
			yRow1Value =  rc.bottom-(syValue/2);
			yRow1Unit  =  yRow1Value;
			yRow1Title =  rc.bottom-(syValue/2) - syTitle;
			break;

		default:
			yRow2Value =  rc.bottom-(syValue*2);
			yRow2Unit  =  rc.bottom-(syValue*2) - NIBLSCALE(2);
			yRow2Title =  rc.bottom-(syValue*2) - syTitle;
			yRow1Value =  rc.bottom-(syValue/2);
			yRow1Unit  =  rc.bottom-(syValue/2) - NIBLSCALE(2);
			yRow1Title =  rc.bottom-(syValue/2) - syTitle;
			break;
	}

	
	if (ScreenLandscape) {
		iconsize=NIBLSCALE(26);
		splitoffset= ((rc.right-iconsize)-rc.left)/splitter;
	} else {
		iconsize=NIBLSCALE(26);
		splitoffset= ((rc.right-iconsize)-rc.left)/splitter;
		// splitoffset2= (rc.right-rc.left)/splitter;
		splitoffset2= splitoffset;
	}

	short ii;
	// set correct initial bottombar stripe, excluding TRM
	for (ii=BM_CRU; ii<=BM_LAST;ii++) {
		if (ConfBB[ii]) break;
	}
	BottomMode=ii;

	DoInit[MDI_DRAWBOTTOMBAR]=false; 

  } // end doinit


    RECT nrc;
    nrc.left=0;
    nrc.top=rc.bottom-BottomSize;
    nrc.right=rc.right;
    nrc.bottom=rc.bottom;


  if (MapWindow::AlphaBlendSupported() && MapSpaceMode==MSM_MAP && BarOpacity<100) {
	static HDC hdc2=NULL;
	static HBITMAP bitmapnew=NULL, bitmapold=NULL;
	if (DoInit[MDI_LOOKABLEND]) {
		// drop old object to allow deleteDC correctly
		if (bitmapold) SelectObject(hdc2,bitmapold);
		if (bitmapnew) DeleteObject(bitmapnew);
		if (hdc2) DeleteObject(hdc2);

		hdc2=CreateCompatibleDC(hdc);
		bitmapnew=CreateCompatibleBitmap(hdc,rc.right,rc.bottom);
		DoInit[MDI_LOOKABLEND]=false;
	}

	if (BarOpacity==0) {
		barTextColor=RGB_BLACK;
	} else {
		BLENDFUNCTION bs;
		bs.BlendOp=AC_SRC_OVER;
		bs.BlendFlags=0;
		// A good value is 195
		bs.SourceConstantAlpha=BarOpacity*255/100;
		bs.AlphaFormat=0;
		bitmapold=(HBITMAP)SelectObject(hdc2,bitmapnew); 
		FillRect(hdc2,&nrc, brush_bar); 
		MapWindow::AlphaBlendF(hdc,0,rc.bottom-BottomSize,rc.right,BottomSize,hdc2,0,rc.bottom-BottomSize,rc.right,BottomSize,bs);
		if (BarOpacity>25)
			barTextColor=RGB_WHITE;
		else
			barTextColor=RGB_BLACK;
	}

	
  } else {
	barTextColor=RGB_WHITE;
	FillRect(hdc,&nrc, brush_bar); 
  }

  // NAVBOXES

  bool showunit=false;

  if (!ConfBB0Auto) goto _afterautotrm;

  if ( MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING) && !wascircling) {
	// switch to thermal mode
	OldBottomMode=BottomMode;
	BottomMode=BM_TRM;
	wascircling=true;
  }
  if ( !MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING) && wascircling) {
	// back to cruise mode
	BottomMode=OldBottomMode;
	wascircling=false;
  }

_afterautotrm:

  /*
   *   FIRST VALUE
   */

  showunit=true; // normally we do have a unit to show


  switch(BottomMode) {
	case BM_TRM:
		index=GetInfoboxIndex(1,MapWindow::Mode::MODE_FLY_CIRCLING);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CRU:
		if (ISCAR)
			showunit=LKFormatValue(LK_ODOMETER, true, BufferValue, BufferUnit, BufferTitle);
		else
			showunit=LKFormatValue(LK_TL_AVG, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_HGH:
		showunit=LKFormatValue(LK_HGPS, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_AUX:
		if (ISCAR) {
			_stprintf(BufferTitle, MsgToken(1809)); // Moving
			if (DerivedDrawInfo.Flying) {
                                showunit=Units::TimeToTextDown(BufferValue, (int)Trip_Moving_Time);
				if (showunit)
					_stprintf(BufferUnit, _T("h"));
				else 
					_stprintf(BufferUnit, _T(""));
				showunit=true;
                        } else {
                                wsprintf(BufferValue, TEXT("--:--"));
				showunit=false;
                        }
		} else {
			showunit=LKFormatValue(LK_TC_ALL, true, BufferValue, BufferUnit, BufferTitle);
		}
		break;
	case BM_TSK:
		showunit=LKFormatValue(LK_FIN_DIST, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_ALT:
		showunit=LKFormatValue(LK_BESTALTERN_GR, true, BufferValue, BufferUnit, BufferTitle); // 100221
		BufferTitle[7]='\0';
		break;
	case BM_SYS:
		showunit=LKFormatValue(LK_BATTERY, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;
	case BM_CUS2:
		index=GetInfoboxIndex(1,MapWindow::Mode::MODE_FLY_CRUISE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CUS3:
		index=GetInfoboxIndex(1,MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CUS:
		index=GetInfoboxType(1);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;

	default:
		showunit=LKFormatValue(LK_ERROR, true, BufferValue, BufferUnit, BufferTitle);
		break;
  }

  rcx=rc.left+(splitoffset/2);
  if (ScreenLandscape) {
	#include "LKMW3include_navbox1.cpp"
  } else {
	#include "LKMW3include_navbox2.cpp"
  }
  LKWriteText(hdc, BufferTitle, rcx+NIBLSCALE(7), rcy, 0, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);

  /*
   *   SECOND VALUE
   */
  showunit=true;
  switch(BottomMode) {
	case BM_TRM:
		index=GetInfoboxIndex(2,MapWindow::Mode::MODE_FLY_CIRCLING);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CRU:
		showunit=LKFormatValue(LK_GNDSPEED, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_HGH:
		showunit=LKFormatValue(LK_HBARO, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_AUX:
		if (ISCAR) {
			_stprintf(BufferTitle, MsgToken(1810)); // Steady
			if (DerivedDrawInfo.Flying) {
                                showunit=Units::TimeToTextDown(BufferValue, (int)Trip_Steady_Time);
				if (showunit)
					_stprintf(BufferUnit, _T("h"));
				else 
					_stprintf(BufferUnit, _T(""));
				showunit=true;
                        } else {
                                wsprintf(BufferValue, TEXT("--:--"));
				showunit=false;
                        }
		} else {
			if (UseContestEngine())
			  showunit=LKFormatValue(LK_OLC_CLASSIC_DIST, true, BufferValue, BufferUnit, BufferTitle);
			else
			  showunit=LKFormatValue(LK_ODOMETER, true, BufferValue, BufferUnit, BufferTitle); // 100221
		}
		break;
	case BM_TSK:
		showunit=LKFormatValue(LK_FIN_ALTDIFF, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;
	case BM_ALT:
		if (ScreenLandscape) {
			showunit=LKFormatValue(LK_BESTALTERN_ARRIV, false, BufferValue, BufferUnit, BufferTitle);
			wcscpy(BufferTitle,_T("<<<"));
		} else {
			showunit=LKFormatValue(LK_ALTERN1_GR, true, BufferValue, BufferUnit, BufferTitle);
			BufferTitle[7]='\0';
		}
		break;
	case BM_SYS:
		showunit=LKFormatValue(LK_EXTBATT1VOLT, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;

	case BM_CUS2:
		index=GetInfoboxIndex(2,MapWindow::Mode::MODE_FLY_CRUISE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
		
	case BM_CUS3:
		index=GetInfoboxIndex(2,MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CUS:
		index=GetInfoboxType(2);
		showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;

	default:
		showunit=LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
		break;
  }

  rcx+=splitoffset;
  if (ScreenLandscape) {
	#include "LKMW3include_navbox1.cpp"
  } else {
	#include "LKMW3include_navbox2.cpp"
  }
  LKWriteText(hdc, BufferTitle, rcx+NIBLSCALE(7), rcy, 0, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);

  /*
   *   THIRD VALUE
   */

  showunit=true;
  switch(BottomMode) {
	case BM_TRM:
		index=GetInfoboxIndex(3,MapWindow::Mode::MODE_FLY_CIRCLING);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CRU:
		showunit=LKFormatValue(LK_HNAV, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;
	case BM_HGH:
		showunit=LKFormatValue(LK_QFE, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_AUX:
		if (ISCAR) {
			_stprintf(BufferTitle, MsgToken(1811)); // Total
			if (DerivedDrawInfo.Flying) {
                                showunit=Units::TimeToTextDown(BufferValue, (int)(Trip_Steady_Time+Trip_Moving_Time));
				if (showunit)
					_stprintf(BufferUnit, _T("h"));
				else 
					_stprintf(BufferUnit, _T(""));
				showunit=true;
                        } else {
                                wsprintf(BufferValue, TEXT("--:--"));
				showunit=false;
                        }
		} else {
			showunit=LKFormatValue(LK_TIMEFLIGHT, true, BufferValue, BufferUnit, BufferTitle); // 100221
		}
		break;
	case BM_TSK:
		showunit=LKFormatValue(LK_FIN_ETE, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_ALT:
		if (ScreenLandscape)
			showunit=LKFormatValue(LK_ALTERN1_GR, true, BufferValue, BufferUnit, BufferTitle); // 100221
		else
			showunit=LKFormatValue(LK_ALTERN2_GR, true, BufferValue, BufferUnit, BufferTitle); // 100221
		BufferTitle[7]='\0';
		break;
	case BM_SYS:
  		showunit=true;
  			wsprintf(BufferUnit, TEXT(""));
			if (SIMMODE) {
				// LKTOKEN _@M1199_ "Sat"
				wsprintf(BufferTitle, MsgToken(1199));
				wsprintf(BufferValue,TEXT("SIM"));
			} else {
				Value=DrawInfo.SatellitesUsed;
				if (Value<1 || Value>30) {
					wsprintf(BufferValue,TEXT("---"));
				} else {
					sprintf(text,"%d",(int)Value);
					wsprintf(BufferValue, TEXT("%S"),text);

				}
				if (nmeaParser1.activeGPS == true)
					// LKTOKEN _@M1199_ "Sat"
					wsprintf(BufferTitle, TEXT("%s:A"), MsgToken(1199));
				else {
					if (nmeaParser2.activeGPS == true)
						// LKTOKEN _@M1199_ "Sat"
						wsprintf(BufferTitle, TEXT("%s:B"), MsgToken(1199));
					else
						// LKTOKEN _@M1199_ "Sat"
						wsprintf(BufferTitle, TEXT("%s:?"), MsgToken(1199));
				}
			}
		break;
	case BM_CUS2:
		index=GetInfoboxIndex(3,MapWindow::Mode::MODE_FLY_CRUISE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
		
	case BM_CUS3:
		index=GetInfoboxIndex(3,MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CUS:
		index=GetInfoboxType(3);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;

	default:
		showunit=LKFormatValue(LK_ERROR, true, BufferValue, BufferUnit, BufferTitle);
		break;
  }

  rcx+=splitoffset;
  if (ScreenLandscape) {
	#include "LKMW3include_navbox1.cpp"
  } else {
	#include "LKMW3include_navbox2.cpp"
  }
  LKWriteText(hdc, BufferTitle, rcx+NIBLSCALE(7), rcy, 0, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);

  /*
   *   FOURTH VALUE
   */

  showunit=true;
  switch(BottomMode) {
	case BM_TRM:
		index=GetInfoboxIndex(4,MapWindow::Mode::MODE_FLY_CIRCLING);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CRU:
		showunit=LKFormatValue(LK_TRACK, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_HGH:
		showunit=LKFormatValue(LK_HAGL, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_AUX:  
		if (ISCAR) {
			_stprintf(BufferValue,_T("%.1f"),SPEEDMODIFY*Rotary_Speed);
			_stprintf(BufferTitle,_T("AvgSpd"));
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			showunit=true;
		} else {
			showunit=LKFormatValue(LK_HOME_DIST, true, BufferValue, BufferUnit, BufferTitle);
		}
		break;
	case BM_TSK:
		showunit=LKFormatValue(LK_TASK_DISTCOV, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_ALT:
		if (ScreenLandscape) {
			showunit=LKFormatValue(LK_ALTERN1_ARRIV, true, BufferValue, BufferUnit, BufferTitle); // 100221
			wcscpy(BufferTitle,_T("<<<"));
		} else {
			showunit=LKFormatValue(LK_BESTALTERN_ARRIV, true, BufferValue, BufferUnit, BufferTitle); // 100221
			wcscpy(BufferTitle,_T(""));
		}
		break;
	case BM_SYS:
		// LKTOKEN _@M1068_ "HBAR"
  		wsprintf(BufferTitle, MsgToken(1068));
		if (DrawInfo.BaroAltitudeAvailable) {
			if (EnableNavBaroAltitude)
				// LKTOKEN _@M894_ "ON"
				wsprintf(BufferValue,MsgToken(894));
			else
				// LKTOKEN _@M491_ "OFF"
				wsprintf(BufferValue,MsgToken(491));
		} else
			wsprintf(BufferValue,TEXT("---"));
  		showunit=false;
		break;
	case BM_CUS2:
		index=GetInfoboxIndex(4,MapWindow::Mode::MODE_FLY_CRUISE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
		
	case BM_CUS3:
		index=GetInfoboxIndex(4,MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CUS:
		index=GetInfoboxType(4);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;

	default:
		showunit=LKFormatValue(LK_ERROR, true, BufferValue, BufferUnit, BufferTitle);
		break;
  }


  if (ScreenLandscape) {
	rcx+=splitoffset;
  }else {
	rcx=rc.left+(splitoffset2/2);
  }
  #include "LKMW3include_navbox1.cpp"
  LKWriteText(hdc, BufferTitle, rcx+NIBLSCALE(7), rcy, 0, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);

  /*
   *   FIFTH VALUE
   */
  if (ScreenLandscape && (splitter<5)) goto EndOfNavboxes;
  showunit=true;
  switch(BottomMode) {
	case BM_TRM:
		index=GetInfoboxIndex(5,MapWindow::Mode::MODE_FLY_CIRCLING);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CRU: 
		if (ISCAR) {
			_tcscpy(BufferTitle,MsgToken(2091)); // Averag (speed average really)
			int totime=(int)(Trip_Steady_Time+Trip_Moving_Time);
			if (totime>0) {
				_stprintf(BufferValue,_T("%.1f"),(DerivedDrawInfo.Odometer*SPEEDMODIFY)/totime);
  				wsprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			} else {
				wsprintf(BufferValue, TEXT("---"));
				showunit=false;
			}
		} else
			showunit=LKFormatValue(LK_HEADWINDSPEED, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_HGH:
		showunit=LKFormatValue(LK_FL, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_AUX:
		if (ISCAR) {
			if (Rotary_Distance<100000) {
				_stprintf(BufferValue,_T("%.2f"),DISTANCEMODIFY*Rotary_Distance);
			} else {
				_stprintf(BufferValue,_T("%.2f"),0);
			}
			_stprintf(BufferTitle,_T("AvgDist"));
			wsprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			showunit=true;
		} else {
			showunit=LKFormatValue(LK_MAXALT, true, BufferValue, BufferUnit, BufferTitle);
		}
		break;
	case BM_TSK:
// TODO MAKE IT LKPROCESS
  		Value=ALTITUDEMODIFY*DerivedDrawInfo.TaskStartAltitude;
		if (Value>0) {
			sprintf(text,"%d",(int)Value);
			wsprintf(BufferValue, TEXT("%S"),text);
  			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
		} else {
			wsprintf(BufferValue, TEXT("---"));
			wsprintf(BufferUnit, TEXT(""));
			showunit=false;
		}
 		// LKTOKEN _@M1200_ "Start"
		wsprintf(BufferTitle, MsgToken(1200));
		break;
	case BM_ALT:
		if (ScreenLandscape) {
			showunit=LKFormatValue(LK_ALTERN2_GR, true, BufferValue, BufferUnit, BufferTitle); // 100221
			BufferTitle[7]='\0';
		} else {
			showunit=LKFormatValue(LK_ALTERN1_ARRIV, false, BufferValue, BufferUnit, BufferTitle); // 100221
			wcscpy(BufferTitle,_T(""));
		}
		break;
	case BM_SYS:
		//showunit=LKFormatValue(LK_LOGGER, true, BufferValue, BufferUnit, BufferTitle);
		showunit=true;
		extern int CpuSummary();
		_stprintf(BufferValue,_T("%d"),CpuSummary());
		wcscpy(BufferTitle,_T("CPU"));
		wcscpy(BufferUnit,_T("%"));
		break;
	case BM_CUS2:
		index=GetInfoboxIndex(5,MapWindow::Mode::MODE_FLY_CRUISE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
		
	case BM_CUS3:
		index=GetInfoboxIndex(5,MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CUS:
		index=GetInfoboxType(5);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;

	default:
		showunit=LKFormatValue(LK_ERROR, true, BufferValue, BufferUnit, BufferTitle);
		break;
  }


  if (ScreenLandscape) {
	rcx+=splitoffset;
  }else {
	rcx+=splitoffset2;
  }
  #include "LKMW3include_navbox1.cpp"

  LKWriteText(hdc, BufferTitle, rcx+NIBLSCALE(3), rcy, 0, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);

  /*
   *   SIXTH VALUE
   */
  if (ScreenLandscape && (splitter<6)) goto EndOfNavboxes;
  showunit=true;
  switch(BottomMode) {
	case BM_TRM:
		index=GetInfoboxIndex(6,MapWindow::Mode::MODE_FLY_CIRCLING);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CRU:
		if (ISCAR)
			showunit=LKFormatValue(LK_TIME_LOCAL, false, BufferValue, BufferUnit, BufferTitle);
		else
			showunit=LKFormatValue(LK_LD_INST, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_HGH:
		showunit=LKFormatValue(LK_AQNH, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;
	case BM_AUX:
		if (ISCAR)
			showunit=LKFormatValue(LK_MAXALT, true, BufferValue, BufferUnit, BufferTitle);
		else
			showunit=LKFormatValue(LK_ODOMETER, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;
	case BM_TSK:
		showunit=LKFormatValue(LK_SPEEDTASK_ACH, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_ALT:
		showunit=LKFormatValue(LK_ALTERN2_ARRIV, true, BufferValue, BufferUnit, BufferTitle); // 100221
		if (ScreenLandscape)
			wcscpy(BufferTitle,_T("<<<"));
		else
			wcscpy(BufferTitle,_T(""));
		break;
	case BM_SYS:
		showunit=LKFormatValue(LK_LOGGER, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_CUS2:
		index=GetInfoboxIndex(6,MapWindow::Mode::MODE_FLY_CRUISE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
		
	case BM_CUS3:
		index=GetInfoboxIndex(6,MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CUS:
		index=GetInfoboxType(6);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;

	default:
		showunit=LKFormatValue(LK_ERROR, true, BufferValue, BufferUnit, BufferTitle);
		break;
  }


  if (ScreenLandscape) {
	rcx+=splitoffset;
  }else {
	rcx+=splitoffset2;
  }
  #include "LKMW3include_navbox1.cpp"
  LKWriteText(hdc, BufferTitle, rcx+NIBLSCALE(3), rcy, 0, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);

  /*
   *    CLEAN UP 
   */

EndOfNavboxes:
  ;


  // restore objects here, before returning


}

