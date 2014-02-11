/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Utils.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#if !defined(AFX_UTILS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_UTILS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


extern bool LockSettingsInFlight;
extern bool LoggerShortName;


#if defined(PNA) && defined(UNDER_CE)
bool SetBacklight();
bool SetSoundVolume();
#endif

#ifdef PNA
BOOL GetFontPath(TCHAR *pPos);
#endif
void StoreType(int Index,int InfoType);


void DistanceBearing(double lat1, double lon1, double lat2, double lon2,
                     double *Distance, double *Bearing);
double DoubleDistance(double lat1, double lon1, double lat2, double lon2,
		      double lat3, double lon3);

void SectorEndPoint(double StartLat, double StartLon, double  Radial, double Dist, double *EndLat, double *EndLon);
void CalculateNewPolarCoef(void);
void FindLatitudeLongitude(double Lat, double Lon, 
                           double Bearing, double Distance, 
                           double *lat_out, double *lon_out);
void ConvertFlightLevels(void);
BOOL PolygonVisible(const POINT *lpPoints, int nCount, RECT rc);
void ReadPort1Settings(LPTSTR szPort, DWORD *SpeedIndex, DWORD *Bit1Index);
void ReadPort2Settings(LPTSTR szPort, DWORD *SpeedIndex, DWORD *Bit2Index);
//void ReadPort3Settings(DWORD *PortIndex, DWORD *SpeedIndex, DWORD *Bit3Index);
void WritePort1Settings(DWORD PortIndex, DWORD SpeedIndex, DWORD Bit1Index);
void WritePort2Settings(DWORD PortIndex, DWORD SpeedIndex, DWORD Bit2Index);
//void WritePort3Settings(DWORD PortIndex, DWORD SpeedIndex, DWORD Bit3Index);



void ReadAssetNumber(void);
void WriteProfile(const TCHAR *szFile);
void ReadProfile(const TCHAR *szFile);
bool SetModelType();
bool SetModelName(DWORD Temp);
void ReadUUID(void);
void FormatWarningString(int Type, TCHAR *Name , AIRSPACE_ALT Base, AIRSPACE_ALT Top, TCHAR *szMessageBuffer, TCHAR *TileBuffer );
BOOL ReadString(ZZIP_FILE* zFile, int Max, TCHAR *String);
BOOL ReadStringX(FILE *fp, int Max, TCHAR *String);
bool ReadULine(ZZIP_FILE* fp, TCHAR *unicode, int maxChars);


void LatLon2Flat(double lon, double lat, int *scx, int *scy);

double StrToDouble(TCHAR *Source, TCHAR **Stop);
void PExtractParameter(TCHAR *Source, TCHAR *Destination, int DesiredFieldNumber);
void SaveWindToRegistry();
void LoadWindFromRegistry();
void SaveSoundSettings();
void ReadDeviceSettings(const int devIdx, TCHAR *Name);
void WriteDeviceSettings(const int devIdx, const TCHAR *Name);

WORD crcCalc(void *Buffer, size_t size);
void ExtractDirectory(TCHAR *Dest, TCHAR *Source);
double DoSunEphemeris(double lon, double lat);

void *bsearch(void *key, void *base0, size_t nmemb, size_t size, int (*compar)(const void *elem1, const void *elem2));

TCHAR *_tcstok_r(TCHAR *s, const TCHAR *delim, TCHAR **lasts);
TCHAR *strsep_r(TCHAR *s, const TCHAR *delim, TCHAR **lasts);

void SaveRegistryToFile(const TCHAR* szFile); 
void LoadRegistryFromFile(const TCHAR* szFile); 

/* =====================================================
   Interface Files !
   ===================================================== */

void StatusFileInit(void);

typedef struct {
	const TCHAR *key;		/* English key */
	const TCHAR *sound;		/* What sound entry to play */
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

void LocalPath(TCHAR* buf, const TCHAR* file = TEXT(""));
void LocalPathS(TCHAR* buf, const TCHAR* file = TEXT(""));
const TCHAR *LKGetLocalPath(void);

void ExpandLocalPath(TCHAR* filein);
void ContractLocalPath(TCHAR* filein);

void propGetFontSettings(const TCHAR *Name, LOGFONT* lplf);
void propGetFontSettingsFromString(const TCHAR *Buffer, LOGFONT* lplf);
#if 0
int propGetScaleList(double *List, size_t Size);
#endif

long GetUTCOffset(void);
int TextToLineOffsets(TCHAR* text, int* LineOffsets, int maxLines);
void RestoreRegistry(void);
void StoreRegistry(void);
void LK8000GetOpts();

bool CheckRectOverlap(const RECT *rc1, const RECT *rc2);
int MeasureCPULoad();

TCHAR* GetWinPilotPolarInternalName(int i);
void WeightOffset(double wload);

void InitCustomHardware(void);
void DeInitCustomHardware(void);

void OpenFLARMDetails();
void CloseFLARMDetails();
TCHAR* LookupFLARMCn(long id);
TCHAR* LookupFLARMDetails(long id);
int LookupFLARMDetails(TCHAR *cn);
bool AddFlarmLookupItem(int id, TCHAR *name, bool saveFile);
int LookupSecondaryFLARMId(int id);

double QNHAltitudeToStaticPressure(double alt);
double FindQNH(double alt_raw, double alt_known);
double AltitudeToQNHAltitude(double alt);
double AltitudeToQNEAltitude(double alt);
double StaticPressureToAltitude(double ps);
double AirDensity(double altitude);
double AirDensityRatio(double altitude);
double AirDensity(double hr, double temp, double absp);
double AirDensitySinkRate(double ias, double qnhaltitude);
double AirDensitySinkRate(double ias, double qnhaltitude, double gload);
double TrueAirSpeed( double delta_press, double hr, double temp, double abs_press );

double HexStrToDouble(TCHAR *Source, TCHAR **Stop);

// Fast convert from Hex string To integer
int HexStrToInt(TCHAR *Source);

///////////////////////////////////////////////////////////////////////
// Extract H, M, S from string like "HH:MM:SS"
//   Sec output parameter is optional
void StrToTime(LPCTSTR szString, int *Hour, int *Min, int *Sec = NULL);


unsigned long CheckFreeRam(void);
// check maximum allocatable heap block
unsigned long CheckMaxHeapBlock(void);

const TCHAR *TaskFileName(unsigned bufferLen, TCHAR buffer[]);
bool UseContestEngine(void);
int  GetWaypointFileFormatType(const TCHAR* wextension);

// LK Utils
void LKBatteryManager();
void LKSound(const TCHAR *lpName);
void ChangeWindCalcSpeed(const int newspeed);
bool LKRun(const TCHAR *prog, const int runmode, const DWORD dwaitime);
void GotoWaypoint(const int wpnum);
void ToggleBaroAltitude(void);
bool ReducedMapSize(void);

void InitAlarms(void);
bool CheckAlarms(unsigned short al);

void MasterTimeReset(void);
bool DoOptimizeRoute(void);
TCHAR * WhatTimeIsIt(void);
void OutOfMemory(const char *where, int line);

void MemCheckPoint();
void MemLeakCheck();
void MyCompactHeaps();
unsigned long FindFreeSpace(const TCHAR *path);
bool MatchesExtension(const TCHAR *filename, const TCHAR* extension);
BOOL PlayResource (const TCHAR* lpName);
void CreateDirectoryIfAbsent(const TCHAR *filename);

bool RotateScreen(short angle);

void UpdateConfBB(void);
void UpdateConfIP(void);
void UpdateMultimapOrient(void);
void SetInitialModeTypes(void);

bool	InitLDRotary(ldrotary_s *buf);
void	InitWindRotary(windrotary_s *wbuf);

void	SetOverColorRef();
TCHAR*  GetSizeSuffix(void);
void	LKRunStartEnd(bool);

bool	LockMode(short lmode);
double	GetMacCready(int wpindex, short wpmode);
void CheckSetMACCREADY(const double value);

double CheckSetBugs(double val);
double CheckSetBallast(double val);
bool   IsThermalBarVisible(void);

extern bool CheckClubVersion(void);
extern void ClubForbiddenMsg(void);

#if USELKASSERT
void LK_tcsncpy_internal(TCHAR *dest, const TCHAR *src, const unsigned int numofchars,
			const unsigned int sizedest, const int line = 0, const char *filename = NULL); 
#define LK_tcsncpy(dest, src, numofchars) {;LK_tcsncpy_internal(dest, src, numofchars, sizeof(dest), __LINE__, __FILE__);}
#else
void LK_tcsncpy_internal(TCHAR *dest, const TCHAR *src, const unsigned int numofchars);
#define LK_tcsncpy(dest, src, numofchars) LK_tcsncpy_internal(dest, src, numofchars)
#endif

#endif
