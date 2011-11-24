/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Utils.h"
#include "McReady.h"
#include "Modeltype.h"

#if NEWPROFILES

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
void LKProfileInitRuntime(void) {

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

  ActiveMap	=	ActiveMap_Config;
  Appearance.InverseInfoBox = InverseInfoBox_Config;
  AutoAdvance	=	AutoAdvance_Config;
  AutoMcMode	=	AutoMcMode_Config;
  BgMapColor	=	BgMapColor_Config;
  EnableNavBaroAltitude	= EnableNavBaroAltitude_Config;
  EnableTerrain =	EnableTerrain_Config;
  EnableTopology=	EnableTopology_Config;
  Orbiter	= 	Orbiter_Config;
  Shading	=	Shading_Config;
  TerrainRamp	=	TerrainRamp_Config;
  TrailActive	=	TrailActive_Config;
  UseTotalEnergy=	UseTotalEnergy_Config;

  // MapWindow::zoom.AutoZoom(AutoZoom_Config); // Not yet using a global!

  CALCULATED_INFO.AutoMacCready = AutoMacCready_Config==true?1:0;
  DisplayOrientation = DisplayOrientation_Config;
  MapWindow::SetAutoOrientation(true); // reset old autoorientation

  MapWindow::GliderScreenPositionY = MapWindow::GliderScreenPosition;

  GlobalModelType=Appearance.InfoBoxModel;

  //
  // Units
  //
  switch(SpeedUnit_Config) {
    case 0 :
	SPEEDMODIFY = TOMPH;
	break;
    case 1 :
	SPEEDMODIFY = TOKNOTS;
	break;
    case 2 :
    default:
	SPEEDMODIFY = TOKPH;
	break;
  }
  switch(TaskSpeedUnit_Config) {
    case 0 :
      TASKSPEEDMODIFY = TOMPH;
      break;
    case 1 :
      TASKSPEEDMODIFY = TOKNOTS;
      break;
    case 2 :
    default:
      TASKSPEEDMODIFY = TOKPH;
      break;
  }
  switch(DistanceUnit_Config) {
    case 0 : DISTANCEMODIFY = TOMILES; break;
    case 1 : DISTANCEMODIFY = TONAUTICALMILES; break;
    default:
    case 2 : DISTANCEMODIFY = TOKILOMETER; break;
  }
  switch(AltitudeUnit_Config) {
    case 0 : ALTITUDEMODIFY = TOFEET; break;
    default:
    case 1 : ALTITUDEMODIFY = TOMETER; break;
  }
  switch(LiftUnit_Config) {
    case 0 : LIFTMODIFY = TOKNOTS; break;
    case 2 : LIFTMODIFY = TOKNOTS; break;
    default:
    case 1 : LIFTMODIFY = TOMETER; break;
  }
  Units::NotifyUnitChanged(); // set unit strings


  SetOverColorRef();


  PGOpenTime=((PGOpenTimeH*60)+PGOpenTimeM)*60;
  PGCloseTime=PGOpenTime+(PGGateIntervalTime*PGNumberOfGates*60);
  if (PGCloseTime>86399) PGCloseTime=86399; // 23:59:59

  if ( ISPARAGLIDER ) {
	AverEffTime = (AverEffTime_t)ae15seconds;
	AATEnabled=TRUE;
  } else {
	AverEffTime = (AverEffTime_t)ae60seconds;
	AATEnabled=FALSE;
  }

  //
  // ModelType specials for PNAs
  //
  #if (WINDOWSPC<1)
  if (GlobalModelType == MODELTYPE_PNA_HP31X ) {
	needclipping=true;
	// key transcoding for this one
	StartupStore(TEXT(". Loading HP31X settings%s"),NEWLINE);
  }
  else
  if (GlobalModelType == MODELTYPE_PNA_PN6000 ) {
	// key transcoding for this one
  }
  else
  if (GlobalModelType == MODELTYPE_PNA_MIO ) {
	StartupStore(TEXT(". Loading MIO settings%s"),NEWLINE);
	// currently no special settings from MIO but need to handle hw keys
  }
  else
  if (GlobalModelType == MODELTYPE_PNA_NOKIA_500 ) {
	StartupStore(TEXT(". Loading Nokia500 settings%s"),NEWLINE);
	// key transcoding is made
  }
  else
  if (GlobalModelType == MODELTYPE_PNA_MEDION_P5 ) {
	StartupStore(TEXT(".Loading Medion settings%s"),NEWLINE);
	needclipping=true;
  }
  if (GlobalModelType == MODELTYPE_PNA_NAVIGON ) {
	StartupStore(TEXT(".Loading Navigon settings%s"),NEWLINE);
	needclipping=true;
  }
  else
  if (GlobalModelType == MODELTYPE_PNA_PNA ) {
	StartupStore(TEXT(". Loading default PNA settings%s"),NEWLINE);
  }
  else
	StartupStore(TEXT(". No special regsets for this device%s"),NEWLINE);

  #endif // ModelType specials



  LKalarms[0].triggervalue=(int)AlarmMaxAltitude1/1000;
  LKalarms[1].triggervalue=(int)AlarmMaxAltitude2/1000;
  LKalarms[2].triggervalue=(int)AlarmMaxAltitude3/1000;

  UpdateConfBB();
  UpdateConfIP();

}


#endif // NEWPROFILES
