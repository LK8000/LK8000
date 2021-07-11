/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
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
#include "Util/UTF8.hpp"
#include "Asset.hpp"
#include "OS/CpuLoad.h"

extern void VDrawLine(LKSurface& Surface, const RECT& rc, int x1, int y1, int x2, int y2, const LKColor& col);


// Approx.size of the bottom-right icon corner on the bottom bar
#define BB_ICONSIZE NIBLSCALE(26)

void CropString(TCHAR* String, unsigned max_char) {
#ifdef UNICODE
    String[max_char] = _T('\0');
#else
    // utf8 : number of Char are not equal to Number of Byte, we need to iterate each code point
    auto next = NextUTF8(String);
    while (next.second && --max_char) {
        next = NextUTF8(next.second);
    }
    // we can use const cast here "next.second" are inside String content.
    char* pend = const_cast<char*>(next.second);
    if(pend) {
        // check if pend are inside String.
        LKASSERT(static_cast<size_t>(pend - String) <= strlen(String));
        (*pend) = '\0';
    }
#endif
}

void MapWindow::DrawBottomBar(LKSurface& Surface, const RECT& rc )
{

  TCHAR BufferValue[LKSIZEBUFFERVALUE];
  TCHAR BufferUnit[LKSIZEBUFFERUNIT];
  TCHAR BufferTitle[LKSIZEBUFFERTITLE];

  int index=-1;
  double Value;
  short rcx, rcy;
  short userow2=0;

  SIZE TextSize;


  static short lastMode=-1; // init to not existing mode
  static short OldBottomMode=BM_FIRST;

  LKColor barTextColor=RGB_WHITE; // default bottom bar text color, reversable

  // position Y of text in navboxes
  static short yRow2Title=0;	// higher row in portrait, unused in landscape
  static short yRow2Value=0;
  static short yRow2Unit=0;
  static short yRow1Title=0;	// lower row in portrait, the only one in landscape
  static short yRow1Value=0;
  static short yRow1Unit=0;

  static int bbsplitter;   // how many items on a single row on the bottom bar
  static int splitoffset;
  static int splitoffset2; // second raw, which really is the first from top!

  if (DoInit[MDI_DRAWBOTTOMBAR]) {

    lastMode=-1;
	OldBottomMode=BM_FIRST;

	Surface.SelectObject(LK8BottomBarTitleFont);
	Surface.GetTextSize(_T("M"), &TextSize);
	int syTitle = TextSize.cy;

	Surface.SelectObject(LK8BottomBarUnitFont);
	// m for meters unit, f is shorter anyway
	Surface.GetTextSize(_T("m"), &TextSize);
	int sxUnit = TextSize.cx;

	Surface.SelectObject(LK8BottomBarValueFont);
        // we need to be able to print 12345f with no problems
	Surface.GetTextSize(_T("12345"), &TextSize);
	int syValue = TextSize.cy;
	int sxValue = TextSize.cx + NIBLSCALE(2) + (HideUnits ? 0 : sxUnit);

	// The vertical align of fonts cannot be done using the TextSize metrics..
	// because the reported height is considering also vertical spacing.
	// So this is a workaround. We cannot align perfectly.
        // WARNING. Changing these settings will require also Fonts1.cpp sizing of BottomBar
        // to be adjust as well!

	yRow2Value =  rc.bottom-(syValue*2);
	yRow2Title =  yRow2Value - (syValue/2) - (syTitle/2) + NIBLSCALE(2);
	yRow2Unit  =  yRow2Value;

	yRow1Value =  rc.bottom-(syValue/2);
	yRow1Title =  yRow1Value - (syValue/2) - (syTitle/2) + NIBLSCALE(2);
	yRow1Unit  =  yRow1Value;


        if (ScreenLandscape) {
		bbsplitter= iround( (double) ((rc.right-rc.left)-NIBLSCALE(26)) / (double)(sxValue) );
                if (bbsplitter>10) bbsplitter=10;
		splitoffset= ((rc.right-BB_ICONSIZE)-rc.left)/bbsplitter;
        } else {
		bbsplitter= iround( (double) ((rc.right-rc.left)-NIBLSCALE(26)) / (double)(sxValue) );
                if (bbsplitter>10) bbsplitter=10;
		splitoffset= ((rc.right-BB_ICONSIZE)-rc.left)/bbsplitter;
		splitoffset2= splitoffset;
        }
	#if TESTBENCH
	StartupStore(_T("... BOTTOMBAR splitter=%d\n"),bbsplitter);
	#endif


	short ii;
	// set correct initial bottombar stripe, excluding TRM
	if(!ConfBB[BottomMode])
	{
	    for (ii=BM_CRU; ii<=BM_LAST;ii++) {
                if (ConfBB[ii]) break;
	    }
            BottomMode=ii;
	}

        _tcscpy(BufferValue,_T(""));
        _tcscpy(BufferUnit,_T(""));
        _tcscpy(BufferTitle,_T(""));

	DoInit[MDI_DRAWBOTTOMBAR]=false;

  } // end doinit


    RECT nrc;
    nrc.left=0;
    nrc.top=rc.bottom-BottomSize;
    nrc.right=rc.right;
    nrc.bottom=rc.bottom;


    const auto& brush_bar = INVERTCOLORS?LKBrush_Black:LKBrush_Nlight;

    if (LKSurface::AlphaBlendSupported() && MapSpaceMode == MSM_MAP && BarOpacity < 100) {
        if (BarOpacity == 0) {
            barTextColor = RGB_BLACK;
        } else {
#ifdef USE_GDI
            TempSurface.FillRect(&nrc, brush_bar);
            Surface.AlphaBlend(nrc, TempSurface, nrc, BarOpacity * 255 / 100);
#else
            const LKBrush AlphaBrush(brush_bar.GetColor().WithAlpha(BarOpacity * 255 / 100));
            Surface.FillRect(&nrc, AlphaBrush);
#endif
            if (BarOpacity > 25) {
                barTextColor = RGB_WHITE;
            } else {
                barTextColor = RGB_BLACK;
            }
        }
    } else {
        barTextColor = RGB_WHITE;
        Surface.FillRect(&nrc, brush_bar);
        if(IsDithered()) {
            VDrawLine(Surface,rc, rc.left,rc.bottom-BottomSize,rc.right,rc.bottom-BottomSize,INVERTCOLORS?RGB_WHITE:RGB_BLACK);
        }
    }

  // NAVBOXES

  bool showunit=false;

  if (ConfBB0Auto == BBSM_AUTO_THERMALLING) {
    if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)
        && lastMode != MapWindow::Mode::MODE_CIRCLING) {
      OldBottomMode = BottomMode;
      BottomMode = BM_TRM;
    }
    else if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)
        && lastMode == MapWindow::Mode::MODE_CIRCLING) {
      BottomMode = OldBottomMode;
    }
    lastMode = MapWindow::mode.Fly();
  } else if (ConfBB0Auto == BBSM_FULL_AUTO) {
    if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)
        && lastMode != MapWindow::Mode::MODE_CIRCLING) {
      BottomMode = BM_TRM;
    }
    else if (MapWindow::mode.Is(MapWindow::Mode::MODE_CRUISE)
        && lastMode != MapWindow::Mode::MODE_CRUISE) {
      BottomMode = BM_CUS2;
    }
    else if (MapWindow::mode.Is(MapWindow::Mode::MODE_FINAL_GLIDE)
        && lastMode != MapWindow::Mode::MODE_FINAL_GLIDE) {
      BottomMode = BM_CUS3;
    }
    lastMode = MapWindow::mode.Fly();
  }



  /*
   *   FIRST VALUE
   */

  showunit=true; // normally we do have a unit to show
  int cpusummary=INVALID_VALUE;


  switch(BottomMode) {
	case BM_TRM:
		index=GetInfoboxIndex(1,MapWindow::Mode::MODE_FLY_CIRCLING);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
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
			_tcscpy(BufferTitle, MsgToken(1809)); // Moving
			if (DerivedDrawInfo.Flying) {
                                showunit=Units::TimeToTextDown(BufferValue, (int)Trip_Moving_Time);
				if (showunit)
					_tcscpy(BufferUnit, _T("h"));
				else
					_tcscpy(BufferUnit, _T(""));
				showunit=true;
                        } else {
                                _stprintf(BufferValue, TEXT("--:--"));
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
		CropString(BufferTitle, 7);
		break;
	case BM_SYS:
		showunit=LKFormatValue(LK_BATTERY, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;
	case BM_CUS2:
		index=GetInfoboxIndex(1,MapWindow::Mode::MODE_FLY_CRUISE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;
	case BM_CUS3:
		index=GetInfoboxIndex(1,MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;
	case BM_CUS:
		index=GetInfoboxType(1);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
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
  LKWriteText(Surface, BufferTitle, rcx+NIBLSCALE(7), rcy, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);

  /*
   *   SECOND VALUE
   */
  showunit=true;
  switch(BottomMode) {
	case BM_TRM:
		index=GetInfoboxIndex(2,MapWindow::Mode::MODE_FLY_CIRCLING);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;
	case BM_CRU:
		showunit=LKFormatValue(LK_GNDSPEED, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_HGH:
		showunit=LKFormatValue(LK_HBARO, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_AUX:
		if (ISCAR) {
			_tcscpy(BufferTitle, MsgToken(1810)); // Steady
			if (DerivedDrawInfo.Flying) {
                                showunit=Units::TimeToTextDown(BufferValue, (int)Trip_Steady_Time);
				if (showunit)
					_tcscpy(BufferUnit, _T("h"));
				else
					_tcscpy(BufferUnit, _T(""));
				showunit=true;
                        } else {
                                _stprintf(BufferValue, TEXT("--:--"));
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
			_tcscpy(BufferTitle,_T("<<<"));
		} else {
			showunit=LKFormatValue(LK_ALTERN1_GR, true, BufferValue, BufferUnit, BufferTitle);
			CropString(BufferTitle, 7);
		}
		break;
	case BM_SYS:
		showunit=LKFormatValue(LK_EXTBATT1VOLT, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;

	case BM_CUS2:
		index=GetInfoboxIndex(2,MapWindow::Mode::MODE_FLY_CRUISE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;

	case BM_CUS3:
		index=GetInfoboxIndex(2,MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;
	case BM_CUS:
		index=GetInfoboxType(2);
		showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
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
  LKWriteText(Surface, BufferTitle, rcx+NIBLSCALE(7), rcy, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);

  /*
   *   THIRD VALUE
   */

  if (bbsplitter<3) {
      if (ScreenLandscape) goto EndOfNavboxes;
      userow2++;
      rcx=rc.left+(splitoffset2/2);
  }
  else {
      rcx+=splitoffset;
  }
  showunit=true;
  switch(BottomMode) {
	case BM_TRM:
		index=GetInfoboxIndex(3,MapWindow::Mode::MODE_FLY_CIRCLING);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;
	case BM_CRU:
		showunit=LKFormatValue(LK_HNAV, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;
	case BM_HGH:
		showunit=LKFormatValue(LK_QFE, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_AUX:
		if (ISCAR) {
			_tcscpy(BufferTitle, MsgToken(1811)); // Total
			if (DerivedDrawInfo.Flying) {
                                showunit=Units::TimeToTextDown(BufferValue, (int)(Trip_Steady_Time+Trip_Moving_Time));
				if (showunit)
					_tcscpy(BufferUnit, _T("h"));
				else
					_tcscpy(BufferUnit, _T(""));
				showunit=true;
                        } else {
                                _stprintf(BufferValue, TEXT("--:--"));
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
		CropString(BufferTitle, 7);
		break;
	case BM_SYS:
		showunit=true;
			_tcscpy(BufferUnit, TEXT(""));
			if (SIMMODE) {
				// LKTOKEN _@M1199_ "Sat"
				_tcscpy(BufferTitle, MsgToken(1199));
				_stprintf(BufferValue,TEXT("SIM"));
			} else {
				Value=DrawInfo.SatellitesUsed;
				if (Value<1 || Value>30) {
					_stprintf(BufferValue,TEXT("---"));
				} else {
					_stprintf(BufferValue,TEXT("%d"),(int)Value);
				}
        
  			// LKTOKEN _@M1199_ "Sat"
				_stprintf(BufferTitle, TEXT("%s:?"), MsgToken(1199));
        for(const auto& dev : DeviceList) {
          if(dev.nmeaParser.activeGPS) {
					// LKTOKEN _@M1199_ "Sat"
            _stprintf(BufferTitle,_T("  (%s:%c)"), MsgToken(1199), _T('A')+dev.PortNumber);
            break; // we have got the first active port.
          }
        }        
			}
		break;
	case BM_CUS2:
		index=GetInfoboxIndex(3,MapWindow::Mode::MODE_FLY_CRUISE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;

	case BM_CUS3:
		index=GetInfoboxIndex(3,MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;
	case BM_CUS:
		index=GetInfoboxType(3);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;

	default:
		showunit=LKFormatValue(LK_ERROR, true, BufferValue, BufferUnit, BufferTitle);
		break;
  }
  if (ScreenLandscape|| userow2) {
      #include "LKMW3include_navbox1.cpp"
  } else {
      #include "LKMW3include_navbox2.cpp"
  }
  LKWriteText(Surface, BufferTitle, rcx+NIBLSCALE(7), rcy, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);

  /*
   *   FOURTH VALUE
   */

  if (bbsplitter<4) {
      if (ScreenLandscape) goto EndOfNavboxes;
      if (++userow2 >bbsplitter) goto EndOfNavboxes;
      rcx=rc.left+(splitoffset2/2)+((userow2-1)*splitoffset2);;
  }
  else {
      rcx+=splitoffset;
  }
  if (ScreenLandscape && (bbsplitter<4)) goto EndOfNavboxes;
  showunit=true;
  switch(BottomMode) {
	case BM_TRM:
		index=GetInfoboxIndex(4,MapWindow::Mode::MODE_FLY_CIRCLING);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
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
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
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
			_tcscpy(BufferTitle,_T("<<<"));
		} else {
			showunit=LKFormatValue(LK_BESTALTERN_ARRIV, true, BufferValue, BufferUnit, BufferTitle); // 100221
			_tcscpy(BufferTitle,_T(""));
		}
		break;
	case BM_SYS:
		// LKTOKEN _@M1068_ "HBAR"
		_tcscpy(BufferTitle, MsgToken(1068));
		if (DrawInfo.BaroAltitudeAvailable) {
			if (EnableNavBaroAltitude)
				// LKTOKEN _@M894_ "ON"
				_tcscpy(BufferValue,MsgToken(894));
			else
				// LKTOKEN _@M491_ "OFF"
				_tcscpy(BufferValue,MsgToken(491));
		} else
			_stprintf(BufferValue,TEXT("---"));
		showunit=false;
		break;
	case BM_CUS2:
		index=GetInfoboxIndex(4,MapWindow::Mode::MODE_FLY_CRUISE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;

	case BM_CUS3:
		index=GetInfoboxIndex(4,MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;
	case BM_CUS:
		index=GetInfoboxType(4);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;

	default:
		showunit=LKFormatValue(LK_ERROR, true, BufferValue, BufferUnit, BufferTitle);
		break;
  }

  if (ScreenLandscape|| userow2) {
      #include "LKMW3include_navbox1.cpp"
  } else {
      #include "LKMW3include_navbox2.cpp"
  }
  LKWriteText(Surface, BufferTitle, rcx+NIBLSCALE(7), rcy, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);

  /*
   *   FIFTH VALUE
   */
  if (bbsplitter<5) {
      if (ScreenLandscape) goto EndOfNavboxes;
      if (++userow2 >bbsplitter) goto EndOfNavboxes;
      rcx=rc.left+(splitoffset2/2)+((userow2-1)*splitoffset2);;
  }
  else {
      rcx+=splitoffset;
  }
  showunit=true;
  switch(BottomMode) {
	case BM_TRM:
		index=GetInfoboxIndex(5,MapWindow::Mode::MODE_FLY_CIRCLING);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;
	case BM_CRU:
		if (ISCAR) {
			_tcscpy(BufferTitle,MsgToken(2091)); // Averag (speed average really)
			int totime=(int)(Trip_Steady_Time+Trip_Moving_Time);
			if (totime>0) {
				_stprintf(BufferValue,_T("%.1f"),(DerivedDrawInfo.Odometer*SPEEDMODIFY)/totime);
				_stprintf(BufferUnit, TEXT("%s"),(Units::GetHorizontalSpeedName()));
			} else {
				_stprintf(BufferValue, TEXT("---"));
				showunit=false;
			}
		} else
			showunit=LKFormatValue(LK_HEADWINDSPEED, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_HGH:
                if (AltitudeUnit_Config==1) // meters
                   showunit=LKFormatValue(LK_AQNH, true, BufferValue, BufferUnit, BufferTitle);
                else
		   showunit=LKFormatValue(LK_FL, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_AUX:
		if (ISCAR) {
			if (Rotary_Distance<100000) {
				_stprintf(BufferValue,_T("%.2f"),DISTANCEMODIFY*Rotary_Distance);
			} else {
				_stprintf(BufferValue,_T("%.2f"),0.0);
			}
			_stprintf(BufferTitle,_T("AvgDist"));
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetDistanceName()));
			showunit=true;
		} else {
			showunit=LKFormatValue(LK_MAXALT, true, BufferValue, BufferUnit, BufferTitle);
		}
		break;
	case BM_TSK:
// TODO MAKE IT LKPROCESS
		Value=ALTITUDEMODIFY*DerivedDrawInfo.TaskStartAltitude;
		if (Value>0) {
			_stprintf(BufferValue,TEXT("%d"),(int)Value);
			_stprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
		} else {
			_tcscpy(BufferValue, TEXT("---"));
			_tcscpy(BufferUnit, TEXT(""));
			showunit=false;
		}
		// LKTOKEN _@M1200_ "Start"
		_tcscpy(BufferTitle, MsgToken(1200));
		break;
	case BM_ALT:
		if (ScreenLandscape) {
			showunit=LKFormatValue(LK_ALTERN2_GR, true, BufferValue, BufferUnit, BufferTitle); // 100221
			CropString(BufferTitle, 7);
		} else {
			showunit=LKFormatValue(LK_ALTERN1_ARRIV, false, BufferValue, BufferUnit, BufferTitle); // 100221
			_tcscpy(BufferTitle,_T(""));
		}
		break;
	case BM_SYS:
		cpusummary=CpuSummary();
		_tcscpy(BufferTitle,_T("CPU"));

		if (cpusummary >= 0 && cpusummary <= 100) {
		    showunit=true;
		    _stprintf(BufferValue,_T("%d"),cpusummary);
		    _tcscpy(BufferUnit,_T("%"));
		} else {
		    showunit=false;
		    _tcscpy(BufferValue, TEXT("---"));
		}
		break;
	case BM_CUS2:
		index=GetInfoboxIndex(5,MapWindow::Mode::MODE_FLY_CRUISE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;

	case BM_CUS3:
		index=GetInfoboxIndex(5,MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;
	case BM_CUS:
		index=GetInfoboxType(5);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;

	default:
		showunit=LKFormatValue(LK_ERROR, true, BufferValue, BufferUnit, BufferTitle);
		break;
  }

  if (ScreenLandscape|| userow2) {
      #include "LKMW3include_navbox1.cpp"
  } else {
      #include "LKMW3include_navbox2.cpp"
  }

  LKWriteText(Surface, BufferTitle, rcx+NIBLSCALE(3), rcy, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);

  /*
   *   SIXTH VALUE
   */
  if (bbsplitter<6) {
      if (ScreenLandscape) goto EndOfNavboxes;
      if (++userow2 >bbsplitter) goto EndOfNavboxes;
      rcx=rc.left+(splitoffset2/2)+((userow2-1)*splitoffset2);;
  }
  else {
      rcx+=splitoffset;
  }
  showunit=true;
  switch(BottomMode) {
	case BM_TRM:
		index=GetInfoboxIndex(6,MapWindow::Mode::MODE_FLY_CIRCLING);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;
	case BM_CRU:
		if (ISCAR)
			showunit=LKFormatValue(LK_TIME_LOCAL, false, BufferValue, BufferUnit, BufferTitle);
		else
			showunit=LKFormatValue(LK_LD_INST, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_HGH:
                if (AltitudeUnit_Config==1) // meters
		   showunit=LKFormatValue(LK_FL, true, BufferValue, BufferUnit, BufferTitle);
                else
                   showunit=LKFormatValue(LK_AQNH, true, BufferValue, BufferUnit, BufferTitle);
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
			_tcscpy(BufferTitle,_T("<<<"));
		else
			_tcscpy(BufferTitle,_T(""));
		break;
	case BM_SYS:
		showunit=LKFormatValue(LK_LOGGER, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_CUS2:
		index=GetInfoboxIndex(6,MapWindow::Mode::MODE_FLY_CRUISE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;

	case BM_CUS3:
		index=GetInfoboxIndex(6,MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;
	case BM_CUS:
		index=GetInfoboxType(6);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;

	default:
		showunit=LKFormatValue(LK_ERROR, true, BufferValue, BufferUnit, BufferTitle);
		break;
  }

  if (ScreenLandscape|| userow2) {
      #include "LKMW3include_navbox1.cpp"
  } else {
      #include "LKMW3include_navbox2.cpp"
  }
  LKWriteText(Surface, BufferTitle, rcx+NIBLSCALE(3), rcy, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);


  /*
   *   SEVENTH VALUE
   */
  if (bbsplitter<7) {
      if (ScreenLandscape) goto EndOfNavboxes;
      if (++userow2 >bbsplitter) goto EndOfNavboxes;
      rcx=rc.left+(splitoffset2/2)+((userow2-1)*splitoffset2);
  }
  else {
      rcx+=splitoffset;
  }
  showunit=true;
  switch(BottomMode) {
	case BM_TRM:
		index=GetInfoboxIndex(7,MapWindow::Mode::MODE_FLY_CIRCLING);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;
	case BM_CRU:
		if (ISCAR)
			showunit=LKFormatValue(LK_TIME_LOCAL, false, BufferValue, BufferUnit, BufferTitle); //QUI
		else
			showunit=LKFormatValue(LK_LD_AVR, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_HGH:
		showunit=LKFormatValue(LK_HGND, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_AUX:
		if (ISCAR)
			showunit=LKFormatValue(LK_MAXALT, true, BufferValue, BufferUnit, BufferTitle); // QUI
		else
			showunit=LKFormatValue(LK_PRCCLIMB, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_TSK:
		showunit=LKFormatValue(LK_FIN_GR, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_ALT:
		showunit=LKFormatValue(LK_HOME_ARRIVAL, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_SYS:
		showunit=LKFormatValue(LK_EXTBATT2VOLT, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_CUS2:
		index=GetInfoboxIndex(7,MapWindow::Mode::MODE_FLY_CRUISE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;

	case BM_CUS3:
		index=GetInfoboxIndex(7,MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;
	case BM_CUS:
		index=GetInfoboxType(7);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;

	default:
		showunit=LKFormatValue(LK_ERROR, true, BufferValue, BufferUnit, BufferTitle);
		break;
  }

  if (ScreenLandscape|| userow2) {
      #include "LKMW3include_navbox1.cpp"
  } else {
      #include "LKMW3include_navbox2.cpp"
  }
  LKWriteText(Surface, BufferTitle, rcx+NIBLSCALE(3), rcy, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);


  /*
   *   EIGTH VALUE
   */
  if (bbsplitter<8) {
      if (ScreenLandscape) goto EndOfNavboxes;
      if (++userow2 >bbsplitter) goto EndOfNavboxes;
      rcx=rc.left+(splitoffset2/2)+((userow2-1)*splitoffset2);
  }
  else {
      rcx+=splitoffset;
  }
  showunit=true;
  switch(BottomMode) {
	case BM_TRM:
		index=GetInfoboxIndex(8,MapWindow::Mode::MODE_FLY_CIRCLING);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;
	case BM_CRU:
		if (ISCAR)
			showunit=LKFormatValue(LK_TIME_LOCAL, false, BufferValue, BufferUnit, BufferTitle); //QUI
		else
			showunit=LKFormatValue(LK_LD_CRUISE, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_HGH:
		showunit=LKFormatValue(LK_AALTAGL, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_AUX:
		showunit=LKFormatValue(LK_MAXHGAINED, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_TSK:
		showunit=LKFormatValue(LK_SPEEDTASK_AVG, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_ALT:
		showunit=LKFormatValue(LK_HOMERADIAL, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;
	case BM_SYS:
		showunit=LKFormatValue(LK_EXTBATTBANK, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;
	case BM_CUS2:
		index=GetInfoboxIndex(8,MapWindow::Mode::MODE_FLY_CRUISE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;

	case BM_CUS3:
		index=GetInfoboxIndex(8,MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;
	case BM_CUS:
		index=GetInfoboxType(8);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		CropString(BufferTitle, 7);
		break;

	default:
		showunit=LKFormatValue(LK_ERROR, true, BufferValue, BufferUnit, BufferTitle);
		break;
  }

  if (ScreenLandscape|| userow2) {
      #include "LKMW3include_navbox1.cpp"
  } else {
      #include "LKMW3include_navbox2.cpp"
  }
  LKWriteText(Surface, BufferTitle, rcx+NIBLSCALE(3), rcy, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);

  /*
   *   NINTH VALUE
   */
  if (bbsplitter<9) {
      if (ScreenLandscape) goto EndOfNavboxes;
      if (++userow2 >bbsplitter) goto EndOfNavboxes;
      rcx=rc.left+(splitoffset2/2)+((userow2-1)*splitoffset2);
  }
  else {
      rcx+=splitoffset;
  }
  showunit=true;
  showunit=LKFormatValue(LK_IAS, true, BufferValue, BufferUnit, BufferTitle);
  if (ScreenLandscape|| userow2) {
      #include "LKMW3include_navbox1.cpp"
  } else {
      #include "LKMW3include_navbox2.cpp"
  }
  LKWriteText(Surface, BufferTitle, rcx+NIBLSCALE(3), rcy, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);

  /*
   *   TENTH VALUE
   */
  if (bbsplitter<9) {
      if (ScreenLandscape) goto EndOfNavboxes;
      if (++userow2 >bbsplitter) goto EndOfNavboxes;
      rcx=rc.left+(splitoffset2/2)+((userow2-1)*splitoffset2);
  }
  else {
      rcx+=splitoffset;
  }
  showunit=true;
  showunit=LKFormatValue(LK_SPEED_MC, true, BufferValue, BufferUnit, BufferTitle);
  if (ScreenLandscape|| userow2) {
      #include "LKMW3include_navbox1.cpp"
  } else {
      #include "LKMW3include_navbox2.cpp"
  }
  LKWriteText(Surface, BufferTitle, rcx+NIBLSCALE(3), rcy, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);

  /*
   *    CLEAN UP
   */

EndOfNavboxes:
  ;


  // restore objects here, before returning


}
