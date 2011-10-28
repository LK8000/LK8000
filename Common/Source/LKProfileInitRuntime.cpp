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
// Init runtime variables from configuration
//
void LKProfileInitRuntime(void) {

  //
  // Runtime from Config
  //
  MapWindow::zoom.AutoZoom(AutoZoom_Config);
  TerrainRamp = TerrainRamp_Config;
  AutoMcMode = AutoMcMode_Config;
  // AutoMacCready in calculations.h is an int, should be a bool
  CALCULATED_INFO.AutoMacCready = AutoMacCready_Config==true?1:0;
  UseTotalEnergy = UseTotalEnergy_Config;
  DisplayOrientation = DisplayOrientation_Config;
  MapWindow::SetAutoOrientation(true); // reset old autoorientation

  MapWindow::GliderScreenPositionY = MapWindow::GliderScreenPosition;

  if (UTCOffset>12*3600) {
    UTCOffset-= 24*3600;
  }

  // Units
  switch(Speed)
    {
    case 0 :
      SPEEDMODIFY = TOMPH;
      break;
    case 1 :
      SPEEDMODIFY = TOKNOTS;
      break;
    case 2 :
      SPEEDMODIFY = TOKPH;
      break;
    }
  switch(TaskSpeed)
    {
    case 0 :
      TASKSPEEDMODIFY = TOMPH;
      break;
    case 1 :
      TASKSPEEDMODIFY = TOKNOTS;
      break;
    case 2 :
      TASKSPEEDMODIFY = TOKPH;
      break;
    }
  switch(Distance)
    {
    case 0 : DISTANCEMODIFY = TOMILES; break;
    case 1 : DISTANCEMODIFY = TONAUTICALMILES; break;
    case 2 : DISTANCEMODIFY = TOKILOMETER; break;
    }
  switch(Altitude)
    {
    case 0 : ALTITUDEMODIFY = TOFEET; break;
    case 1 : ALTITUDEMODIFY = TOMETER; break;
    }
  switch(Lift)
    {
    case 0 : LIFTMODIFY = TOKNOTS; break;
    case 1 : LIFTMODIFY = TOMETER; break;
    case 2 : LIFTMODIFY = TOKNOTS; break;
    }

  Units::NotifyUnitChanged();

  MapWindow::SetAirSpaceFillType((MapWindow::EAirspaceFillType) MapWindow::AirspaceFillType);
  MapWindow::SetAirSpaceOpacity(MapWindow::AirspaceOpacity);

  SetOverColorRef();


  PGOpenTime=((PGOpenTimeH*60)+PGOpenTimeM)*60;
  PGCloseTime=PGOpenTime+(PGGateIntervalTime*PGNumberOfGates*60);
  if (PGCloseTime>86399) PGCloseTime=86399; // 23:59:59

  // NO!
  if ( ISPARAGLIDER ) {
	AverEffTime = (AverEffTime_t)ae15seconds;
	AATEnabled=TRUE;
  } else {
	AverEffTime = (AverEffTime_t)ae2minutes;
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




  LKalarms[0].triggervalue=0;
  LKalarms[1].triggervalue=0;
  LKalarms[2].triggervalue=0;

  UpdateConfBB();
  UpdateConfIP();

}








#endif // NEWPROFILES
