/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"
#include "LKInterface.h"
#include "Multimap.h"
#include "Asset.hpp"
#include "OS/RotateScreen.h"
#include "Baro.h"
#ifdef KOBO
#include "Kobo/System.hpp"
#endif
#include <algorithm>
#include "Calc/Task/TimeGates.h"

extern bool HaveGauges(void);

static void ReplaceInString(TCHAR* String, const TCHAR* ToReplace, const TCHAR* ReplaceWith, size_t Size) {
  tstring str(String);
  tstring_view toReplace(ToReplace);
  tstring_view replaceWith(ReplaceWith);

  size_t pos = 0;
  while ((pos = str.find(toReplace, pos)) != tstring::npos) {
    str.replace(pos, toReplace.size(), replaceWith);
    pos += replaceWith.size();
  }

  lk::strcpy(String, str.c_str(), Size);
}

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
using namespace std::string_view_literals;

TEST_CASE("ReplaceInString") {

  SUBCASE("overflow buffer") {
    TCHAR pBuffer[23];
    _tcscpy(pBuffer, _T("This is an old string."));
    ReplaceInString(pBuffer, _T("an old"), _T("a to long"), std::size(pBuffer));
    CHECK(pBuffer == _T("This is a to long stri"sv));
  }
  SUBCASE("overflow buffer (inside replaced value") {
    TCHAR pBuffer[31];
    _tcscpy(pBuffer, _T("This is an old string. an old"));
    ReplaceInString(pBuffer, _T("an old"), _T("a longer"), std::size(pBuffer));
    CHECK(pBuffer == _T("This is a longer string. a lon"sv));
  }

  SUBCASE(R"(Replace "old" with "new")") {
    TCHAR pBuffer[100];
    _tcscpy(pBuffer, _T("This is an old string."));
    ReplaceInString(pBuffer, _T("old"), _T("new"), std::size(pBuffer));
    CHECK(pBuffer == _T("This is an new string."sv));
  }

  SUBCASE("No replacement") {
    TCHAR pBuffer[100];
    _tcscpy(pBuffer, _T("This is a test string."));
    ReplaceInString(pBuffer, _T("old"), _T("new"), std::size(pBuffer));
    CHECK(pBuffer == _T("This is a test string."sv));
  }

  SUBCASE("Multiple replacements") {
    TCHAR pBuffer[100];
    _tcscpy(pBuffer, _T("a old b old c old"));
    ReplaceInString(pBuffer, _T("old"), _T("new"), std::size(pBuffer));
    CHECK(pBuffer == _T("a new b new c new"sv));
  }

  SUBCASE("Empty string") {
    TCHAR pBuffer[100];
    _tcscpy(pBuffer, _T(""));
    ReplaceInString(pBuffer, _T("old"), _T("new"), std::size(pBuffer));
    CHECK(pBuffer == _T(""sv));
  }

  SUBCASE("replace short string by longer one") {
    TCHAR pBuffer[100];
    _tcscpy(pBuffer, _T("aa bb cc dd ee ff gg"));
    ReplaceInString(pBuffer, _T("dd"), _T("dddd"), std::size(pBuffer));
    CHECK(pBuffer == _T("aa bb cc dddd ee ff gg"sv));
  }

  SUBCASE("replace long string by shorter one") {
    TCHAR pBuffer[100];
    _tcscpy(pBuffer, _T("aa bb cc dd ee ff gg"));
    ReplaceInString(pBuffer, _T("dd"), _T("d"), std::size(pBuffer));
    CHECK(pBuffer == _T("aa bb cc d ee ff gg"sv));
  }

  SUBCASE("replace long string by empty one") {
    TCHAR pBuffer[100];
    _tcscpy(pBuffer, _T("aa bb cc dd ee ff gg"));
    ReplaceInString(pBuffer, _T("dd "), _T(""), std::size(pBuffer));
    CHECK(pBuffer == _T("aa bb cc ee ff gg"sv));
  }


}

#endif  // DOCTEST_CONFIG_DISABLE

static void CondReplaceInString(bool Condition, TCHAR *Buffer,
                                const TCHAR *Macro, const TCHAR *TrueText,
                                const TCHAR *FalseText, size_t Size){
  ReplaceInString(Buffer, Macro, Condition ? TrueText : FalseText, Size);
}

//
// TODO CHECK BUFFER SIZE, which is 100 (set in Buttons).
//
bool ExpandMacros(const TCHAR *In, TCHAR *OutBuffer, size_t Size){

  TCHAR *a;
  LK_tcsncpy(OutBuffer, In, Size - 1);

  if (_tcsstr(OutBuffer, TEXT("$(")) == NULL) {
	return false;
  }

  short items=1;
  bool invalid = false;

  // Accelerator for entire label replacement- only one macro per label accepted
  a =_tcsstr(OutBuffer, TEXT("$(AC"));
  if (a != NULL) {
	TCHAR tbuf[20];
	short i;
	i= (*(a+4)-'0')*10;
	i+= *(a+5)-'0';
	LKASSERT(i>=0 && i<42);

	switch(i) {
		case 0:	// LOCKMODE
			lk::strcpy(OutBuffer, _T(""), Size);	// default is button invisible
			if (LockMode(0)) {	// query availability
				if (LockMode(1)) // query status
					lk::strcpy(OutBuffer,  MsgToken<965>(), Size); // UNLOCK\nSCREEN
				else
					lk::strcpy(OutBuffer, MsgToken<966>(), Size); // LOCK\nSCREEN

				if (!LockMode(3)) invalid=true; // button not usable
			}
			break;

		case 1:	// Pan Supertoggle  PanModeStatus  mode=pan location=8
			if ( MapWindow::mode.AnyPan() )
				_stprintf(OutBuffer, _T("%s%s"),MsgToken<2004>(),MsgToken<491>());	// OFF
			else
				_stprintf(OutBuffer, _T("%s%s"),MsgToken<2004>(),MsgToken<894>());	// ON
			break;

		case 2:	// Pan Supertoggle  PanModeStatus  mode=ScreenMode location=8
			if ( MapWindow::mode.AnyPan() )
				_stprintf(OutBuffer, _T("%s\n%s"),MsgToken<2082>(),MsgToken<491>());	// OFF
			else
				_stprintf(OutBuffer, _T("%s\n%s"),MsgToken<2082>(),MsgToken<894>());	// ON
			break;

		case 3: // DISABLED
			lk::strcpy(OutBuffer, MsgToken<2023>(), Size); // Reserved
			invalid=true;
			break;

		case 4: // MacCreadyValue + 2078
			_stprintf(tbuf,_T("%2.1lf"), iround(Units::ToVerticalSpeed(MACCREADY*10))/10.0);
			_stprintf(OutBuffer, _T("%s\n%s"), MsgToken<2078>(), tbuf);
			break;

		case 5:
			if (CALCULATED_INFO.AutoMacCready)  {
				switch(AutoMcMode) {
					case amcFinalGlide:
						_stprintf(tbuf,_T("%s"), MsgToken<1681>());
						break;
					case amcAverageClimb:
						_stprintf(tbuf,_T("%s"), MsgToken<1682>());
						break;
					case amcEquivalent:
						_stprintf(tbuf,_T("%s"), MsgToken<1683>());
						break;
					case amcFinalAndClimb:
						if (CALCULATED_INFO.FinalGlide)
							_stprintf(tbuf,_T("%s"), MsgToken<1681>());
						else
							_stprintf(tbuf,_T("%s"), MsgToken<1682>());
						break;
					default:
						// LKTOKEN _@M1202_ "Auto"
						_stprintf(tbuf,_T("%s"), MsgToken<1202>());
						break;
				}
			} else {
				// LKTOKEN _@M1201_ "Man"
				_stprintf(tbuf,_T("%s"), MsgToken<1201>());
			}
			_stprintf(OutBuffer,_T("Mc %s\n%2.1lf"), tbuf,iround(Units::ToVerticalSpeed(MACCREADY*10))/10.0);

			break;

		case 6: // WaypointNext
			invalid = !ValidTaskPoint(ActiveTaskPoint+1);
			if (!ValidTaskPoint(ActiveTaskPoint+2))
				lk::strcpy(OutBuffer, MsgToken<801>(), Size); // Waypoint Finish
			else
				lk::strcpy(OutBuffer, MsgToken<802>(), Size); // Waypoint Next
			break;

		case 7: // WaypointPrevious

			if (ActiveTaskPoint==1) {
				invalid = !ValidTaskPoint(ActiveTaskPoint-1);
				lk::strcpy(OutBuffer, MsgToken<804>(), Size); // Waypoint Start
			} else if (EnableMultipleStartPoints) {
				invalid = !ValidTaskPoint(0);

				if (ActiveTaskPoint==0)
					lk::strcpy(OutBuffer, _T("StartPnt\nCycle"), Size);
				else
					lk::strcpy(OutBuffer, MsgToken<803>(), Size); // Waypoint Previous
			} else {
				invalid = (ActiveTaskPoint<=0);
				lk::strcpy(OutBuffer, MsgToken<803>(), Size); // Waypoint Previous
			}
			break;

		case 8: // RealTask  check for Task reset

			if (! (ValidTaskPoint(ActiveTaskPoint) && ValidTaskPoint(1))) {
				invalid=true;
			}

			lk::strcpy(OutBuffer, MsgToken<2019>(), Size); // Task reset
			break;

		case 9: // TerrainVisible for ChangeBack topology color
			if (CALCULATED_INFO.TerrainValid && IsMultimapTerrain() && !LKVarioBar) {
				invalid = true;
			}
			lk::strcpy(OutBuffer, MsgToken<2037>(), Size); // Change topo back
			break;

		case 10: // TOGGLEHBAR HBARAVAILABLE for Toggle HBAR button

			if (!BaroAltitudeAvailable(GPS_INFO)) {
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2045>(),MsgToken<1068>()); // Nav by HBAR
				invalid=true;
			} else {
				if (EnableNavBaroAltitude)
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2045>(),MsgToken<1174>()); // Nav by HGPS
				else
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2045>(),MsgToken<1068>()); // Nav by HBAR
			}
			break;

		case 11: // SIM MENU SIMONLY
			if (SIMMODE)
				lk::strcpy(OutBuffer, MsgToken<2074>(), Size); // SIM MENU
			else
				lk::strcpy(OutBuffer, _T(""), Size);
			break;

		case 12: // THIS MACRO IS AVAILABLE FOR USE
			break;

		case 13:
			if(UseTotalEnergy) {
                           _stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2115>(),MsgToken<491>()); // TE OFF
                        } else {
                           _stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2115>(),MsgToken<894>()); // TE ON
			}
			break;

		case 14:
			if ( Shading )
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2080>(),MsgToken<491>()); // OFF
			else
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2080>(),MsgToken<894>()); // ON
			break;

		case 15:
			if (EnableSoundModes)
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2055>(),MsgToken<491>()); // OFF
			else
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2055>(),MsgToken<894>()); // ON
			break;

		case 16: // ActiveMap no more used now Radio Button
			if (RadioPara.Enabled) {
			   _stprintf(OutBuffer,_T("%s\n"),MsgToken<2306>()); // TEXT
			   invalid=false;
			} else {
			  _stprintf(OutBuffer,_T("%s\n"),MsgToken<2306>()); // TEXT
			  invalid=true;
			}
			break;
		case 17:
			// Order is:  ALL ON, TEXT ONLY, GAUGES ONLY, ALL OFF
			if (!HaveGauges()) {
				if (!IsMultimapOverlaysText()) {
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2079>(),MsgToken<2234>()); // TEXT
				} else {
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2079>(),MsgToken<491>()); // OFF
				}
				break;
			}
			if (!IsMultimapOverlaysText()&&!IsMultimapOverlaysGauges()) {
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2079>(),MsgToken<899>()); // ALL ON
			} else {
				if (IsMultimapOverlaysAll()) {
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2079>(),MsgToken<2234>()); // TEXT
				} else {
					if (IsMultimapOverlaysText())
						_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2079>(),MsgToken<2235>()); // GAUGES
					else
						_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2079>(),MsgToken<898>()); // ALL OFF
				}
			}
			break;

		case 18:
			if (Orbiter)
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2065>(),MsgToken<491>()); // OFF
			else
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2065>(),MsgToken<894>()); // ON
			if (!EnableThermalLocator) invalid = true;
			break;

		case 19:
			if (IsMultimapAirspace())
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2029>(),MsgToken<491>()); // OFF
			else
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2029>(),MsgToken<894>()); // ON
			break;

		case 20:
			if (MapWindow::zoom.AutoZoom() )
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2009>(),MsgToken<418>());
			else
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2009>(),MsgToken<897>());
			break;

		case 21:
			if (IsMultimapTopology())
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2027>(),MsgToken<491>()); // OFF
			else
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2027>(),MsgToken<894>()); // ON
			break;

		case 22:
			if (IsMultimapTerrain())
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2028>(),MsgToken<491>()); // OFF
			else
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2028>(),MsgToken<894>()); // ON
			break;

		case 23:
			if (MapSpaceMode!=MSM_MAP) invalid=true;
			if (MapWindow::mode.UserForcedMode() == MapWindow::Mode::MODE_FLY_CIRCLING)
				_stprintf(OutBuffer,_T("DspMode\n_%s_"),MsgToken<2031>());
			else
				_stprintf(OutBuffer,_T("DspMode\n%s"),MsgToken<2031>());
			break;

		case 24:
			if (MapSpaceMode!=MSM_MAP) invalid=true;
			if (MapWindow::mode.UserForcedMode() == MapWindow::Mode::MODE_FLY_CRUISE)
				_stprintf(OutBuffer,_T("DspMode\n_%s_"),MsgToken<2032>());
			else
				_stprintf(OutBuffer,_T("DspMode\n%s"),MsgToken<2032>());
			break;
		case 25:
			if (MapSpaceMode!=MSM_MAP) invalid=true;
			if (MapWindow::mode.UserForcedMode() == MapWindow::Mode::MODE_FLY_NONE)
				_stprintf(OutBuffer,_T("DspMode\n_%s_"),MsgToken<2034>());
			else
				_stprintf(OutBuffer,_T("DspMode\n%s"),MsgToken<2034>());
			break;
		case 26:
			if (MapSpaceMode!=MSM_MAP) invalid=true;
			if (MapWindow::mode.UserForcedMode() == MapWindow::Mode::MODE_FLY_FINAL_GLIDE)
				_stprintf(OutBuffer,_T("DspMode\n_%s_"),MsgToken<2033>());
			else
				_stprintf(OutBuffer,_T("DspMode\n%s"),MsgToken<2033>());
			break;

		case 27: // amcIsBoth
			if (CALCULATED_INFO.AutoMacCready && AutoMcMode==amcFinalAndClimb)
				_stprintf(OutBuffer,_T("Auto\n_%s_"),MsgToken<2117>());
			else
				_stprintf(OutBuffer,_T("Auto\n%s"),MsgToken<2117>());
			break;
		case 28: // amcIsFinal
			if (CALCULATED_INFO.AutoMacCready && AutoMcMode==amcFinalGlide)
				_stprintf(OutBuffer,_T("Auto\n_%s_"),MsgToken<2033>());
			else
				_stprintf(OutBuffer,_T("Auto\n%s"),MsgToken<2033>());
			break;
		case 29: // amcIsClimb
			if (CALCULATED_INFO.AutoMacCready && AutoMcMode==amcAverageClimb)
				_stprintf(OutBuffer,_T("Auto\n_%s_"),MsgToken<2075>());
			else
				_stprintf(OutBuffer,_T("Auto\n%s"),MsgToken<2075>());
			break;
		case 30: // amcIsEquiv
			if (CALCULATED_INFO.AutoMacCready && AutoMcMode==amcEquivalent)
				_stprintf(OutBuffer,_T("Auto\n_%s_"),MsgToken<2076>());
			else
				_stprintf(OutBuffer,_T("Auto\n%s"),MsgToken<2076>());
			break;
		case 31: // CheckManMc
			if (CALCULATED_INFO.AutoMacCready)
				_stprintf(OutBuffer,_T("Mc\n%s"),MsgToken<2077>());
			else
				_stprintf(OutBuffer,_T("Mc\n_%s_"),MsgToken<2077>());
			break;

		case 32: // AirspaceMode
			switch(AltitudeMode) {
				case 0:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2029>(),MsgToken<184>()); // Clip
					break;
				case 1:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2029>(),MsgToken<897>()); // Auto
					break;
				case 2:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2029>(),MsgToken<139>()); // Below
					break;
				case 3:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2029>(),MsgToken<359>()); // Inside
					break;
				case 4:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2029>(),MsgToken<75>()); // All Off
					break;
				case 5:
				default:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2029>(),MsgToken<76>()); // All On
					break;
			}
			break;

		case 33: // SnailTrailToggleName
			if (MapSpaceMode!=MSM_MAP) invalid=true;
                        // Since we show the next choice, but the order is not respected in 5.0:
                        // the new order is artificially off-short-long-full, as in the inputevents button management
			switch(TrailActive) {
				case 0: // off to short
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2035>(),MsgToken<612>()); // Short
					break;
				case 1: // long to full
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2035>(),MsgToken<312>()); // Full
					break;
				case 2: // short to long
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2035>(),MsgToken<410>()); // Long
					break;
				case 3: // full to off
				default:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2035>(),MsgToken<491>()); // OFF
					break;
			}
			break;

		case 34: // MapLabelsToggleActionName
			switch(GetMultimap_Labels()) {
				case MAPLABELS_ALLON:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2026>(),MsgToken<1203>()); // WPTS
					break;
				case MAPLABELS_ONLYWPS:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2026>(),MsgToken<1204>()); // TOPO
					break;
				case MAPLABELS_ONLYTOPO:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2026>(),MsgToken<898>());
					break;
				case MAPLABELS_ALLOFF:
				default:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken<2026>(),MsgToken<899>());
					break;
			}
			break;

		case 35: // SIM PAN MODE REPOSITION, PANREPOS
			if (SIMMODE)
				lk::strcpy(OutBuffer, MsgToken<2133>(), Size); // Position
			else
				lk::strcpy(OutBuffer, _T(""), Size);
			break;

		case 36: //
			// Order is:  ALL ON, TASK ONLY, FAI ONLY, ALL OFF
			if (Flags_DrawTask&&Flags_DrawFAI) {
				lk::strcpy(OutBuffer, MsgToken<2238>(), Size); // Draw Task
			} else {
				if (Flags_DrawTask&&!Flags_DrawFAI) {
					lk::strcpy(OutBuffer, MsgToken<2239>(), Size); // Draw FAI
				} else {
					if (!Flags_DrawTask&&Flags_DrawFAI) {
						lk::strcpy(OutBuffer, MsgToken<2240>(), Size); // NoDraw TaskFAI
					} else {
						lk::strcpy(OutBuffer, MsgToken<2241>(), Size); // Draw TaskFAI
					}
				}
			}
			break;
		case 37: //
			if (SonarWarning)
				lk::strcpy(OutBuffer, MsgToken<2243>(), Size); // Sonar OFF
			else
				lk::strcpy(OutBuffer, MsgToken<2242>(), Size); // Sonar ON
			break;
		case 38: //
			if (MapSpaceMode!=MSM_MAP) invalid=true;
			lk::strcpy(OutBuffer, MsgToken<2081>(), Size); // Set Map
			break;
		case 39:
			if (! (ValidTaskPoint(ActiveTaskPoint) && ValidTaskPoint(1))) {
				invalid=true;
			}

			lk::strcpy(OutBuffer, MsgToken<1850>(), Size); // Task reverse
			break;
        case 40:
            if(IsKobo()) {
#ifdef KOBO
                if(IsKoboWifiOn()) {
                    lk::strcpy(OutBuffer, _T("Wifi\nOff"), Size);
                } else {
                    lk::strcpy(OutBuffer, _T("Wifi\nOn"), Size);
                }
#endif
            } else {
              lk::strcpy(OutBuffer,  _T(""), Size);
            }
            break;
		case 41:
			if (Flags_DrawXC )
				_stprintf(OutBuffer,_T("%s"),MsgToken<2388>());  //  "Draw\nXC"
			else
				_stprintf(OutBuffer,_T("%s"),MsgToken<2389>());  //	"NoDraw\nXC"
		break;


		default:
			_stprintf(OutBuffer, _T("INVALID\n%d"),i);
			break;
	}
	goto label_ret;
  } // ACcelerator

  // No accelerator? First check if we have a second macro embedded in string

  a =_tcsstr(OutBuffer, TEXT("&("));
  if (a != NULL) {
	*a=_T('$');
	items=2;
  }

  // Then go for one-by-one match search, slow



  if (_tcsstr(OutBuffer, TEXT("$(AdvanceArmed)"))) {
    switch (AutoAdvance) {
    case 0:
      ReplaceInString(OutBuffer, TEXT("$(AdvanceArmed)"), MsgToken<892>(), Size); // (manual)
      invalid = true;
      break;
    case 1:
      ReplaceInString(OutBuffer, TEXT("$(AdvanceArmed)"), MsgToken<893>(), Size); // (auto)
      invalid = true;
      break;
    case 2:
      if (ActiveTaskPoint>0) {
        if (ValidTaskPoint(ActiveTaskPoint+1)) {
          CondReplaceInString(AdvanceArmed, OutBuffer, TEXT("$(AdvanceArmed)"),
		MsgToken<161>(),  // Cancel
		MsgToken<678>(), Size); // TURN
        } else {
          ReplaceInString(OutBuffer, TEXT("$(AdvanceArmed)"), MsgToken<8>(), Size); // (finish)
          invalid = true;
        }
      } else {
        CondReplaceInString(AdvanceArmed, OutBuffer, TEXT("$(AdvanceArmed)"),
		MsgToken<161>(),  // Cancel
		MsgToken<571>(), Size); // START
      }
      break;
    case 3:
      if (ActiveTaskPoint==0) {
        CondReplaceInString(AdvanceArmed, OutBuffer, TEXT("$(AdvanceArmed)"),
		MsgToken<161>(),  // Cancel
		MsgToken<571>(), Size); // START
      } else if (ActiveTaskPoint==1) {
        CondReplaceInString(AdvanceArmed, OutBuffer, TEXT("$(AdvanceArmed)"),
		MsgToken<161>(),  // Cancel
		MsgToken<539>(), Size); // RESTART
      } else {
        ReplaceInString(OutBuffer, TEXT("$(AdvanceArmed)"), MsgToken<893>(), Size); // (auto)
        invalid = true;
      }
      break;
      // TODO bug: no need to arm finish
    case 4:
      if (ActiveTaskPoint>0) {
        if (ValidTaskPoint(ActiveTaskPoint+1)) {
          CondReplaceInString(AdvanceArmed, OutBuffer, TEXT("$(AdvanceArmed)"),
		MsgToken<161>(),  // Cancel
		MsgToken<678>(), Size); // TURN
        } else {
          ReplaceInString(OutBuffer, TEXT("$(AdvanceArmed)"), MsgToken<8>(), Size); // (finish)
          invalid = true;
        }
      }
      else {
        ReplaceInString(OutBuffer, TEXT("$(AdvanceArmed)"), MsgToken<893>(), Size); // (auto)
        invalid = true;
      }
      break;
    default:
      break;
    }
	if (--items<=0) goto label_ret; // 100517
  }


  if (_tcsstr(OutBuffer, TEXT("$(CheckFlying)"))) {
    if (!CALCULATED_INFO.Flying) {
      invalid = true;
    }
    ReplaceInString(OutBuffer, TEXT("$(CheckFlying)"), TEXT(""), Size);
	if (--items<=0) goto label_ret;
  }

  if (_tcsstr(OutBuffer, TEXT("$(NotInReplay)"))) {
    if (ReplayLogger::IsEnabled()) {
      invalid = true;
    }
    ReplaceInString(OutBuffer, TEXT("$(NotInReplay)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }

  if (_tcsstr(OutBuffer, TEXT("$(CheckWaypointFile)"))) {
    if (!ValidWayPoint(NUMRESWP)) {
      invalid = true;
    }
    ReplaceInString(OutBuffer, TEXT("$(CheckWaypointFile)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }
  if (_tcsstr(OutBuffer, TEXT("$(CheckSettingsLockout)"))) {
    if (LockSettingsInFlight && CALCULATED_INFO.Flying) {
      invalid = true;
    }
    ReplaceInString(OutBuffer, TEXT("$(CheckSettingsLockout)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }
  if (_tcsstr(OutBuffer, TEXT("$(CheckTask)"))) {
    if (!ValidTaskPoint(ActiveTaskPoint)) {
      invalid = true;
    }
    ReplaceInString(OutBuffer, TEXT("$(CheckTask)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }
  if (_tcsstr(OutBuffer, TEXT("$(CheckAirspace)"))) {
	if (!CAirspaceManager::Instance().ValidAirspaces()) {
      invalid = true;
    }
    ReplaceInString(OutBuffer, TEXT("$(CheckAirspace)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }
  if (_tcsstr(OutBuffer, TEXT("$(CheckFLARM)"))) {
    if (!GPS_INFO.FLARM_Available) {
      invalid = true;
    }
    ReplaceInString(OutBuffer, TEXT("$(CheckFLARM)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }



  // If it is not SIM mode, it is invalid
  if (_tcsstr(OutBuffer, TEXT("$(OnlyInSim)"))) {
	if (!SIMMODE) invalid = true;
	ReplaceInString(OutBuffer, TEXT("$(OnlyInSim)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }
  if (_tcsstr(OutBuffer, TEXT("$(OnlyInFly)"))) {
	#if TESTBENCH
	invalid=false;
	#else
	if (SIMMODE) invalid = true;
	#endif
	ReplaceInString(OutBuffer, TEXT("$(OnlyInFly)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }


  if (_tcsstr(OutBuffer, TEXT("$(WCSpeed)"))) {
	TCHAR tbuf[10];
	_stprintf(tbuf,_T("%.0f%s"), Units::ToHorizontalSpeed(WindCalcSpeed), Units::GetHorizontalSpeedName());
	ReplaceInString(OutBuffer, TEXT("$(WCSpeed)"), tbuf, Size);
	if (--items<=0) goto label_ret; // 100517
  }

  if (_tcsstr(OutBuffer, TEXT("$(GS"))) {
	TCHAR tbuf[10];
	_stprintf(tbuf,_T("%.0f%s"), Units::ToHorizontalSpeed(GPS_INFO.Speed), Units::GetHorizontalSpeedName());
	ReplaceInString(OutBuffer, TEXT("$(GS)"), tbuf, Size);
	if (--items<=0) goto label_ret;
  }
  if (_tcsstr(OutBuffer, TEXT("$(HGPS"))) {
	TCHAR tbuf[10];
	_stprintf(tbuf,_T("%.0f%s"), Units::ToAltitude(GPS_INFO.Altitude), Units::GetAltitudeName());
	ReplaceInString(OutBuffer, TEXT("$(HGPS)"), tbuf, Size);
	if (--items<=0) goto label_ret;
  }
  if (_tcsstr(OutBuffer, TEXT("$(TURN"))) {
	TCHAR tbuf[10];
	_stprintf(tbuf,_T("%.0f"),SimTurn);
	ReplaceInString(OutBuffer, TEXT("$(TURN)"), tbuf, Size);
	if (--items<=0) goto label_ret;
  }
  if (_tcsstr(OutBuffer, TEXT("$(NETTO"))) {
	TCHAR tbuf[10];
	_stprintf(tbuf,_T("%.1f"),SimNettoVario);
	ReplaceInString(OutBuffer, TEXT("$(NETTO)"), tbuf, Size);
	if (--items<=0) goto label_ret;
  }


  if (_tcsstr(OutBuffer, TEXT("$(LoggerActive)"))) {
	CondReplaceInString(LoggerActive, OutBuffer, TEXT("$(LoggerActive)"), MsgToken<670>(), MsgToken<657>(), Size); // Stop Start
	if (--items<=0) goto label_ret; // 100517
  }


  if (_tcsstr(OutBuffer, TEXT("$(NoSmart)"))) {
	if (DisplayOrientation == NORTHSMART) invalid = true;
	ReplaceInString(OutBuffer, TEXT("$(NoSmart)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }


  if (_tcsstr(OutBuffer, TEXT("$(FinalForceToggleActionName)"))) {
    CondReplaceInString(ForceFinalGlide, OutBuffer,
                        TEXT("$(FinalForceToggleActionName)"),
                        MsgToken<896>(), // Unforce
                        MsgToken<895>(), // Force
			Size);
    if (AutoForceFinalGlide) {
      invalid = true;
    }
	if (--items<=0) goto label_ret; // 100517
  }



  if (_tcsstr(OutBuffer, TEXT("$(PCONLY)"))) {
      if(IsEmbedded()) {
        lk::strcpy(OutBuffer, _T(""), Size);
        invalid = true;
      } else {
        ReplaceInString(OutBuffer, TEXT("$(PCONLY)"), TEXT(""), Size);
      }
    if (--items<=0) goto label_ret;
  }
  if (_tcsstr(OutBuffer, TEXT("$(NOTPC)"))) {
      if(IsEmbedded()) {
        ReplaceInString(OutBuffer, TEXT("$(NOTPC)"), TEXT(""), Size);
      } else {
        lk::strcpy(OutBuffer, _T(""), Size);
        invalid = true;
      }
      if (--items<=0) goto label_ret;
  }

  if (_tcsstr(OutBuffer, TEXT("$(ONLYMAP)"))) {
    if (MapSpaceMode!=MSM_MAP) invalid=true;
    ReplaceInString(OutBuffer, TEXT("$(ONLYMAP)"), TEXT(""), Size);

    if (--items<=0) goto label_ret;
  }

  if (_tcsstr(OutBuffer, TEXT("$(SCREENROTATE)"))) {
      if(CanRotateScreen()) {
        ReplaceInString(OutBuffer, TEXT("$(SCREENROTATE)"), TEXT(""), Size);
      } else {
        lk::strcpy(OutBuffer, _T(""), Size);
        invalid = true;
      }
      if (--items<=0) goto label_ret;
  }

  if (_tcsstr(OutBuffer, TEXT("$(PilotEvent)"))) {
	if (PilotEventEnabled()) {
		ReplaceInString(OutBuffer, TEXT("$(PilotEvent)"), TEXT(""), Size);
	} else {
		lk::strcpy(OutBuffer, _T(""), Size);
		invalid = true;
	}
  }

  // We dont replace macro, we do replace the entire label
  a =_tcsstr(OutBuffer, TEXT("$(MM"));
  if (a != NULL) {
    short i = *(a+4)-48;
    if (i == 0) {
      i = 10;
    }
    LKASSERT(i> 0 && i <11);
    // get the label for the custom menu item here
    // Decide if invalid=true or if no label at all, setting Replace to empty string

    // test mode only
    CustomKeyMode_t key = CustomKeyFromMenu(i);
    if (key != CustomKeyMode_t::ckDisabled) {
      lk::strcpy(OutBuffer,  CustomKeyLabel(key), Size);
    } else {
      invalid = true;              // non selectable
      lk::strcpy(OutBuffer,  _T(""), Size);  // make it invisible
    }                              // MM
  }
label_ret:

  return invalid;
}
