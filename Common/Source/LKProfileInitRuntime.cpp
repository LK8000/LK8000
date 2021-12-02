/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKProfiles.h"
#include "McReady.h"
#include "Modeltype.h"


//
// Init runtime variables using _Config variables
// This is needed after loading a new profile, and
// also on startup after loading a reset configuration.
//
// LK8000 is (normally!) separating config values from runtime values.
// For example, ActiveMap can be configured disabled by default in System Config,
// but enabled at runtime with a button or a customkey. However the configuration
// will still be "disabled" and saved as disabled in profile.
// It is important to keep runtime and config variables separated, if a config
// variable can be changed with a button out of System Config!
//
// Be careful, like AdjustVariables this function is called twice or more times on startup
//
void LKProfileInitRuntime() {

  #if TESTBENCH
  StartupStore(_T("... LKProfileInitRuntime\n"));
  #endif

  // Todo: use _Config values for files, and then we can compare if they changed
  WAYPOINTFILECHANGED = TRUE;
  TERRAINFILECHANGED = TRUE;
  TOPOLOGYFILECHANGED = TRUE;
  AIRSPACEFILECHANGED = TRUE;
  AIRFIELDFILECHANGED = TRUE;
  POLARFILECHANGED = TRUE;


  //
  // Runtime from Config
  //

  Appearance.InverseInfoBox	= InverseInfoBox_Config;
  AutoAdvance			= AutoAdvance_Config;
  AutoMcMode			= AutoMcMode_Config;
  BgMapColor			= BgMapColor_Config;
  EnableNavBaroAltitude		= EnableNavBaroAltitude_Config;
  Orbiter			= Orbiter_Config;
  Shading			= Shading_Config;
  TerrainRamp			= TerrainRamp_Config;
  TrailActive			= TrailActive_Config;
  UseTotalEnergy		= UseTotalEnergy_Config;
  AutoWindMode			= AutoWindMode_Config;
  MapWindow::EnableTrailDrift	= EnableTrailDrift_Config;
  AltitudeMode			= AltitudeMode_Config;
  OutlinedTp			= OutlinedTp_Config;
  BUGS				= BUGS_Config;
  SonarWarning			= SonarWarning_Config;

  Flags_DrawFAI = Flags_DrawFAI_config;
  Flags_DrawXC = Flags_DrawXC_config;

  MapWindow::zoom.AutoZoom(AutoZoom_Config);

  CALCULATED_INFO.AutoMacCready = AutoMacCready_Config==true?1:0;
  DisplayOrientation = DisplayOrientation_Config;
  MapWindow::SetAutoOrientation(); // reset old autoorientation

  MapWindow::GliderScreenPositionY = MapWindow::GliderScreenPosition;

  //
  // Units
  //
  Units::NotifyUnitChanged(); // set unit strings


  SetOverColorRef();

  InitActiveGate();


  if ( ISPARAGLIDER ) {
  	gTaskType=TSK_GP;
  } else {
	gTaskType=TSK_DEFAULT;
  }

  if ( ISPARAGLIDER || ISCAR ) {
	// paragliders can takeoff at 5kmh ground with some head wind!
	TakeOffSpeedThreshold=1.39;	// 5kmh
	if (ISCAR) TakeOffSpeedThreshold=0.83; // 3 kmh
  } else {
	TakeOffSpeedThreshold=11.12; // 40kmh
  }

  //
  // ModelType specials for PNAs
  //

#if TESTBENCH
    StartupStore(TEXT(". Loading %s settings"), ModelType::get_name(GlobalModelType));
#endif

  if (GlobalModelType == ModelType::GENERIC ) {

  } 
  else if (GlobalModelType == ModelType::HP31X ) {
	  DeviceNeedClipping = true;
    // key transcoding for this one
  }
  else if (GlobalModelType == ModelType::PN6000 ) {
	  // key transcoding for this one
  }
  else if (GlobalModelType == ModelType::PNA_MIO ) {
  	// currently no special settings from MIO but need to handle hw keys
  }
  else if (GlobalModelType == ModelType::NOKIA_500 ) {
  	// key transcoding is made
  }
  else if (GlobalModelType == ModelType::MEDION_P5 ) {
  	DeviceNeedClipping=true;
  } 
  else if (GlobalModelType == ModelType::PNA_NAVIGON ) {
  	DeviceNeedClipping=true;
  }
  else if (GlobalModelType == ModelType::BTKA ) {

  }
  else if (GlobalModelType == ModelType::BTKB ) {

  }
  else if (GlobalModelType == ModelType::BTKC ) {

  }
  else if (GlobalModelType == ModelType::BTK1 ) {

  }
  else if (GlobalModelType == ModelType::BTK2 ) {

  }
  else if (GlobalModelType == ModelType::BTK3 ) {

  }

  LKalarms[0].triggervalue=(int)AlarmMaxAltitude1/1000;
  LKalarms[1].triggervalue=(int)AlarmMaxAltitude2/1000;
  LKalarms[2].triggervalue=(int)AlarmMaxAltitude3/1000;

  UpdateConfBB();
  UpdateConfIP();
  UpdateMultimapOrient();

}
