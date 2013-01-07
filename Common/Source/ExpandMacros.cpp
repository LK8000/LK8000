/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"
#include "LKInterface.h"
#include "Multimap.h"

static void ReplaceInString(TCHAR *String, TCHAR *ToReplace, 
                            TCHAR *ReplaceWith, size_t Size){
  TCHAR TmpBuf[MAX_PATH];
  int   iR;
  TCHAR *pC;

  while((pC = _tcsstr(String, ToReplace)) != NULL){
    iR = _tcsclen(ToReplace);
    _tcscpy(TmpBuf, pC + iR);
    _tcscpy(pC, ReplaceWith);
    _tcscat(pC, TmpBuf);
  }

}

static void CondReplaceInString(bool Condition, TCHAR *Buffer, 
                                TCHAR *Macro, TCHAR *TrueText, 
                                TCHAR *FalseText, size_t Size){
  if (Condition)
    ReplaceInString(Buffer, Macro, TrueText, Size);
  else
    ReplaceInString(Buffer, Macro, FalseText, Size);
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
	i= (*(a+4)-48)*10;
	i+= *(a+5)-48;
	LKASSERT(i>=0 && i<40);

	switch(i) {
		case 0:	// LOCKMODE
			_tcscpy(OutBuffer,_T(""));	// default is button invisible
			if (LockMode(0)) {	// query availability
				if (LockMode(1)) // query status
					_tcscpy(OutBuffer,MsgToken(965)); // UNLOCK\nSCREEN
				else
					_tcscpy(OutBuffer,MsgToken(966)); // LOCK\nSCREEN

				if (!LockMode(3)) invalid=true; // button not usable
			}
			break;

		case 1:	// Pan Supertoggle  PanModeStatus  mode=pan location=8
			if ( MapWindow::mode.AnyPan() )
				_stprintf(OutBuffer, _T("%s%s"),MsgToken(2004),MsgToken(491));	// OFF
			else
				_stprintf(OutBuffer, _T("%s%s"),MsgToken(2004),MsgToken(894));	// ON
			break;

		case 2:	// Pan Supertoggle  PanModeStatus  mode=ScreenMode location=8
			if ( MapWindow::mode.AnyPan() )
				_stprintf(OutBuffer, _T("%s\n%s"),MsgToken(2082),MsgToken(491));	// OFF
			else
				_stprintf(OutBuffer, _T("%s\n%s"),MsgToken(2082),MsgToken(894));	// ON
			break;

		case 3: // DISABLED
			_tcscpy(OutBuffer,MsgToken(2023)); // Reserved
			invalid=true;
			break;

		case 4: // MacCreadyValue + 2078
			_stprintf(tbuf,_T("%2.1lf"), iround(LIFTMODIFY*MACCREADY*10)/10.0);
			_stprintf(OutBuffer, _T("%s\n%s"), MsgToken(2078), tbuf);
			break;

		case 5:
			if (CALCULATED_INFO.AutoMacCready)  {
				switch(AutoMcMode) {
					case amcFinalGlide:
						_stprintf(tbuf,_T("%s"), MsgToken(1681));
						break;
					case amcAverageClimb:
						_stprintf(tbuf,_T("%s"), MsgToken(1682));
						break;
					case amcEquivalent:
						_stprintf(tbuf,_T("%s"), MsgToken(1683));
						break;
					case amcFinalAndClimb:
						if (CALCULATED_INFO.FinalGlide)
							_stprintf(tbuf,_T("%s"), MsgToken(1681));
						else
							_stprintf(tbuf,_T("%s"), MsgToken(1682));
						break;
					default:
						// LKTOKEN _@M1202_ "Auto"
						_stprintf(tbuf,_T("%s"), MsgToken(1202));
						break;
				}
			} else {
				// LKTOKEN _@M1201_ "Man"
				_stprintf(tbuf,_T("%s"), MsgToken(1201));
			}
			_stprintf(OutBuffer,_T("Mc %s\n%2.1lf"), tbuf,iround(LIFTMODIFY*MACCREADY*10)/10.0);

			break;

		case 6: // WaypointNext
			invalid = !ValidTaskPoint(ActiveWayPoint+1);
			if (!ValidTaskPoint(ActiveWayPoint+2))
				_tcscpy(OutBuffer,MsgToken(801)); // Waypoint Finish
			else
				_tcscpy(OutBuffer,MsgToken(802)); // Waypoint Next
			break;

		case 7: // WaypointPrevious

			if (ActiveWayPoint==1) {
				invalid = !ValidTaskPoint(ActiveWayPoint-1);
				_tcscpy(OutBuffer,MsgToken(804)); // Waypoint Start
			} else if (EnableMultipleStartPoints) {
				invalid = !ValidTaskPoint(0);

				if (ActiveWayPoint==0)
					_tcscpy(OutBuffer,_T("StartPnt\nCycle"));
				else
					_tcscpy(OutBuffer,MsgToken(803)); // Waypoint Previous
			} else {
				invalid = (ActiveWayPoint<=0);
				_tcscpy(OutBuffer,MsgToken(803)); // Waypoint Previous
			}
			break;

		case 8: // RealTask  check for Task reset

			if (! (ValidTaskPoint(ActiveWayPoint) && ValidTaskPoint(1))) {
				invalid=true;
			}

			_tcscpy(OutBuffer,MsgToken(2019)); // Task reset
			break;

		case 9: // TerrainVisible for ChangeBack topology color
			if (CALCULATED_INFO.TerrainValid && IsMultimapTerrain() && !LKVarioBar) {
				invalid = true;
			}
			_tcscpy(OutBuffer,MsgToken(2037)); // Change topo back
			break;

		case 10: // TOGGLEHBAR HBARAVAILABLE for Toggle HBAR button

			if (!GPS_INFO.BaroAltitudeAvailable) {
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2045),MsgToken(1068)); // Nav by HBAR
				invalid=true;
			} else {
				if (EnableNavBaroAltitude)
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2045),MsgToken(1174)); // Nav by HGPS
				else
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2045),MsgToken(1068)); // Nav by HBAR
			}
			break;

		case 11: // SIM MENU SIMONLY
			if (SIMMODE)
				_tcscpy(OutBuffer,MsgToken(2074)); // SIM MENU
			else
				_tcscpy(OutBuffer,_T(""));
			break;

		case 12: // VisualGlideToggleName

			switch(VisualGlide) {
				case 0:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2030),MsgToken(894)); // VisualG ON
					break;
				case 1:
					if (ExtendedVisualGlide)
						_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2030),MsgToken(1205)); // VisualG moving
					else
						_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2030),MsgToken(491)); // VisualG OFF
					break;
				case 2:
				default:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2030),MsgToken(491)); // VisualG OFF
					break;
			}
			break;

		case 13:
			switch(UseTotalEnergy) {
				case 0:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2115),MsgToken(894)); // TE ON
					break;
				case 1:
				default:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2115),MsgToken(491)); // TE OFF
					break;
			}
			break;

		case 14:
			if ( Shading )
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2080),MsgToken(491)); // OFF
			else
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2080),MsgToken(894)); // ON
			break;
 
		case 15:
			if (EnableSoundModes)
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2055),MsgToken(491)); // OFF
			else
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2055),MsgToken(894)); // ON
			break;

		case 16:
			if (ActiveMap)
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2044),MsgToken(491)); // OFF
			else
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2044),MsgToken(894)); // ON

			if (MapSpaceMode!=MSM_MAP && MapSpaceMode!=MSM_MAPTRK && MapSpaceMode!=MSM_MAPWPT)
				invalid=true;
			break;

		case 17:
			// Order is:  ALL ON, TEXT ONLY, GAUGES ONLY, ALL OFF
			if (!IsMultimapOverlaysText()&&!IsMultimapOverlaysGauges()) {
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2079),MsgToken(899)); // ALL ON
			} else {
				if (IsMultimapOverlaysAll()) {
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2079),MsgToken(2234)); // TEXT
				} else {
					if (IsMultimapOverlaysText())
						_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2079),MsgToken(2235)); // GAUGES
					else
						_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2079),MsgToken(898)); // ALL OFF
				}
			}
			break;

		case 18:
			if (Orbiter)
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2065),MsgToken(491)); // OFF
			else
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2065),MsgToken(894)); // ON
			if (!EnableThermalLocator) invalid = true;
			break;

		case 19:
			if (IsMultimapAirspace())
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2029),MsgToken(491)); // OFF
			else
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2029),MsgToken(894)); // ON
			break;

		case 20:
			if (MapWindow::zoom.AutoZoom() )
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2009),MsgToken(418)); 
			else
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2009),MsgToken(897));
			break;

		case 21:
			if (IsMultimapTopology())
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2027),MsgToken(491)); // OFF
			else
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2027),MsgToken(894)); // ON
			break;

		case 22:
			if (IsMultimapTerrain())
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2028),MsgToken(491)); // OFF
			else
				_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2028),MsgToken(894)); // ON
			break;

		case 23:
			if (MapSpaceMode!=MSM_MAP) invalid=true;
			if (MapWindow::mode.UserForcedMode() == MapWindow::Mode::MODE_FLY_CIRCLING)
				_stprintf(OutBuffer,_T("DspMode\n_%s_"),MsgToken(2031));
			else
				_stprintf(OutBuffer,_T("DspMode\n%s"),MsgToken(2031));
			break;

		case 24:
			if (MapSpaceMode!=MSM_MAP) invalid=true;
			if (MapWindow::mode.UserForcedMode() == MapWindow::Mode::MODE_FLY_CRUISE)
				_stprintf(OutBuffer,_T("DspMode\n_%s_"),MsgToken(2032));
			else
				_stprintf(OutBuffer,_T("DspMode\n%s"),MsgToken(2032));
			break;
		case 25:
			if (MapSpaceMode!=MSM_MAP) invalid=true;
			if (MapWindow::mode.UserForcedMode() == MapWindow::Mode::MODE_FLY_NONE)
				_stprintf(OutBuffer,_T("DspMode\n_%s_"),MsgToken(2034));
			else
				_stprintf(OutBuffer,_T("DspMode\n%s"),MsgToken(2034));
			break;
		case 26:
			if (MapSpaceMode!=MSM_MAP) invalid=true;
			if (MapWindow::mode.UserForcedMode() == MapWindow::Mode::MODE_FLY_FINAL_GLIDE)
				_stprintf(OutBuffer,_T("DspMode\n_%s_"),MsgToken(2033));
			else
				_stprintf(OutBuffer,_T("DspMode\n%s"),MsgToken(2033));
			break;

		case 27: // amcIsBoth
			if (CALCULATED_INFO.AutoMacCready && AutoMcMode==amcFinalAndClimb)
				_stprintf(OutBuffer,_T("Auto\n_%s_"),MsgToken(2117));
			else
				_stprintf(OutBuffer,_T("Auto\n%s"),MsgToken(2117));
			break;
		case 28: // amcIsFinal
			if (CALCULATED_INFO.AutoMacCready && AutoMcMode==amcFinalGlide)
				_stprintf(OutBuffer,_T("Auto\n_%s_"),MsgToken(2033));
			else
				_stprintf(OutBuffer,_T("Auto\n%s"),MsgToken(2033));
			break;
		case 29: // amcIsClimb
			if (CALCULATED_INFO.AutoMacCready && AutoMcMode==amcAverageClimb)
				_stprintf(OutBuffer,_T("Auto\n_%s_"),MsgToken(2075));
			else
				_stprintf(OutBuffer,_T("Auto\n%s"),MsgToken(2075));
			break;
		case 30: // amcIsEquiv
			if (CALCULATED_INFO.AutoMacCready && AutoMcMode==amcEquivalent)
				_stprintf(OutBuffer,_T("Auto\n_%s_"),MsgToken(2076));
			else
				_stprintf(OutBuffer,_T("Auto\n%s"),MsgToken(2076));
			break;
		case 31: // CheckManMc
			if (CALCULATED_INFO.AutoMacCready)
				_stprintf(OutBuffer,_T("Mc\n%s"),MsgToken(2077));
			else
				_stprintf(OutBuffer,_T("Mc\n_%s_"),MsgToken(2077));
			break;

		case 32: // AirspaceMode
			switch(AltitudeMode) {
				case 0:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2029),MsgToken(184)); // Clip
					break;
				case 1:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2029),MsgToken(897)); // Auto
					break;
				case 2:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2029),MsgToken(139)); // Below
					break;
				case 3:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2029),MsgToken(359)); // Inside
					break;
				case 4:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2029),MsgToken(75)); // All Off
					break;
				case 5:
				default:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2029),MsgToken(76)); // All On
					break;
			}
			break;

		case 33: // SnailTrailToggleName
			if (MapSpaceMode!=MSM_MAP) invalid=true;
			switch(TrailActive) {
				case 0:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2035),MsgToken(410)); // Long
					break;
				case 1:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2035),MsgToken(612)); // Short
					break;
				case 2:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2035),MsgToken(312)); // Full
					break;
				case 3:
				default:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2035),MsgToken(491)); // OFF
					break;
			}
			break;

		case 34: // MapLabelsToggleActionName
			switch(GetMultimap_Labels()) {
				case MAPLABELS_ALLON:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2026),MsgToken(1203)); // WPTS
					break;
				case MAPLABELS_ONLYWPS:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2026),MsgToken(1204)); // TOPO
					break;
				case MAPLABELS_ONLYTOPO:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2026),MsgToken(898));
					break;
				case MAPLABELS_ALLOFF:
				default:
					_stprintf(OutBuffer,_T("%s\n%s"),MsgToken(2026),MsgToken(899));
					break;
			}
			break;

		case 35: // SIM PAN MODE REPOSITION, PANREPOS
			if (SIMMODE)
				_tcscpy(OutBuffer,MsgToken(2133)); // Position
			else
				_tcscpy(OutBuffer,_T(""));
			break;

		case 36: // 
			// Order is:  TASK ONLY, FAI ONLY, ALL OFF
			if (Flags_DrawTask&&!Flags_DrawFAI) {
				_stprintf(OutBuffer,MsgToken(2239)); // Draw FAI
			} else {
				if (!Flags_DrawTask&&Flags_DrawFAI) {
					_stprintf(OutBuffer,MsgToken(2240)); // NoDraw TaskFAI
				} else {
					_stprintf(OutBuffer,MsgToken(2238)); // Draw Task
				}
			}
			break;
		case 37: //
			extern bool Sonar_IsEnabled;
			if (Sonar_IsEnabled)
				_stprintf(OutBuffer,MsgToken(2243)); // Sonar OFF
			else
				_stprintf(OutBuffer,MsgToken(2242)); // Sonar ON
			break;
		case 38: //
			if (MapSpaceMode!=MSM_MAP) invalid=true;
			_stprintf(OutBuffer,MsgToken(2081)); // Set Map
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
      ReplaceInString(OutBuffer, TEXT("$(AdvanceArmed)"), MsgToken(892), Size); // (manual)
      invalid = true;
      break;
    case 1:
      ReplaceInString(OutBuffer, TEXT("$(AdvanceArmed)"), MsgToken(893), Size); // (auto)
      invalid = true;
      break;
    case 2:
      if (ActiveWayPoint>0) {
        if (ValidTaskPoint(ActiveWayPoint+1)) {
          CondReplaceInString(AdvanceArmed, OutBuffer, TEXT("$(AdvanceArmed)"), 
		MsgToken(161),  // Cancel
		MsgToken(678), Size); // TURN
        } else {
          ReplaceInString(OutBuffer, TEXT("$(AdvanceArmed)"), MsgToken(8), Size); // (finish)
          invalid = true;
        }
      } else {
        CondReplaceInString(AdvanceArmed, OutBuffer, TEXT("$(AdvanceArmed)"), 
		MsgToken(161),  // Cancel
		MsgToken(571), Size); // START
      }
      break;
    case 3:
      if (ActiveWayPoint==0) {
        CondReplaceInString(AdvanceArmed, OutBuffer, TEXT("$(AdvanceArmed)"), 
		MsgToken(161),  // Cancel
		MsgToken(571), Size); // START
      } else if (ActiveWayPoint==1) {
        CondReplaceInString(AdvanceArmed, OutBuffer, TEXT("$(AdvanceArmed)"), 
		MsgToken(161),  // Cancel
		MsgToken(539), Size); // RESTART
      } else {
        ReplaceInString(OutBuffer, TEXT("$(AdvanceArmed)"), MsgToken(893), Size); // (auto)
        invalid = true;
      }
      break;
      // TODO bug: no need to arm finish
    case 4:
      if (ActiveWayPoint>0) {
        if (ValidTaskPoint(ActiveWayPoint+1)) {
          CondReplaceInString(AdvanceArmed, OutBuffer, TEXT("$(AdvanceArmed)"), 
		MsgToken(161),  // Cancel
		MsgToken(678), Size); // TURN
        } else {
          ReplaceInString(OutBuffer, TEXT("$(AdvanceArmed)"), MsgToken(8), Size); // (finish)
          invalid = true;
        }
      }
      else {
        ReplaceInString(OutBuffer, TEXT("$(AdvanceArmed)"), MsgToken(893), Size); // (auto)
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
    if (!ValidTaskPoint(ActiveWayPoint)) {
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
	_stprintf(tbuf,_T("%.0f%s"),SPEEDMODIFY*WindCalcSpeed,Units::GetUnitName(Units::GetUserHorizontalSpeedUnit()) );
	ReplaceInString(OutBuffer, TEXT("$(WCSpeed)"), tbuf, Size);
	if (--items<=0) goto label_ret; // 100517
  }

  if (_tcsstr(OutBuffer, TEXT("$(GS"))) {
	TCHAR tbuf[10];
	_stprintf(tbuf,_T("%.0f%s"),SPEEDMODIFY*GPS_INFO.Speed,Units::GetUnitName(Units::GetUserHorizontalSpeedUnit()) );
	ReplaceInString(OutBuffer, TEXT("$(GS)"), tbuf, Size);
	if (--items<=0) goto label_ret;
  }
  if (_tcsstr(OutBuffer, TEXT("$(HGPS"))) {
	TCHAR tbuf[10];
	_stprintf(tbuf,_T("%.0f%s"),ALTITUDEMODIFY*GPS_INFO.Altitude,Units::GetUnitName(Units::GetUserAltitudeUnit()) );
	ReplaceInString(OutBuffer, TEXT("$(HGPS)"), tbuf, Size);
	if (--items<=0) goto label_ret;
  }
  if (_tcsstr(OutBuffer, TEXT("$(TURN"))) {
	TCHAR tbuf[10];
	_stprintf(tbuf,_T("%.0f"),SimTurn);
	ReplaceInString(OutBuffer, TEXT("$(TURN)"), tbuf, Size);
	if (--items<=0) goto label_ret;
  }


  if (_tcsstr(OutBuffer, TEXT("$(LoggerActive)"))) {
	CondReplaceInString(LoggerActive, OutBuffer, TEXT("$(LoggerActive)"), MsgToken(670), MsgToken(657), Size); // Stop Start
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
                        MsgToken(896), // Unforce
                        MsgToken(895), // Force
			Size);
    if (AutoForceFinalGlide) {
      invalid = true;
    }
	if (--items<=0) goto label_ret; // 100517
  }



  if (_tcsstr(OutBuffer, TEXT("$(PCONLY)"))) {
    #if (WINDOWSPC>0)
    ReplaceInString(OutBuffer, TEXT("$(PCONLY)"), TEXT(""), Size);
    #else
    _tcscpy(OutBuffer,_T(""));
    invalid = true;
    #endif
    if (--items<=0) goto label_ret;
  }
  if (_tcsstr(OutBuffer, TEXT("$(NOTPC)"))) {
    #if (WINDOWSPC>0)
    _tcscpy(OutBuffer,_T(""));
    invalid = true;
    #else
    ReplaceInString(OutBuffer, TEXT("$(NOTPC)"), TEXT(""), Size);
    #endif
    if (--items<=0) goto label_ret;
  }

  if (_tcsstr(OutBuffer, TEXT("$(ONLYMAP)"))) {
    if (MapSpaceMode!=MSM_MAP) invalid=true;
    ReplaceInString(OutBuffer, TEXT("$(ONLYMAP)"), TEXT(""), Size);

    if (--items<=0) goto label_ret;
  }

  extern unsigned int CustomKeyLabel[];
  // We dont replace macro, we do replace the entire label
  a =_tcsstr(OutBuffer, TEXT("$(MM"));
  if (a != NULL) {
	short i;
	i= *(a+4)-48;
        LKASSERT(i>=0 && i<11);
	// get the label for the custom menu item here
	// Decide if invalid=true or if no label at all, setting Replace to empty string

	unsigned int ckeymode;
	// test mode only
	switch(i) {
	      case 1:
  	              ckeymode=CustomMenu1;
  	              break;
  	      case 2:
  	              ckeymode=CustomMenu2;
  	              break;
  	      case 3:
  	              ckeymode=CustomMenu3;
  	              break;
  	      case 4:
  	              ckeymode=CustomMenu4;
  	              break;
  	      case 5:
  	              ckeymode=CustomMenu5;
  	              break;
  	      case 6:
  	              ckeymode=CustomMenu6;
  	              break;
  	      case 7:
  	              ckeymode=CustomMenu7;
  	              break;
  	      case 8:
  	              ckeymode=CustomMenu8;
  	              break;
  	      case 9:
  	              ckeymode=CustomMenu9;
  	              break;
  	      case 0:
  	              ckeymode=CustomMenu10;
  	              break;
  	      default:
        	        ckeymode=0;
        	        break;
	}
	LKASSERT(ckeymode<ckTOP);
	if (ckeymode==0) {
		invalid=true;			// non selectable
		
		// _stprintf(OutBuffer,_T("Key\n%d"),i);
		 _tcscpy(OutBuffer,_T(""));	// make it invisible
	} else {
		_stprintf(OutBuffer,MsgToken( CustomKeyLabel[ckeymode] ));
	}

  } // MM
	

label_ret:

  return invalid;
}


