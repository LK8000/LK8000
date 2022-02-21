/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id$
 */

#include "externs.h"
#include "LKInterface.h"
#include "McReady.h"
#include "Modeltype.h"
#include "LKProfiles.h"
#include "utils/stringext.h"
#include "Asset.hpp"
#include "Screen/Init.hpp"
#include "Util/Clamp.hpp"
#include "Settings/read.h"
#include <Tracking/Tracking.h>

extern bool CommandResolution;

namespace {

constexpr int nMaxValueValueSize = MAX_PATH * 2 + 6;    // max regkey name is 256 chars + " = "
bool isDefaultProfile = false; // needed to avoid screensize changes from custom profiles on PC

using unique_file_ptr = std::unique_ptr<FILE, decltype(&fclose)>;

unique_file_ptr make_unique_file(const TCHAR* filename, const TCHAR * flags) {
  return unique_file_ptr(_tfopen(filename, flags), &fclose);
}

}

void LKParseProfileString(const char *sname, const char *svalue);


//
// Returns true if at least one value was found,
// excluded comments and empty lines
//
bool LKProfileLoad(const TCHAR *szFile) {
#if TESTBENCH
  StartupStore(_T("... LoadProfile <%s>%s"), szFile, NEWLINE);
#endif

  if (!szFile || !szFile[0]) {
    // nullptr or empty string.
    return false;
  }

  if (!IsEmbedded()) {
    isDefaultProfile = (_tcscmp(defaultProfileFile, szFile) == 0);
  }

  bool found = false;

  unique_file_ptr fp = make_unique_file(szFile, TEXT("rb"));
  if (!fp) {
    StartupStore(_T(".... LoadProfile <%s> open failed%s"), szFile, NEWLINE);
    return false;
  }

  char inval[nMaxValueValueSize];
  char name[nMaxValueValueSize];
  char value[nMaxValueValueSize];

  // UTF8 file
  while (fgets(inval, nMaxValueValueSize, fp.get())) {

    if (sscanf(inval, "%[^#=\r\n ]=\"%[^\r\n\"]\"[\r\n]", name, value) == 2) {
      if (strlen(name) > 0) {
        LKParseProfileString(name, value);
        found = true;
      }
    } else if (sscanf(inval, "%[^#=\r\n ]=%[^\r\n\"][\r\n]", name, value) == 2) {
      if (strlen(name) > 0) {
        LKParseProfileString(name, value);
        found = true;
      }
    } else if (sscanf(inval, "%[^#=\r\n ]=\"\"[\r\n]", name) == 1) {
      if (strlen(name) > 0) {
        LKParseProfileString(name, "");
        found = true;
      }
    }
    // else crlf, or comment, or invalid line
    // else StartupStore(_T("...... PARSE INVALID: <%S>\n"),inval);
  }
  return found;
}



#define IO_PARAM_SIZE 160

//
// Search for a match of the keyname. Profile is NOT necessarily sorted!
// So we must check against all possible values until we find the good one.
// As soon as we find the match, we can return.
// Important: some parameters are saved multiplied by 10 or 1000, so they must
// be adjusted here. Example SafetyMacCready
void LKParseProfileString(const char *sname, const char *svalue) {

  //
  // RESPECT LKPROFILE.H ALPHA ORDER OR WE SHALL GET LOST SOON!
  //
  // -- USE _CONFIG VARIABLES WHEN A RUNTIME VALUE CAN BE CHANGED --
  // WE DONT WANT TO SAVE RUNTIME TEMPORARY CONFIGURATIONS, ONLY SYSTEM CONFIG!
  // FOR EXAMPLE: ActiveMap can be set by default in system config, but also changed
  // at runtime with a button and with a customkey. We must save in profile ONLY
  // the _Config, not the temporary setup!
  //

  if (settings::read(sname, svalue, szRegistryAcknowledgementTime, AcknowledgementTime)) {
    AcknowledgementTime = max(10, AcknowledgementTime);
    return;
  }

  if (settings::read(sname, svalue, szRegistryAircraftCategory, AircraftCategory)) return;
  if (settings::read(sname, svalue, szRegistryAircraftRego, AircraftRego_Config)) return;
  if (settings::read(sname, svalue, szRegistryAircraftType, AircraftType_Config)) return;

  if (settings::read(sname, svalue, szRegistryAirfieldFile, szAirfieldFile)) {
    RemoveFilePathPrefix(_T("%LOCAL_PATH%"), szAirfieldFile);
    RemoveFilePathPrefix(_T(LKD_WAYPOINTS), szAirfieldFile);
    return;
  }

  for (unsigned int i = 0; i < NO_AS_FILES; i++) {
    if (settings::read(sname, svalue, szRegistryAirspaceFile[i], szAirspaceFile[i])) {
      RemoveFilePathPrefix(_T("%LOCAL_PATH%"), szAirspaceFile[i]);
      RemoveFilePathPrefix(_T(LKD_AIRSPACES), szAirspaceFile[i]);
      return;
    }
  }

  // Special cases with no global variable and a function to access the private variable.
  // This is bad. We want a common global variable approach for the future.
  // We want a memory area with values, not with function calls.

  if (!strcmp(szRegistryAirspaceFillType, sname)) {
    int ival = strtol(svalue, nullptr, 10);
    MapWindow::SetAirSpaceFillType((MapWindow::EAirspaceFillType) ival);
    return;
  }
  if (!strcmp(szRegistryAirspaceOpacity, sname)) {
    int ival = strtol(svalue, nullptr, 10);
    MapWindow::SetAirSpaceOpacity(ival);
    return;
  }

  if (settings::read(sname, svalue, szRegistryAirspaceWarningDlgTimeout, AirspaceWarningDlgTimeout)) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryAirspaceWarningMapLabels, AirspaceWarningMapLabels)) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryAirspaceAckAllSame, AirspaceAckAllSame)) return;
  if (settings::read(sname, svalue, szRegistryAirspaceWarningRepeatTime, AirspaceWarningRepeatTime)) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryAirspaceWarningVerticalMargin,
                AirspaceWarningVerticalMargin)) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryAirspaceWarning, AIRSPACEWARNINGS)) return;
  if (settings::read(sname, svalue, szRegistryAlarmMaxAltitude1, AlarmMaxAltitude1)) return;
  if (settings::read(sname, svalue, szRegistryAlarmMaxAltitude2, AlarmMaxAltitude2)) return;
  if (settings::read(sname, svalue, szRegistryAlarmMaxAltitude3, AlarmMaxAltitude3)) return;
  if (settings::read(sname, svalue, szRegistryAlarmTakeoffSafety, AlarmTakeoffSafety)) return;
  if (settings::read(sname, svalue, szRegistryAltMargin, AltWarningMargin)) return;
  if (settings::read(sname, svalue, szRegistryAltMode, AltitudeMode_Config)) return;

  if (settings::read(sname, svalue, szRegistryAlternate1, Alternate1)) return;
  if (settings::read(sname, svalue, szRegistryAlternate2, Alternate2)) return;
  if (settings::read(sname, svalue, szRegistryAltitudeUnitsValue, AltitudeUnit_Config)) return;
  if (settings::read(sname, svalue, szRegistryAppIndLandable, Appearance.IndLandable)) return;
  if (settings::read(sname, svalue, szRegistryUTF8Symbolsl, Appearance.UTF8Pictorials)) return;
  if (settings::read(sname, svalue, szRegistryAppInfoBoxModel, GlobalModelType)) return;

  if (settings::read(sname, svalue, szRegistryAppInverseInfoBox, InverseInfoBox_Config)) return;
  if (settings::read(sname, svalue, szRegistryArrivalValue, ArrivalValue)) return;
  if (settings::read(sname, svalue, szRegistryAutoAdvance, AutoAdvance_Config)) return;
  if (settings::read(sname, svalue, szRegistryAutoBacklight, EnableAutoBacklight)) return;
  if (settings::read(sname, svalue, szRegistryAutoForceFinalGlide, AutoForceFinalGlide)) return;
  if (settings::read(sname, svalue, szRegistryAutoMcMode, AutoMcMode_Config)) return;
  if (settings::read(sname, svalue, szRegistryAutoMcStatus, AutoMacCready_Config)) return;

  if (settings::read(sname, svalue, szRegistryAutoOrientScale, AutoOrientScale)) {
    AutoOrientScale /= 10;
    return;
  }


  if (settings::read(sname, svalue, szRegistryAutoSoundVolume, EnableAutoSoundVolume)) return;
  if (settings::read(sname, svalue, szRegistryAutoWind, AutoWindMode_Config)) return;
  if (settings::read(sname, svalue, szRegistryAutoZoom, AutoZoom_Config)) return;
  if (settings::read(sname, svalue, szRegistryAverEffTime, AverEffTime)) return;
  if (settings::read(sname, svalue, szRegistryBallastSecsToEmpty, BallastSecsToEmpty)) return;
  if (settings::read(sname, svalue, szRegistryBarOpacity, BarOpacity)) return;
  if (settings::read(sname, svalue, szRegistryBestWarning, BestWarning)) return;
  if (settings::read(sname, svalue, szRegistryBgMapColor, BgMapColor_Config)) return;

  if (settings::read(sname, svalue, szRegistryBugs, BUGS_Config)) {
    BUGS_Config /= 100;
    return;
  }

  if (settings::read(sname, svalue, szRegistryCheckSum, CheckSum)) return;

  if (!strcmp(szRegistryCircleZoom, sname)) {
    int ival = strtol(svalue, nullptr, 10);
    MapWindow::zoom.CircleZoom(ival == 1);
    return;
  }

  if (settings::read(sname, svalue, szRegistryClipAlt, ClipAltitude)) return;
  if (settings::read(sname, svalue, szRegistryCompetitionClass, CompetitionClass_Config)) return;
  if (settings::read(sname, svalue, szRegistryCompetitionID, CompetitionID_Config)) return;
  if (settings::read(sname, svalue, szRegistryConfBB0, ConfBB0)) return;
  if (settings::read(sname, svalue, szRegistryConfBB1, ConfBB1)) return;
  if (settings::read(sname, svalue, szRegistryConfBB2, ConfBB2)) return;
  if (settings::read(sname, svalue, szRegistryConfBB3, ConfBB3)) return;
  if (settings::read(sname, svalue, szRegistryConfBB4, ConfBB4)) return;
  if (settings::read(sname, svalue, szRegistryConfBB5, ConfBB5)) return;
  if (settings::read(sname, svalue, szRegistryConfBB6, ConfBB6)) return;
  if (settings::read(sname, svalue, szRegistryConfBB7, ConfBB7)) return;
  if (settings::read(sname, svalue, szRegistryConfBB8, ConfBB8)) return;
  if (settings::read(sname, svalue, szRegistryConfBB9, ConfBB9)) return;
  if (settings::read(sname, svalue, szRegistryConfBB0Auto, ConfBB0Auto)) return;
  if (settings::read(sname, svalue, szRegistryConfIP11, ConfIP11)) return;
  if (settings::read(sname, svalue, szRegistryConfIP12, ConfIP12)) return;
  if (settings::read(sname, svalue, szRegistryConfIP13, ConfIP13)) return;
  if (settings::read(sname, svalue, szRegistryConfIP14, ConfIP14)) return;
  if (settings::read(sname, svalue, szRegistryConfIP15, ConfIP15)) return;
  if (settings::read(sname, svalue, szRegistryConfIP16, ConfIP16)) return;
  if (settings::read(sname, svalue, szRegistryConfIP17, ConfIP17)) return;
  if (settings::read(sname, svalue, szRegistryConfIP21, ConfIP21)) return;
  if (settings::read(sname, svalue, szRegistryConfIP22, ConfIP22)) return;
  if (settings::read(sname, svalue, szRegistryConfIP23, ConfIP23)) return;
  if (settings::read(sname, svalue, szRegistryConfIP24, ConfIP24)) return;
  if (settings::read(sname, svalue, szRegistryConfIP31, ConfIP31)) return;
  if (settings::read(sname, svalue, szRegistryConfIP32, ConfIP32)) return;
  if (settings::read(sname, svalue, szRegistryConfIP33, ConfIP33)) return;
  if (settings::read(sname, svalue, szRegistryCustomKeyModeAircraftIcon, CustomKeyModeAircraftIcon)) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryCustomKeyModeCenterScreen, CustomKeyModeCenterScreen)) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryCustomKeyModeCenter, CustomKeyModeCenter)) return;
  if (settings::read(sname, svalue, szRegistryCustomKeyModeLeftUpCorner, CustomKeyModeLeftUpCorner)) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryCustomKeyModeLeft, CustomKeyModeLeft)) return;
  if (settings::read(sname, svalue, szRegistryCustomKeyModeRightUpCorner, CustomKeyModeRightUpCorner)) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryCustomKeyModeRight, CustomKeyModeRight)) return;
  if (settings::read(sname, svalue, szRegistryCustomKeyTime, CustomKeyTime)) return;
  if (settings::read(sname, svalue, szRegistryCustomMenu1, CustomMenu1)) return;
  if (settings::read(sname, svalue, szRegistryCustomMenu2, CustomMenu2)) return;
  if (settings::read(sname, svalue, szRegistryCustomMenu3, CustomMenu3)) return;
  if (settings::read(sname, svalue, szRegistryCustomMenu4, CustomMenu4)) return;
  if (settings::read(sname, svalue, szRegistryCustomMenu5, CustomMenu5)) return;
  if (settings::read(sname, svalue, szRegistryCustomMenu6, CustomMenu6)) return;
  if (settings::read(sname, svalue, szRegistryCustomMenu7, CustomMenu7)) return;
  if (settings::read(sname, svalue, szRegistryCustomMenu8, CustomMenu8)) return;
  if (settings::read(sname, svalue, szRegistryCustomMenu9, CustomMenu9)) return;
  if (settings::read(sname, svalue, szRegistryCustomMenu10, CustomMenu10)) return;

  if (settings::read(sname, svalue, szRegistryDebounceTimeout, debounceTimeout)) return;
  if (settings::read(sname, svalue, szRegistryDeclutterMode, DeclutterMode)) return;

  if (settings::read(sname, svalue, szRegistryDisableAutoLogger, DisableAutoLogger)) return;
  if (settings::read(sname, svalue, szRegistryDisplayText, DisplayTextType)) return;
  if (settings::read(sname, svalue, szRegistryDisplayUpValue, DisplayOrientation_Config)) return;
  if (settings::read(sname, svalue, szRegistryDistanceUnitsValue, DistanceUnit_Config)) return;
  if (settings::read(sname, svalue, szRegistryEnableFLARMMap, EnableFLARMMap)) return;
  if (settings::read(sname, svalue, szRegistryEnableNavBaroAltitude, EnableNavBaroAltitude_Config)) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryFAIFinishHeight, EnableFAIFinishHeight)) return;
  if (settings::read(sname, svalue, szRegistryFAISector, SectorType)) return;
  if (settings::read(sname, svalue, szRegistryFinalGlideTerrain, FinalGlideTerrain)) return;
  if (settings::read(sname, svalue, szRegistryFinishLine, FinishLine)) return;
  if (settings::read(sname, svalue, szRegistryFinishMinHeight, FinishMinHeight)) return;
  if (settings::read(sname, svalue, szRegistryFinishRadius, FinishRadius)) return;
  if (settings::read(sname, svalue, szRegistryFontRenderer, FontRenderer)) return;
  if (settings::read(sname, svalue, szRegistryFontMapWaypoint, FontMapWaypoint)) return;
  if (settings::read(sname, svalue, szRegistryFontMapTopology, FontMapTopology)) return;
  if (settings::read(sname, svalue, szRegistryFontInfopage1L, FontInfopage1L)) return;
  if (settings::read(sname, svalue, szRegistryFontInfopage2L, FontInfopage2L)) return;
  if (settings::read(sname, svalue, szRegistryFontBottomBar, FontBottomBar)) return;
  if (settings::read(sname, svalue, szRegistryFontCustom1, FontCustom1)) return;
  if (settings::read(sname, svalue, szRegistryFontOverlayBig, FontOverlayBig)) return;
  if (settings::read(sname, svalue, szRegistryFontOverlayMedium, FontOverlayMedium)) return;
  if (settings::read(sname, svalue, szRegistryFontVisualGlide, FontVisualGlide)) return;
  if (settings::read(sname, svalue, szRegistryGlideBarMode, GlideBarMode)) return;
  if (settings::read(sname, svalue, szRegistryGliderScreenPosition, MapWindow::GliderScreenPosition)) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryGpsAltitudeOffset, GPSAltitudeOffset)) return;
  if (settings::read(sname, svalue, szRegistryHandicap, Handicap)) return;
  if (settings::read(sname, svalue, szRegistryHideUnits, HideUnits)) return;
  if (settings::read(sname, svalue, szRegistryHomeWaypoint, HomeWaypoint)) return;
  if (settings::read(sname, svalue, szRegistryDeclTakeOffLanding, DeclTakeoffLanding)) return;

  // InfoType
  for (int i = 0; i < MAXINFOWINDOWS; i++) {
    if (settings::read(sname, svalue, &*szRegistryDisplayType[i], InfoType[i])) return;
  }

  if (settings::read(sname, svalue, szRegistryInputFile, szInputFile)) {
    RemoveFilePathPrefix(_T("%LOCAL_PATH%"), szInputFile);
    RemoveFilePathPrefix(_T(LKD_CONF), szInputFile);
    return;
  }


  if (settings::read(sname, svalue, szRegistryIphoneGestures, IphoneGestures)) return;
  if (settings::read(sname, svalue, szRegistryLKMaxLabels, LKMaxLabels)) return;
  if (settings::read(sname, svalue, szRegistryLKTopoZoomCat05, LKTopoZoomCat05)) {
    LKTopoZoomCat05 /= 1000;
    return;
  }
  if (settings::read(sname, svalue, szRegistryLKTopoZoomCat100, LKTopoZoomCat100)) {
    LKTopoZoomCat100 /= 1000;
    return;
  }
  if (settings::read(sname, svalue, szRegistryLKTopoZoomCat10, LKTopoZoomCat10)) {
    LKTopoZoomCat10 /= 1000;
    return;
  }
  if (settings::read(sname, svalue, szRegistryLKTopoZoomCat110, LKTopoZoomCat110)) {
    LKTopoZoomCat110 /= 1000;
    return;
  }
  if (settings::read(sname, svalue, szRegistryLKTopoZoomCat20, LKTopoZoomCat20)) {
    LKTopoZoomCat20 /= 1000;
    return;
  }
  if (settings::read(sname, svalue, szRegistryLKTopoZoomCat30, LKTopoZoomCat30)) {
    LKTopoZoomCat30 /= 1000;
    return;
  }
  if (settings::read(sname, svalue, szRegistryLKTopoZoomCat40, LKTopoZoomCat40)) {
    LKTopoZoomCat40 /= 1000;
    return;
  }
  if (settings::read(sname, svalue, szRegistryLKTopoZoomCat50, LKTopoZoomCat50)) {
    LKTopoZoomCat50 /= 1000;
    return;
  }
  if (settings::read(sname, svalue, szRegistryLKTopoZoomCat60, LKTopoZoomCat60)) {
    LKTopoZoomCat60 /= 1000;
    return;
  }
  if (settings::read(sname, svalue, szRegistryLKTopoZoomCat70, LKTopoZoomCat70)) {
    LKTopoZoomCat70 /= 1000;
    return;
  }
  if (settings::read(sname, svalue, szRegistryLKTopoZoomCat80, LKTopoZoomCat80)) {
    LKTopoZoomCat80 /= 1000;
    return;
  }
  if (settings::read(sname, svalue, szRegistryLKTopoZoomCat90, LKTopoZoomCat90)) {
    LKTopoZoomCat90 /= 1000;
    return;
  }
  if (settings::read(sname, svalue, szRegistryLKVarioBar, LKVarioBar)) return;
  if (settings::read(sname, svalue, szRegistryLKVarioVal, LKVarioVal)) return;

  if (settings::read(sname, svalue, szRegistryLanguageCode, szLanguageCode)) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryLatLonUnits, Units::CoordinateFormat)) return;

  if (settings::read(sname, svalue, szRegistryLiftUnitsValue, LiftUnit_Config)) return;
  if (settings::read(sname, svalue, szRegistryLockSettingsInFlight, LockSettingsInFlight)) return;
  if (settings::read(sname, svalue, szRegistryLoggerShort, LoggerShortName)) return;
  if (settings::read(sname, svalue, szRegistryMapBox, MapBox)) return;
  if (settings::read(sname, svalue, szRegistryMapFile, szMapFile)) {
    RemoveFilePathPrefix(_T("%LOCAL_PATH%"), szMapFile);
    RemoveFilePathPrefix(_T(LKD_MAPS), szMapFile);
    return;
  }


  if (settings::read(sname, svalue, szRegistryMenuTimeout, MenuTimeout_Config)) return;
  if (settings::read(sname, svalue, szRegistryNewMapDeclutter, NewMapDeclutter)) return;
  if (settings::read(sname, svalue, szRegistryOrbiter, Orbiter_Config)) return;
  if (settings::read(sname, svalue, szRegistryOutlinedTp, OutlinedTp_Config)) return;
  if (settings::read(sname, svalue, szRegistryOverColor, OverColor)) return;
  if (settings::read(sname, svalue, szRegistryOverlayClock, OverlayClock)) return;
  if (settings::read(sname, svalue, szRegistryUseTwoLines, UseTwoLines)) return;
  if (settings::read(sname, svalue, szRegistryOverlaySize, OverlaySize)) return;
  if (settings::read(sname, svalue, szRegistryAutoZoomThreshold, AutoZoomThreshold)) return;
  if (settings::read(sname, svalue, szRegistryClimbZoom, ClimbZoom)) return;
  if (settings::read(sname, svalue, szRegistryCruiseZoom, CruiseZoom)) return;
  if (settings::read(sname, svalue, szRegistryMaxAutoZoom, MaxAutoZoom)) return;
  if (settings::read(sname, svalue, szRegistryTskOptimizeRoute, TskOptimizeRoute_Config)) return;
  if (settings::read(sname, svalue, szRegistryGliderSymbol, GliderSymbol)) return;
  if (settings::read(sname, svalue, szRegistryPilotName, PilotName_Config)) return;
  if (settings::read(sname, svalue, szRegistryPolarFile, szPolarFile)) {
    RemoveFilePathPrefix(_T("%LOCAL_PATH%"), szPolarFile);
    RemoveFilePathPrefix(_T(LKD_POLARS), szPolarFile);
    RemoveFilePathPrefix(_T(LKD_SYS_POLAR), szPolarFile);
    return;
  }

  if (settings::read(sname, svalue, szRegistryPollingMode, PollingMode)) return;

  /***************************************************/
  /* for compatibilty with old file                  */
  unsigned dwIdxPort;
  if (settings::read(sname, svalue, szRegistryPort1Index, dwIdxPort)) {

#ifdef ANDROID
    ScopeLock lock(COMMPort_mutex);
#endif

    if (COMMPort.size() == 0) {
      RefreshComPortList();
    }
    if (dwIdxPort < COMMPort.size()) {
      PortConfig[0].SetPort(COMMPort[dwIdxPort].GetName());
    }
    return;
  }

  if (settings::read(sname, svalue, szRegistryPort2Index, dwIdxPort)) {

#ifdef ANDROID
    ScopeLock lock(COMMPort_mutex);
#endif

    if (COMMPort.size() == 0) {
      RefreshComPortList();
    }
    if (dwIdxPort < COMMPort.size()) {
      PortConfig[1].SetPort(COMMPort[dwIdxPort].GetName());
    }
    return;
  }
  /***************************************************/

  for (int n = 0; n < NUMDEV; n++) {
    auto& Port = PortConfig[n];

    if (settings::read(sname, svalue, szRegistryDevice[n], Port.szDeviceName)) return;
    if (settings::read(sname, svalue, szRegistryPortName[n], Port.szPort)) return;

    if (settings::read(sname, svalue, szRegistrySpeedIndex[n], Port.dwSpeedIndex)) return;
    if (settings::read(sname, svalue, szRegistryBitIndex[n], Port.dwBitIndex)) return;

    if (settings::read(sname, svalue, szRegistryIpAddress[n], Port.szIpAddress)) return;
    if (settings::read(sname, svalue, szRegistryIpPort[n], Port.dwIpPort)) return;
    
    if (settings::read(sname, svalue, szRegistryUseExtSound[n], Port.UseExtSound)) return;

    if (settings::read(sname, svalue, szRegistryReplayFile[n], Port.Replay_FileName)) return;
    if (settings::read(sname, svalue, szRegistryReplaySpeed[n], Port.ReplaySpeed)) return;
    if (settings::read(sname, svalue, szRegistryReplayRaw[n], Port.RawByteData)) return;
    if (settings::read(sname, svalue, szRegistryReplaySync[n], Port.ReplaySync)) return;

    int i = 0;

    TCHAR szTmp[IO_PARAM_SIZE] = _T("");
    TCHAR szItem[10];

    char szKey[20] = ("");
    sprintf(szKey, "%s%u", szRegistryIOValues, n + 1);
    if (settings::read(sname, svalue, szKey, szTmp) && (_tcslen(szTmp) > 0)) {
//      StartupStore(TEXT(" Load : szRegistryIOValues[%u] (%s) %s  %s"), n, szKey, szTmp, NEWLINE);
      NMEAParser::ExtractParameter(szTmp, szItem, i++);
      Port.PortIO.MCDir = (DataBiIoDir) _tcstoul(szItem, nullptr, 10);
      NMEAParser::ExtractParameter(szTmp, szItem, i++);
      Port.PortIO.BUGDir = (DataBiIoDir) _tcstoul(szItem, nullptr, 10); // (String, nullptr, 0)
      NMEAParser::ExtractParameter(szTmp, szItem, i++);
      Port.PortIO.BALDir = (DataBiIoDir) _tcstoul(szItem, nullptr, 10);

      NMEAParser::ExtractParameter(szTmp, szItem, i++);
      Port.PortIO.STFDir = (DataBiIoDir) _tcstoul(szItem, nullptr, 10);
      NMEAParser::ExtractParameter(szTmp, szItem, i++);
      Port.PortIO.WINDDir = (DataBiIoDir) _tcstoul(szItem, nullptr, 10);
      NMEAParser::ExtractParameter(szTmp, szItem, i++);
      Port.PortIO.BARODir = (DataBiIoDir) _tcstoul(szItem, nullptr, 10);

      NMEAParser::ExtractParameter(szTmp, szItem, i++);
      Port.PortIO.VARIODir = (DataBiIoDir) _tcstoul(szItem, nullptr, 10);
      NMEAParser::ExtractParameter(szTmp, szItem, i++);
      Port.PortIO.SPEEDDir = (DataBiIoDir) _tcstoul(szItem, nullptr, 10);
      NMEAParser::ExtractParameter(szTmp, szItem, i++);
      Port.PortIO.R_TRGTDir = (DataTP_Type) _tcstoul(szItem, nullptr, 10);

      NMEAParser::ExtractParameter(szTmp, szItem, i++);
      Port.PortIO.RADIODir = (DataBiIoDir) _tcstoul(szItem, nullptr, 10);
      NMEAParser::ExtractParameter(szTmp, szItem, i++);
      Port.PortIO.TRAFDir = (DataBiIoDir) _tcstoul(szItem, nullptr, 10);
      NMEAParser::ExtractParameter(szTmp, szItem, i++);
      Port.PortIO.GYRODir = (DataBiIoDir) _tcstoul(szItem, nullptr, 10);

      NMEAParser::ExtractParameter(szTmp, szItem, i++);
      Port.PortIO.GFORCEDir = (DataBiIoDir) _tcstoul(szItem, nullptr, 10);
      NMEAParser::ExtractParameter(szTmp, szItem, i++);
      Port.PortIO.OATDir = (DataBiIoDir) _tcstoul(szItem, nullptr, 10);
      NMEAParser::ExtractParameter(szTmp, szItem, i++);
      Port.PortIO.BAT1Dir = (DataBiIoDir) _tcstoul(szItem, nullptr, 10);

      NMEAParser::ExtractParameter(szTmp, szItem, i++);
      Port.PortIO.BAT2Dir = (DataBiIoDir) _tcstoul(szItem, nullptr, 10);
      NMEAParser::ExtractParameter(szTmp, szItem, i++);
      Port.PortIO.POLARDir = (DataBiIoDir) _tcstoul(szItem, nullptr, 10);
      NMEAParser::ExtractParameter(szTmp,szItem,i++);
      Port.PortIO.DirLink  = (DataBiIoDir) _tcstoul(szItem, nullptr, 10);
      NMEAParser::ExtractParameter(szTmp,szItem,i++);
      Port.PortIO.T_TRGTDir = (DataTP_Type) _tcstoul(szItem, nullptr, 10);
      NMEAParser::ExtractParameter(szTmp,szItem,i++);
      Port.PortIO.QNHDir = (DataBiIoDir) _tcstoul(szItem, nullptr, 10);
    }
  }


  if (settings::read(sname, svalue, szRegistryPressureHg, PressureHg)) return;
  if (settings::read(sname, svalue, szRegistrySafetyAltitudeArrival, SAFETYALTITUDEARRIVAL)) return;
  if (settings::read(sname, svalue, szRegistrySafetyAltitudeMode, SafetyAltitudeMode)) return;
  if (settings::read(sname, svalue, szRegistrySafetyAltitudeTerrain, SAFETYALTITUDETERRAIN)) return;

  // We save SafetyMacCready multiplied by 10, so we adjust it back after loading
  if (settings::read(sname, svalue, szRegistrySafetyMacCready, GlidePolar::SafetyMacCready)) {
    GlidePolar::SafetyMacCready /= 10;
    return;
  }

  if (settings::read(sname, svalue, szRegistrySafteySpeed, SAFTEYSPEED)) {
    SAFTEYSPEED /= 1000.0;
    if (SAFTEYSPEED < 8.0) {
#if TESTBENCH
      StartupStore(_T("... SAFTEYSPEED<8 set to 50 = 180kmh\n"));
#endif
      SAFTEYSPEED = 50.0;
    }
    return;
  }

  if (settings::read(sname, svalue, szRegistrySectorRadius, SectorRadius)) return;

#if defined(PPC2003) || defined(PNA)
  if( settings::read(sname,svalue,szRegistrySetSystemTimeFromGPS,SetSystemTimeFromGPS)) return;
#endif

  if (settings::read(sname, svalue, szRegistrySaveRuntime, SaveRuntime)) return;
  if (settings::read(sname, svalue, szRegistryShading, Shading_Config)) return;
  if (settings::read(sname, svalue, szRegistryIsoLine, IsoLine_Config)) return;
  if (settings::read(sname, svalue, szRegistrySnailTrail, TrailActive_Config)) return;
  if (settings::read(sname, svalue, szRegistrySnailScale, SnailScale)) return;
  if (settings::read(sname, svalue, szRegistrySpeedUnitsValue, SpeedUnit_Config)) return;
  if (settings::read(sname, svalue, szRegistryStartHeightRef, StartHeightRef)) return;
  if (settings::read(sname, svalue, szRegistryStartLine, StartLine)) return;
  if (settings::read(sname, svalue, szRegistryStartMaxHeightMargin, StartMaxHeightMargin)) return;
  if (settings::read(sname, svalue, szRegistryStartMaxHeight, StartMaxHeight)) return;
  if (settings::read(sname, svalue, szRegistryStartMaxSpeedMargin, StartMaxSpeedMargin)) return;
  if (settings::read(sname, svalue, szRegistryStartMaxSpeed, StartMaxSpeed)) return;
  if (settings::read(sname, svalue, szRegistryStartRadius, StartRadius)) return;
  if (settings::read(sname, svalue, szRegistryTaskSpeedUnitsValue, TaskSpeedUnit_Config)) return;
  if (settings::read(sname, svalue, szRegistryTeamcodeRefWaypoint, TeamCodeRefWaypoint)) return;
  if (settings::read(sname, svalue, szRegistryTerrainBrightness, TerrainBrightness)) return;
  if (settings::read(sname, svalue, szRegistryTerrainContrast, TerrainContrast)) return;

  if (settings::read(sname, svalue, szRegistryTerrainFile, szTerrainFile)) {
    RemoveFilePathPrefix(_T("%LOCAL_PATH%"), szTerrainFile);
    RemoveFilePathPrefix(_T(LKD_MAPS), szTerrainFile);
    return;
  }

  if (settings::read(sname, svalue, szRegistryTerrainRamp, TerrainRamp_Config)) return;
  if (settings::read(sname, svalue, szRegistryTerrainWhiteness, TerrainWhiteness)) {
    // we must be sure we are changing TerrainWhiteness only when loaded from profile
    // otherwise we would get 0.01  (1/100), black screen..
    TerrainWhiteness /= 100;
    return;
  }
  if (settings::read(sname, svalue, szRegistryThermalBar, ThermalBar)) return;
  if (settings::read(sname, svalue, szRegistryThermalLocator, EnableThermalLocator)) return;
  if (settings::read(sname, svalue, szRegistryTpFilter, TpFilter)) return;
  if (settings::read(sname, svalue, szRegistryTrackBar, TrackBar)) return;
  if (settings::read(sname, svalue, szRegistryTrailDrift, EnableTrailDrift_Config)) return;

  if (settings::read(sname, svalue, szRegistryUTCOffset, UTCOffset)) {
    while (UTCOffset > 12 * 3600) {
      UTCOffset -= 24 * 3600;
    }
    return;
  }

  if (settings::read(sname, svalue, szRegistryUseGeoidSeparation, UseGeoidSeparation)) return;

  if (settings::read(sname, svalue, szRegistryUseUngestures, UseUngestures)) return;
  if (settings::read(sname, svalue, szRegistryUseTotalEnergy, UseTotalEnergy_Config)) return;
  if (settings::read(sname, svalue, szRegistryWarningTime, WarningTime)) return;

  for (unsigned int i = 0; i < NO_WP_FILES; i++) {
    if (settings::read(sname, svalue, szRegistryWayPointFile[i], szWaypointFile[i])) {
      RemoveFilePathPrefix(_T("%LOCAL_PATH%"), szWaypointFile[i]);
      RemoveFilePathPrefix(_T(LKD_WAYPOINTS), szWaypointFile[i]);
      return;
    }
  }

  if (settings::read(sname, svalue, szRegistryWaypointsOutOfRange, WaypointsOutOfRange)) return;
  if (settings::read(sname, svalue, szRegistryWindCalcSpeed, WindCalcSpeed)) {
    WindCalcSpeed /= 1000.0;
    if (WindCalcSpeed < 2) {
      WindCalcSpeed = 27.778;
    }
    return;
  }
  if (settings::read(sname, svalue, szRegistryWindCalcTime, WindCalcTime)) return;

  for (int i = 0; i < AIRSPACECLASSCOUNT; i++) {
    if (settings::read(sname, svalue, szRegistryAirspaceMode[i], MapWindow::iAirspaceMode[i])) return;
    if (settings::read(sname, svalue, szRegistryColour[i], MapWindow::iAirspaceColour[i])) return;
#ifdef HAVE_HATCHED_BRUSH
    if (settings::read(sname, svalue, szRegistryBrush[i], MapWindow::iAirspaceBrush[i])) return;
#endif
  }

  if (settings::read(sname, svalue, szRegistryUseWindRose, UseWindRose)) return;

  if (settings::read(sname, svalue, szRegistryMultiTerr0, Multimap_Flags_Terrain[MP_MOVING])) return;
  if (settings::read(sname, svalue, szRegistryMultiTerr1, Multimap_Flags_Terrain[MP_MAPTRK])) return;
  if (settings::read(sname, svalue, szRegistryMultiTerr2, Multimap_Flags_Terrain[MP_MAPWPT])) return;
  if (settings::read(sname, svalue, szRegistryMultiTerr3, Multimap_Flags_Terrain[MP_MAPASP])) return;
  if (settings::read(sname, svalue, szRegistryMultiTerr4, Multimap_Flags_Terrain[MP_VISUALGLIDE]))
    return;
  if (settings::read(sname, svalue, szRegistryMultiTopo0, Multimap_Flags_Topology[MP_MOVING])) return;
  if (settings::read(sname, svalue, szRegistryMultiTopo1, Multimap_Flags_Topology[MP_MAPTRK])) return;
  if (settings::read(sname, svalue, szRegistryMultiTopo2, Multimap_Flags_Topology[MP_MAPWPT])) return;
  if (settings::read(sname, svalue, szRegistryMultiTopo3, Multimap_Flags_Topology[MP_MAPASP])) return;
  if (settings::read(sname, svalue, szRegistryMultiTopo4, Multimap_Flags_Topology[MP_VISUALGLIDE]))
    return;
  if (settings::read(sname, svalue, szRegistryMultiAsp0, Multimap_Flags_Airspace[MP_MOVING])) return;
  if (settings::read(sname, svalue, szRegistryMultiAsp1, Multimap_Flags_Airspace[MP_MAPTRK])) return;
  if (settings::read(sname, svalue, szRegistryMultiAsp2, Multimap_Flags_Airspace[MP_MAPWPT])) return;
  if (settings::read(sname, svalue, szRegistryMultiAsp3, Multimap_Flags_Airspace[MP_MAPASP])) return;
  if (settings::read(sname, svalue, szRegistryMultiAsp4, Multimap_Flags_Airspace[MP_VISUALGLIDE]))
    return;
  if (settings::read(sname, svalue, szRegistryMultiLab0, Multimap_Labels[MP_MOVING])) return;
  if (settings::read(sname, svalue, szRegistryMultiLab1, Multimap_Labels[MP_MAPTRK])) return;
  if (settings::read(sname, svalue, szRegistryMultiLab2, Multimap_Labels[MP_MAPWPT])) return;
  if (settings::read(sname, svalue, szRegistryMultiLab3, Multimap_Labels[MP_MAPASP])) return;
  if (settings::read(sname, svalue, szRegistryMultiLab4, Multimap_Labels[MP_VISUALGLIDE])) return;
  if (settings::read(sname, svalue, szRegistryMultiWpt0, Multimap_Flags_Waypoints[MP_MOVING])) return;
  if (settings::read(sname, svalue, szRegistryMultiWpt1, Multimap_Flags_Waypoints[MP_MAPTRK])) return;
  if (settings::read(sname, svalue, szRegistryMultiWpt2, Multimap_Flags_Waypoints[MP_MAPWPT])) return;
  if (settings::read(sname, svalue, szRegistryMultiWpt3, Multimap_Flags_Waypoints[MP_MAPASP])) return;
  if (settings::read(sname, svalue, szRegistryMultiWpt4, Multimap_Flags_Waypoints[MP_VISUALGLIDE])) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryMultiOvrT0, Multimap_Flags_Overlays_Text[MP_MOVING])) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryMultiOvrT1, Multimap_Flags_Overlays_Text[MP_MAPTRK])) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryMultiOvrT2, Multimap_Flags_Overlays_Text[MP_MAPWPT])) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryMultiOvrT3, Multimap_Flags_Overlays_Text[MP_MAPASP])) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryMultiOvrT4,
                Multimap_Flags_Overlays_Text[MP_VISUALGLIDE])) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryMultiOvrG0, Multimap_Flags_Overlays_Gauges[MP_MOVING])) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryMultiOvrG1, Multimap_Flags_Overlays_Gauges[MP_MAPTRK])) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryMultiOvrG2, Multimap_Flags_Overlays_Gauges[MP_MAPWPT])) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryMultiOvrG3, Multimap_Flags_Overlays_Gauges[MP_MAPASP])) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryMultiOvrG4,
                Multimap_Flags_Overlays_Gauges[MP_VISUALGLIDE])) {
    return;
  }
  if (settings::read(sname, svalue, szRegistryMultiSizeY1, Multimap_SizeY[MP_MAPTRK])) return;
  if (settings::read(sname, svalue, szRegistryMultiSizeY2, Multimap_SizeY[MP_MAPWPT])) return;
  if (settings::read(sname, svalue, szRegistryMultiSizeY3, Multimap_SizeY[MP_MAPASP])) return;
  if (settings::read(sname, svalue, szRegistryMultiSizeY4, Multimap_SizeY[MP_VISUALGLIDE])) return;
  if (settings::read(sname, svalue, szRegistryMultimap1, Multimap1)) return;
  if (settings::read(sname, svalue, szRegistryMultimap2, Multimap2)) return;
  if (settings::read(sname, svalue, szRegistryMultimap3, Multimap3)) return;
  if (settings::read(sname, svalue, szRegistryMultimap4, Multimap4)) return;
  if (settings::read(sname, svalue, szRegistryMultimap5, Multimap5)) return;
  if (settings::read(sname, svalue, szRegistryMMNorthUp1, MMNorthUp_Runtime[0])) return;
  if (settings::read(sname, svalue, szRegistryMMNorthUp2, MMNorthUp_Runtime[1])) return;
  if (settings::read(sname, svalue, szRegistryMMNorthUp3, MMNorthUp_Runtime[2])) return;
  if (settings::read(sname, svalue, szRegistryMMNorthUp4, MMNorthUp_Runtime[3])) return;
  if (settings::read(sname, svalue, szRegistryAspPermanent, AspPermanentChanged)) return;
  if (settings::read(sname, svalue, szRegistryFlarmDirection, iFlarmDirection)) return;
  if (settings::read(sname, svalue, szRegistryDrawFAI, Flags_DrawFAI_config)) return;
  if (settings::read(sname, svalue, szRegistryGearMode, GearWarningMode)) return;
  if (settings::read(sname, svalue, szRegistryGearAltitude, GearWarningAltitude)) return;
  if (settings::read(sname, svalue, szRegistryBottomMode, BottomMode)) return;
  if (settings::read(sname, svalue, szRegistryBigFAIThreshold, FAI28_45Threshold)) return;
  if (settings::read(sname, svalue, szRegistryDrawXC, Flags_DrawXC_config)) return;
  if (settings::read(sname, svalue, szRegistrySonarWarning, SonarWarning_Config)) return;
  if (settings::read(sname, svalue, szRegistryOverlay_TopLeft, Overlay_TopLeft)) return;
  if (settings::read(sname, svalue, szRegistryOverlay_TopMid, Overlay_TopMid)) return;
  if (settings::read(sname, svalue, szRegistryOverlay_TopRight, Overlay_TopRight)) return;
  if (settings::read(sname, svalue, szRegistryOverlay_TopDown, Overlay_TopDown)) return;

  if (settings::read(sname, svalue, szRegistryOverlay_LeftTop, Overlay_LeftTop)) {
    if (Overlay_LeftTop == 2) { // old Customs
      Overlay_LeftTop = GetInfoboxType(4) + 1000;
    }
    return;
  }
  if (settings::read(sname, svalue, szRegistryOverlay_LeftMid, Overlay_LeftMid)) {
    if (Overlay_LeftMid == 2) { // old Customs
      Overlay_LeftMid = GetInfoboxType(5) + 1000;
    }
    return;
  }
  if (settings::read(sname, svalue, szRegistryOverlay_LeftBottom, Overlay_LeftBottom)) {
    if (Overlay_LeftBottom == 2) { // old Customs
      Overlay_LeftBottom = GetInfoboxType(6) + 1000;
    }
    return;
  }
  if (settings::read(sname, svalue, szRegistryOverlay_LeftDown, Overlay_LeftDown)) {
    if (Overlay_LeftDown == 2) { // old Customs
      Overlay_LeftDown = GetInfoboxType(7) + 1000;
    }
    return;
  }
  if (settings::read(sname, svalue, szRegistryOverlay_RightTop, Overlay_RightTop)) {
    if (Overlay_RightTop == 2) { // old Customs
      Overlay_RightTop = GetInfoboxType(1) + 1000;
    }
    return;
  }
  if (settings::read(sname, svalue, szRegistryOverlay_RightMid, Overlay_RightMid)) {
    if (Overlay_RightMid == 2) { // old Customs
      Overlay_RightMid = GetInfoboxType(2) + 1000;
    }
    return;
  }
  if (settings::read(sname, svalue, szRegistryOverlay_RightBottom, Overlay_RightBottom)) {
    if (Overlay_RightBottom == 2) { // old Customs
      Overlay_RightBottom = GetInfoboxType(3) + 1000;
    }
    return;
  }
  if (settings::read(sname, svalue, szRegistryAdditionalContestRule, AdditionalContestRule)) return;

  if (settings::read(sname, svalue, szRegistryOverlay_Title, Overlay_Title)) return;

#ifdef _WGS84
  if (settings::read(sname, svalue, szRegistry_earth_model_wgs84, earth_model_wgs84)) return;
#endif
  if (settings::read(sname, svalue, szRegistryAutoContrast, AutoContrast)) return;
  if (settings::read(sname, svalue, szRegistryEnableAudioVario, EnableAudioVario)) return;

  if (SaveRuntime && !IsEmbedded()) {
    // Do NOT load resolution from profile, if we have requested a resolution from command line
    // And also, we can only load saved screen parameters from the default profile!
    // It does not work from custom profiles.
    if (!CommandResolution && isDefaultProfile) {
      if (settings::read(sname, svalue, szRegistryScreenSize, ScreenSize)) return;
      if (settings::read(sname, svalue, szRegistryScreenSizeX, ScreenSizeX)) return;
      if (settings::read(sname, svalue, szRegistryScreenSizeY, ScreenSizeY)) return;
    }
  }

  if (settings::read(sname, svalue, szRegistrySoundSwitch, EnableSoundModes)) return;

  if(tracking::LoadSettings(sname,svalue)) {
    return;
  }

#if TESTBENCH
  if (!strcmp(sname, "LKVERSION") && !strcmp(sname, "PROFILEVERSION")) {
    StartupStore(_T("... UNMANAGED PARAMETER inside profile: <%s>=<%s>\n"), sname, svalue);
  }
#endif
}

