/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Utils.h,v 8.3 2010/12/16 14:44:47 root Exp root $
*/

#if !defined(AFX_UTILS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_UTILS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include <shlobj.h>
#include <math.h>
#include "Task.h"
#include "options.h"
#ifdef LKAIRSPACE
#include "LKAirspace.h"
#else
#include "Airspace.h"
#endif
#include <zzip/lib.h>


#define  POLARUSEWINPILOTFILE  6    // if this polat is selected use the winpilot file

extern const TCHAR szRegistryKey[];
extern const TCHAR szRegistrySpeedUnitsValue[];
extern const TCHAR szRegistryDistanceUnitsValue[];
extern const TCHAR szRegistryAltitudeUnitsValue[];
extern const TCHAR szRegistryLiftUnitsValue[];
extern const TCHAR szRegistryTaskSpeedUnitsValue[];
extern const TCHAR szRegistryDisplayUpValue[];
extern const TCHAR szRegistryDisplayText[];   
extern const TCHAR szRegistrySafetyAltitudeArrival[];
extern const TCHAR szRegistrySafetyAltitudeTerrain[];
extern const TCHAR szRegistrySafteySpeed[];
extern const TCHAR szRegistryWindCalcSpeed[];
extern const TCHAR szRegistryWindCalcTime[];
extern const TCHAR szRegistryFAISector[];
extern const TCHAR szRegistrySectorRadius[];
extern const TCHAR szRegistryPolarID[];
extern const TCHAR szRegistryWayPointFile[];
extern const TCHAR szRegistryAdditionalWayPointFile[];
extern const TCHAR szRegistryAirspaceFile[];
extern const TCHAR szRegistryAdditionalAirspaceFile[];
extern const TCHAR szRegistryAirfieldFile[];
extern const TCHAR szRegistryTopologyFile[];
extern const TCHAR szRegistryPolarFile[];
extern const TCHAR szRegistryTerrainFile[];
extern const TCHAR szRegistryLanguageFile[];
extern const TCHAR szRegistryStatusFile[];
extern const TCHAR szRegistryInputFile[];
extern const TCHAR szRegistryAltMode[];
extern const TCHAR szRegistrySafetyAltitudeMode[];
extern const TCHAR szRegistryClipAlt[];
extern const TCHAR szRegistryAltMargin[];
extern const TCHAR szRegistryRegKey[];
extern const TCHAR szRegistrySnailTrail[];
extern const TCHAR szRegistryDrawTopology[];
extern const TCHAR szRegistryDrawTerrain[];
extern const TCHAR szRegistryFinalGlideTerrain[];
extern const TCHAR szRegistryAutoWind[];
extern const TCHAR szRegistryStartLine[];
extern const TCHAR szRegistryStartRadius[];
extern const TCHAR szRegistryFinishLine[];
extern const TCHAR szRegistryFinishRadius[];
extern const TCHAR szRegistryAirspaceWarning[];
extern const TCHAR szRegistryAirspaceBlackOutline[];
extern const TCHAR szRegistryAirspaceFillType[];
extern const TCHAR szRegistryAirspaceOpacity[];
extern const TCHAR szRegistryWarningTime[];
extern const TCHAR szRegistryAcknowledgementTime[];
#ifdef LKAIRSPACE
extern const TCHAR szRegistryAirspaceWarningRepeatTime[];
extern const TCHAR szRegistryAirspaceWarningVerticalMargin[];
extern const TCHAR szRegistryAirspaceWarningDlgTimeout[];
extern const TCHAR szRegistryAirspaceWarningMapLabels[];
#endif
extern const TCHAR szRegistryCircleZoom[];
extern const TCHAR szRegistryWindUpdateMode[];        
extern const TCHAR szRegistryHomeWaypoint[];        
extern const TCHAR szRegistryAlternate1[];         // VENTA3
extern const TCHAR szRegistryAlternate2[];        
extern const TCHAR szRegistryTeamcodeRefWaypoint[];
extern const TCHAR szRegistryPilotName[];        
extern const TCHAR szRegistryAircraftType[];        
extern const TCHAR szRegistryAircraftRego[];        
extern const TCHAR szRegistryCompetitionClass[];        
extern const TCHAR szRegistryCompetitionID[];  
extern const TCHAR szRegistryLoggerID[];        
extern const TCHAR szRegistryLoggerShort[];        
extern const TCHAR szRegistryNettoSpeed[];        
extern const TCHAR szRegistryAutoBacklight[]; // VENTA4
extern const TCHAR szRegistryAutoSoundVolume[]; // VENTA4
extern const TCHAR szRegistryAircraftCategory[]; // VENTA4
extern const TCHAR szRegistryExtendedVisualGlide[]; // VENTA4
extern const TCHAR szRegistryLook8000[]; // VENTA5
extern const TCHAR szRegistryAltArrivMode[]; // VENTA11
extern const TCHAR szRegistryNewMap[]; // VENTA5
extern const TCHAR szRegistryIphoneGestures[];
extern const TCHAR szRegistryPollingMode[];
extern const TCHAR szRegistryLKVarioBar[];
extern const TCHAR szRegistryLKVarioVal[];
extern const TCHAR szRegistryOverlaySize[];
extern const TCHAR szRegistryBarOpacity[];
extern const TCHAR szRegistryFontRenderer[];
extern const TCHAR szRegistryActiveMap[]; 
extern const TCHAR szRegistryCheckSum[]; 
extern const TCHAR szRegistryBestWarning[];
extern const TCHAR szRegistryThermalBar[];
extern const TCHAR szRegistryTrackBar[];
extern const TCHAR szRegistryMcOverlay[];
extern const TCHAR szRegistryHideUnits[]; // VENTA5
extern const TCHAR szRegistryOutlinedTp[]; // VENTA5
extern const TCHAR szRegistryOverColor[];
extern const TCHAR szRegistryTpFilter[];
extern const TCHAR szRegistryMapBox[]; // VENTA6
extern const TCHAR szRegistryGlideBarMode[]; // VENTA6
extern const TCHAR szRegistryArrivalValue[]; // VENTA6
extern const TCHAR szRegistryNewMapDeclutter[]; // VENTA6
extern const TCHAR szRegistryDeclutterMode[]; // VENTA10
extern const TCHAR szRegistryAverEffTime[]; // VENTA6
extern const TCHAR szRegistryBgMapColor[]; 
extern const TCHAR szRegistryDebounceTimeout[];
extern const TCHAR szRegistryAppDefaultMapWidth[];
extern const TCHAR szRegistryAppIndLandable[];
extern const TCHAR szRegistryAppInverseInfoBox[];
extern const TCHAR szRegistryAppInfoBoxColors[];
extern const TCHAR szRegistryAppCompassAppearance[];
extern const TCHAR szRegistryAppInfoBoxBorder[];
extern const TCHAR szRegistryAppInfoBoxGeom[];
extern const TCHAR szRegistryAppInfoBoxModel[];
extern const TCHAR szRegistryGpsAltitudeOffset[];
extern const TCHAR szRegistryUseGeoidSeparation[];
extern const TCHAR szRegistryPressureHg[];
//extern const TCHAR szRegistryShortcutIbox[];
extern const TCHAR szRegistryCustomKeyTime[];
extern const TCHAR szRegistryCustomKeyModeCenter[];
extern const TCHAR szRegistryCustomKeyModeLeft[];
extern const TCHAR szRegistryCustomKeyModeRight[];
extern const TCHAR szRegistryCustomKeyModeAircraftIcon[];
extern const TCHAR szRegistryCustomKeyModeLeftUpCorner[];
extern const TCHAR szRegistryCustomKeyModeRightUpCorner[];
extern const TCHAR szRegistryAppAveNeedle[];
extern const TCHAR szRegistryAutoAdvance[];
extern const TCHAR szRegistryUTCOffset[];
extern const TCHAR szRegistryAutoZoom[];
extern const TCHAR szRegistryPGCruiseZoom[];
extern const TCHAR szRegistryPGClimbZoom[];
extern const TCHAR szRegistryAutoOrientScale[];
extern const TCHAR szRegistryPGNumberOfGates[];
extern const TCHAR szRegistryPGOpenTimeH[];
extern const TCHAR szRegistryPGOpenTimeM[];
extern const TCHAR szRegistryPGGateIntervalTime[];
extern const TCHAR szRegistryPGStartOut[];
extern const TCHAR szRegistryLKTopoZoomCat05[];
extern const TCHAR szRegistryLKTopoZoomCat10[];
extern const TCHAR szRegistryLKTopoZoomCat20[];
extern const TCHAR szRegistryLKTopoZoomCat30[];
extern const TCHAR szRegistryLKTopoZoomCat40[];
extern const TCHAR szRegistryLKTopoZoomCat50[];
extern const TCHAR szRegistryLKTopoZoomCat60[];
extern const TCHAR szRegistryLKTopoZoomCat70[];
extern const TCHAR szRegistryLKTopoZoomCat80[];
extern const TCHAR szRegistryLKTopoZoomCat90[];
extern const TCHAR szRegistryLKTopoZoomCat100[];
extern const TCHAR szRegistryLKTopoZoomCat110[];
extern const TCHAR szRegistryLKMaxLabels[];
extern const TCHAR szRegistryMenuTimeout[];
extern const TCHAR szRegistryLockSettingsInFlight[];
extern const TCHAR szRegistryTerrainContrast[];
extern const TCHAR szRegistryTerrainBrightness[];
extern const TCHAR szRegistryTerrainRamp[];
extern const TCHAR szRegistryEnableFLARMMap[];
extern const TCHAR szRegistrySnailTrail[];
extern const TCHAR szRegistryTrailDrift[];
extern const TCHAR szRegistryThermalLocator[];
extern const TCHAR szRegistryGliderScreenPosition[];
extern const TCHAR szRegistrySetSystemTimeFromGPS[];
extern const TCHAR szRegistryAutoForceFinalGlide[];

extern const TCHAR szRegistryFinishMinHeight[];
extern const TCHAR szRegistryStartMaxHeight[];
extern const TCHAR szRegistryStartMaxHeightMargin[];
extern const TCHAR szRegistryStartMaxSpeed[];
extern const TCHAR szRegistryStartMaxSpeedMargin[];
extern const TCHAR szRegistryStartHeightRef[];

extern const TCHAR szRegistryAlarmMaxAltitude1[];
extern const TCHAR szRegistryAlarmMaxAltitude2[];
extern const TCHAR szRegistryAlarmMaxAltitude3[];

extern const TCHAR szRegistryEnableNavBaroAltitude[];
extern const TCHAR szRegistryOrbiter[];
extern const TCHAR szRegistryShading[];
extern const TCHAR szRegistryOverlayClock[];
extern const TCHAR szRegistryLoggerTimeStepCruise[];
extern const TCHAR szRegistryLoggerTimeStepCircling[];
extern const TCHAR szRegistryConfBB1[];
extern const TCHAR szRegistryConfBB2[];
extern const TCHAR szRegistryConfBB3[];
extern const TCHAR szRegistryConfBB4[];
extern const TCHAR szRegistryConfBB5[];
extern const TCHAR szRegistryConfBB6[];
extern const TCHAR szRegistryConfBB7[];
extern const TCHAR szRegistryConfBB8[];
extern const TCHAR szRegistryConfBB9[];

extern const TCHAR szRegistryConfIP11[];
extern const TCHAR szRegistryConfIP12[];
extern const TCHAR szRegistryConfIP13[];
extern const TCHAR szRegistryConfIP14[];
extern const TCHAR szRegistryConfIP15[];
extern const TCHAR szRegistryConfIP16[];
extern const TCHAR szRegistryConfIP21[];
extern const TCHAR szRegistryConfIP22[];
extern const TCHAR szRegistryConfIP23[];
extern const TCHAR szRegistryConfIP24[];
extern const TCHAR szRegistryConfIP31[];
extern const TCHAR szRegistryConfIP32[];


extern const TCHAR szRegistrySafetyMacCready[];
extern const TCHAR szRegistryAutoMcMode[];
extern const TCHAR szRegistryWaypointsOutOfRange[];
extern const TCHAR szRegistryEnableExternalTriggerCruise[];
extern const TCHAR szRegistryFAIFinishHeight[];
extern const TCHAR szRegistryOLCRules[];
extern const TCHAR szRegistryHandicap[];
extern const TCHAR szRegistrySnailWidthScale[];
extern const TCHAR szRegistryLatLonUnits[];
#if USERLEVEL
extern const TCHAR szRegistryUserLevel[];
#endif
extern const TCHAR szRegistryDisableAutoLogger[];
extern const TCHAR szRegistryMapFile[];
extern const TCHAR szRegistryBallastSecsToEmpty[];
extern const TCHAR szRegistryUseCustomFonts[];
extern const TCHAR szRegistryFontInfoWindowFont[]; 
extern const TCHAR szRegistryFontTitleWindowFont[]; 
extern const TCHAR szRegistryFontMapWindowFont[];
extern const TCHAR szRegistryFontTitleSmallWindowFont[];
extern const TCHAR szRegistryFontMapWindowBoldFont[];
extern const TCHAR szRegistryFontCDIWindowFont[];
extern const TCHAR szRegistryFontMapLabelFont[];
extern const TCHAR szRegistryFontStatisticsFont[];

extern bool LockSettingsInFlight;
extern bool LoggerShortName;

BOOL GetFromRegistry(const TCHAR *szRegValue, DWORD *pPos);

BOOL DelRegistryKey(const TCHAR *szRegistryKey); // VENTA2-ADDON delregistrykey
#ifdef PNA
void CleanRegistry(); // VENTA2-ADDON cleanregistrykeyA
bool SetBacklight(); // VENTA4-ADDON for PNA 
bool SetSoundVolume(); // VENTA4-ADDON for PNA 
#endif

HRESULT SetToRegistry(const TCHAR *szRegValue, DWORD Pos);
HRESULT SetToRegistry(const TCHAR *szRegValue, bool bVal);	// JG
HRESULT SetToRegistry(const TCHAR *szRegValue, int nVal);	// JG
BOOL GetRegistryString(const TCHAR *szRegValue, TCHAR *pPos, DWORD dwSize);
#ifdef PNA
BOOL GetFontPath(TCHAR *pPos);
void CreateRecursiveDirectory(TCHAR *fontpath);
#endif
HRESULT SetRegistryString(const TCHAR *szRegValue, const TCHAR *Pos);
void ReadRegistrySettings(void);
void SetRegistryColour(int i, DWORD c);
void SetRegistryBrush(int i, DWORD c);
void SetRegistryAirspacePriority(int i);
void SetRegistryAirspaceMode(int i);
int GetRegistryAirspaceMode(int i);
void StoreType(int Index,int InfoType);
void irotate(int &xin, int &yin, const double &angle);
void irotatescale(int &xin, int &yin, const double &angle, const double &scale,
                  double &x, double &y);
void protate(POINT &pin, const double &angle);
void protateshift(POINT &pin, const double &angle, const int &x, const int &y);
void rotate(double &xin, double &yin, const double &angle);
void frotate(float &xin, float &yin, const float &angle);
void rotatescale(double &xin, double &yin, const double &angle, const double &scale);
void frotatescale(float &xin, float &yin, const float &angle, const float &scale);

void DistanceBearing(double lat1, double lon1, double lat2, double lon2,
                     double *Distance, double *Bearing);
double DoubleDistance(double lat1, double lon1, double lat2, double lon2,
		      double lat3, double lon3);

double Reciprocal(double InBound);
double BiSector(double InBound, double OutBound);
double HalfAngle(double Angle0, double Angle1);
void SectorEndPoint(double StartLat, double StartLon, double  Radial, double Dist, double *EndLat, double *EndLon);
void CalculateNewPolarCoef(void);
void FindLatitudeLongitude(double Lat, double Lon, 
                           double Bearing, double Distance, 
                           double *lat_out, double *lon_out);
void ConvertFlightLevels(void);
BOOL PolygonVisible(const POINT *lpPoints, int nCount, RECT rc);
void ReadPort1Settings(DWORD *PortIndex, DWORD *SpeedIndex, DWORD *Bit1Index);
void ReadPort2Settings(DWORD *PortIndex, DWORD *SpeedIndex, DWORD *Bit2Index);
//void ReadPort3Settings(DWORD *PortIndex, DWORD *SpeedIndex, DWORD *Bit3Index);
void WritePort1Settings(DWORD PortIndex, DWORD SpeedIndex, DWORD Bit1Index);
void WritePort2Settings(DWORD PortIndex, DWORD SpeedIndex, DWORD Bit2Index);
//void WritePort3Settings(DWORD PortIndex, DWORD SpeedIndex, DWORD Bit3Index);
int  Circle(HDC hdc, long x, long y, int radius, RECT rc, bool clip=false,
            bool fill=true);
int Segment(HDC hdc, long x, long y, int radius, RECT rc, 
	    double start,
	    double end,
            bool horizon= false);
// VENTA3 DrawArc
int DrawArc(HDC hdc, long x, long y, int radius, RECT rc, 
	    double start,
	    double end);
void ReadAssetNumber(void);
void WriteProfile(const TCHAR *szFile);
void ReadProfile(const TCHAR *szFile);
bool SetModelType();
bool SetModelName(DWORD Temp);
double ScreenAngle(int x1, int y1, int x2, int y2);
void ReadUUID(void);
void FormatWarningString(int Type, TCHAR *Name , AIRSPACE_ALT Base, AIRSPACE_ALT Top, TCHAR *szMessageBuffer, TCHAR *TileBuffer );
BOOL ReadString(ZZIP_FILE* zFile, int Max, TCHAR *String);
BOOL ReadString(HANDLE hFile, int Max, TCHAR *String);
BOOL ReadStringX(FILE *fp, int Max, TCHAR *String);
bool ReadULine(ZZIP_FILE* fp, TCHAR *unicode, int maxChars);

// Fast trig functions
void InitSineTable(void);

extern double COSTABLE[4096];
extern double SINETABLE[4096];
extern double INVCOSINETABLE[4096];
extern int ISINETABLE[4096];
extern int ICOSTABLE[4096];

bool AngleInRange(double Angle0, double Angle1, double x, bool is_signed=false);
double AngleLimit180(double theta);
double AngleLimit360(double theta);

#ifdef __MINGW32__
#define DEG_TO_INT(x) ((unsigned short)(int)((x)*(65536.0/360.0)))>>4
#else
#define DEG_TO_INT(x) ((unsigned short)((x)*(65536.0/360.0)))>>4
#endif

#define invfastcosine(x) INVCOSINETABLE[DEG_TO_INT(x)]
#define ifastcosine(x) ICOSTABLE[DEG_TO_INT(x)]
#define ifastsine(x) ISINETABLE[DEG_TO_INT(x)]
#define fastcosine(x) COSTABLE[DEG_TO_INT(x)]
#define fastsine(x) SINETABLE[DEG_TO_INT(x)]

double StrToDouble(TCHAR *Source, TCHAR **Stop);
void PExtractParameter(TCHAR *Source, TCHAR *Destination, int DesiredFieldNumber);
void SaveWindToRegistry();
void LoadWindFromRegistry();
void SaveSoundSettings();
void ReadDeviceSettings(const int devIdx, TCHAR *Name);
void WriteDeviceSettings(const int devIdx, const TCHAR *Name);

unsigned int isqrt4(unsigned long val);


WORD crcCalc(void *Buffer, size_t size);
void ExtractDirectory(TCHAR *Dest, TCHAR *Source);
double DoSunEphemeris(double lon, double lat);

void *bsearch(void *key, void *base0, size_t nmemb, size_t size, int (*compar)(const void *elem1, const void *elem2));
TCHAR *strtok_r(TCHAR *s, TCHAR *delim, TCHAR **lasts);


void ResetInfoBoxes(void);

void SaveRegistryToFile(const TCHAR* szFile); 
void LoadRegistryFromFile(const TCHAR* szFile); 

/* =====================================================
   Interface Files !
   ===================================================== */

void ReadStatusFile(void);
void StatusFileInit(void);
void _init_Status(int num);

typedef struct {
	TCHAR *key;		/* English key */
	TCHAR *sound;		/* What sound entry to play */
	TCHAR *nmea_gps;		/* NMEA Sentence - to GPS serial */
	TCHAR *nmea_vario;		/* NMEA Sentence - to Vario serial */
	bool doStatus;
	bool doSound;
	int delay_ms;		/* Delay for DoStatusMessage */
	int iFontHeightRatio;	// TODO - not yet used
	bool docenter;		// TODO - not yet used
	int *TabStops;		// TODO - not yet used
	int disabled;		/* Disabled - currently during run time */
} StatusMessageSTRUCT;

typedef struct {
	TCHAR *key;
	TCHAR *text;
} GetTextSTRUCT;



// Parse string (new lines etc) and malloc the string
TCHAR* StringMallocParse(TCHAR* old_string);

void LocalPath(TCHAR* buf, const TCHAR* file = TEXT(""), int loc = CSIDL_PERSONAL);
void LocalPathS(char* buf, const TCHAR* file = TEXT(""), int loc = CSIDL_PERSONAL);

void ExpandLocalPath(TCHAR* filein);
void ContractLocalPath(TCHAR* filein);

void ConvertTToC(CHAR* pszDest, const TCHAR* pszSrc);
void ConvertCToT(TCHAR* pszDest, const CHAR* pszSrc);

void propGetFontSettings(TCHAR *Name, LOGFONT* lplf);
void propGetFontSettingsFromString(TCHAR *Buffer, LOGFONT* lplf);
int propGetScaleList(double *List, size_t Size);

long GetUTCOffset(void);
int TextToLineOffsets(TCHAR* text, int* LineOffsets, int maxLines);
void RestoreRegistry(void);
void StoreRegistry(void);
void LK8000GetOpts(LPTSTR CommandLine);

#if TOPOFASTLABEL
bool CheckRectOverlap(const RECT *rc1, const RECT *rc2);
#else
bool CheckRectOverlap(RECT rc1, RECT rc2);
#endif
int MeasureCPULoad();

TCHAR* GetWinPilotPolarInternalName(int i);
void WeightOffset(double wload);

TCHAR *LKgethelptext(const TCHAR *TextIn);
bool LKloadtext(void);
TCHAR *LKGetText(const TCHAR *TextIn);


void SetSourceRectangle(RECT fromRect);
RECT WINAPI DrawWireRects(LPRECT lprcTo, UINT nMilliSecSpeed);

void OpenFLARMDetails();
void CloseFLARMDetails();
TCHAR* LookupFLARMCn(long id);
TCHAR* LookupFLARMDetails(long id);
int LookupFLARMDetails(TCHAR *cn);
bool AddFlarmLookupItem(int id, TCHAR *name, bool saveFile);
int LookupSecondaryFLARMId(int id);

double FindQNH(double alt_raw, double alt_known);
double AltitudeToQNHAltitude(double alt);
double StaticPressureToAltitude(double ps);
double AirDensity(double altitude);
double AirDensityRatio(double altitude);

double HexStrToDouble(TCHAR *Source, TCHAR **Stop); 

unsigned long CheckFreeRam(void);
// check maximum allocatable heap block
unsigned long CheckMaxHeapBlock(void);

const TCHAR *TaskFileName(unsigned bufferLen, TCHAR buffer[]);
bool UseContestEngine(void);
int  GetWaypointFileFormatType(const wchar_t* wextension);

// LK Utils
void LKBatteryManager();
void LKSound(const TCHAR *lpName);
int  RescaleWidth(const int rWidth);
void ChangeWindCalcSpeed(const int newspeed);
bool LKRun(const TCHAR *prog, const int runmode, const DWORD dwaitime);
void GotoWaypoint(const int wpnum);
void ToggleBaroAltitude(void);
bool ReducedMapSize(void);

void InitAlarms(void);
bool CheckAlarms(unsigned short al);

void MemCheckPoint();
void MemLeakCheck();
void MyCompactHeaps();
unsigned long FindFreeSpace(const TCHAR *path);
bool MatchesExtension(const TCHAR *filename, const TCHAR* extension);
BOOL PlayResource (const TCHAR* lpName);
void CreateDirectoryIfAbsent(TCHAR *filename);

bool InterfaceTimeoutZero(void);
void InterfaceTimeoutReset(void);
bool InterfaceTimeoutCheck(void);

#ifdef __cplusplus
extern "C"{
#endif

bool FileExists(TCHAR *FileName);

#ifdef __cplusplus
}
#endif


//2^36 * 1.5,  (52-_shiftamt=36) uses limited precisicion to floor
//16.16 fixed point representation,

// =================================================================================
// Real2Int
// =================================================================================
inline int Real2Int(double val)
{
#if (WINDOWS_PC>0)
  val += 68719476736.0*1.5;
  return *((long*)&val) >> 16; 
#else
  return (int)val;
#endif
}


// =================================================================================
// Real2Int
// =================================================================================
inline int Real2Int(float val)
{
#if (WINDOWS_PC>0)
  return Real2Int ((double)val);
#else
  return (int)val;
#endif
}


inline int iround(double i) {
    return Real2Int(floor(i+0.5));
}

inline long lround(double i) {
    return (long)(floor(i+0.5));
}

inline unsigned int CombinedDivAndMod(unsigned int &lx) {
  unsigned int ox = lx & 0xff;
  // JMW no need to check max since overflow will result in 
  // beyond max dimensions
  lx = lx>>8;
  return ox;
}

bool RotateScreen(void);

int GetTextWidth(HDC hDC, TCHAR *text);
void ExtTextOutClip(HDC hDC, int x, int y, TCHAR *text, int width);
void UpdateConfBB(void);
void UpdateConfIP(void);
void SetInitialModeTypes(void);

#endif
