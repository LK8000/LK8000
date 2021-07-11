/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Enums.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef ENUMS_H
#define ENUMS_H

// 0: not started at all
// 1: everything is alive
// 2: done first draw
// 3: normal operation
typedef enum {psInitInProgress=0, psInitDone=1, psFirstDrawDone=2, psNormalOp=3} StartupState_t;

// This could be also used for PDA in landscape..
typedef enum{
  ssnone=0,
  ss240x320,
  ss240x400,
  ss272x480,
  ss480x640,
  ss480x800,
  ss600x800,
  sslandscape, //  <landscape=portrait modes, >landscape=landscape modes
  ss320x240,
  ss400x240,
  ss480x234,
  ss480x272,
  ss640x480,
  ss800x480,
  ss800x600
}ScreenSize_t;

typedef enum{
  amcFinalGlide=0,
  amcAverageClimb,
  amcFinalAndClimb,
  amcEquivalent,
} AutoMacCready_t;

typedef enum{
  ctBestCruiseTrackDefault=0,
  ctBestCruiseTrackAltA,
}BestCruiseTrack_t;

typedef enum{
  wpLandableDefault=0,
  wpLandableAltA,
}IndLandable_t;

typedef struct{
  int Height;
  int AscentHeight;
  int CapitalHeight;
}FontHeightInfo_t;

typedef enum{
  bit8N1=0,
  bit7E1,
}BitIndex_t;


typedef enum{
	apImPnaGeneric=0,
	apImPnaHp31x,
	apImPnaMedionP5,
	apImPnaMio,
	apImPnaNokia500,
	apImPnaPn6000,
	apImPnaNavigon,
	apImPnaFuntrek,
	apImPnaRoyaltek3200,
        apImPnaMinimap,
	apImPnaGenericBTKA,
	apImPnaGenericBTKB,
	apImPnaGenericBTKC,
	apImPnaGenericBTK1,
	apImPnaGenericBTK2,
	apImPnaGenericBTK3
}InfoBoxModelAppearance_t;

typedef enum{
	umGlider=0,
	umParaglider,
	umCar,
        umGAaircraft,
} AircraftCategory_t;
typedef enum{
	amDisabled=0,
	amEnabled,
} ActiveMap_t;

// warning, this must follow the ALTA_ settings in utils2.h
typedef enum{
	aamMC=0,
	aamMC0,
	aamMCS,
} AltArrivMode_t;
typedef enum{
	csumDisabled=0,
	csumEnabled,
} CheckSum_t;
typedef enum {
	iphDisabled=0,
	iphEnabled,
} IphoneGestures_t;
typedef enum {
	pollmodeDisabled=0,
	pollmodeEnabled,
} PollingMode_t;
typedef enum {
	vBarDisabled=0,
	vBarVarioColor,
	vBarVarioMono,
	vBarVarioRB,
	vBarVarioGR,
} LKVarioBar_t;
typedef enum {
	vValVarioVario,
	vValVarioNetto,
	vValVarioSoll,
} LKVarioVal_t;

typedef enum{
	otDisabled=0,
	otLandable,
	otAll,
} OutlinedTp_t;
typedef enum{
	OcWhite=0,
	OcBlack,
	OcBlue,
	OcYellow,
	OcGreen,
	OcOrange,
	OcCyan,
	OcMagenta,
	OcGrey,
	OcDarkGrey,
	OcDarkWhite,
	OcAmber,
	OcLightGreen,
	OcPetrol,
} OverColor_t;
typedef enum{
	TfNoLandables=0,
	TfAll,
	TfTps,
} TpFilter_t;

typedef enum{
	mbUnboxedNoUnit=0,
	mbUnboxed,
	mbBoxedNoUnit,
	mbBoxed,
} MapBox_t;
// Declutter waypoints on the map
// disabled=no decluttering. Up to 1.21g, disabled low and high (0 1 2)
typedef enum{
	dmDisabled=0,
	dmLow,
	dmMedium,
	dmHigh,
	dmVeryHigh,
} DeclutterMode_t;

typedef enum{
	huDisabled=0,
	huEnabled,
} HideUnits_t;
typedef enum{
	gbDisabled=0,
	gbNextTurnpoint,
	gbFinish,
} GlideBarMode_t;
typedef enum{
	ae3seconds=0,
	ae5seconds,
	ae10seconds,
	ae15seconds,
	ae30seconds,
	ae45seconds,
	ae60seconds,
	ae90seconds,
	ae2minutes,
	ae3minutes,
} AverEffTime_t;
typedef enum{
	avAltitude=0,
	avGR,
	avGR_Altitude,
	avNone,
	// avDistance,
} ArrivalValue_t;

typedef enum{
	mm_disabled=0,
	mm_enabled_normal,
	mm_enabled_northup,
} Multimap_t;

typedef enum {
    mbOk,
    mbOkCancel,
    mbYesNo,
    mbYesNoCancel,
    mbAbortRetryIgnore,
    mbRetryCancel,
} MsgType_t;

typedef enum {
    IdOk = 1,
    IdCancel,
    IdYes,
    IdNo,
    IdAbort,
    IdRetry,
    IdIgnore
} MsgReturn_t;

#endif
