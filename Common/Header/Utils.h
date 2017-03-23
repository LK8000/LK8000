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
#endif

#ifdef PNA
BOOL GetFontPath(TCHAR *pPos);
#endif
void StoreType(int Index,int InfoType);


void SectorEndPoint(double StartLat, double StartLon, double  Radial, double Dist, double *EndLat, double *EndLon);
void CalculateNewPolarCoef(void);
void FindLatitudeLongitude(double Lat, double Lon,
                           double Bearing, double Distance,
                           double *lat_out, double *lon_out);
void ConvertFlightLevels(void);
BOOL PolygonVisible(const POINT *lpPoints, int nCount, RECT rc);
void ReadPortSettings(int idx, LPTSTR szPort, unsigned *SpeedIndex, BitIndex_t *Bit1Index);

//void ReadPort3Settings(DWORD *PortIndex, DWORD *SpeedIndex, DWORD *Bit3Index);


void ReadAssetNumber(void);
void WriteProfile(const TCHAR *szFile);
void ReadProfile(const TCHAR *szFile);
bool SetModelType();
void ReadUUID(void);
void FormatWarningString(int Type, TCHAR *Name , AIRSPACE_ALT Base, AIRSPACE_ALT Top, TCHAR *szMessageBuffer, TCHAR *TileBuffer );


/**
 * charset is used for detect encoding of file, use variable with charset::unknown for first ReadString call.
 * next ReadString call need use charset returned by fist call.
 *  - with that, file are loaded as utf8 file until only valid utf8 char is read and
 *    as latin1 (iso8859) if invalid utf8 char is read.
 */
enum charset {
    unknown,
    utf8,
    latin1
};

BOOL ReadString(ZZIP_FILE* zFile, int Max, TCHAR *String, charset& cs);
BOOL ReadStringX(FILE *fp, int Max, TCHAR *String, charset& cs);

bool ReadULine(ZZIP_FILE* fp, TCHAR *unicode, int maxChars);


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
void TrimRight(TCHAR* str);

void SaveRegistryToFile(const TCHAR* szFile);
void LoadRegistryFromFile(const TCHAR* szFile);

/* =====================================================
   Interface Files !
   ===================================================== */

typedef struct {
	TCHAR *key;
	TCHAR *text;
} GetTextSTRUCT;



// Parse string (new lines etc) and malloc the string
TCHAR* StringMallocParse(const TCHAR* old_string);

void LocalPath(TCHAR* buf, const TCHAR* file = TEXT("")) gcc_nonnull_all;
void LocalPath(TCHAR* buffer, const TCHAR* SubPath, const TCHAR* file) gcc_nonnull_all;

void SystemPath(TCHAR* buf, const TCHAR* file = TEXT("")) gcc_nonnull_all;
void SystemPath(TCHAR* buffer, const TCHAR* SubPath, const TCHAR* file) gcc_nonnull_all;

void RemoveFilePathPrefix(const TCHAR* szPrefix, TCHAR* szFilePath) gcc_nonnull_all;

const TCHAR *LKGetLocalPath(void);
const TCHAR *LKGetSystemPath(void);

void propGetFontSettingsFromString(const TCHAR *Buffer, LOGFONT* lplf);
#if 0
int propGetScaleList(double *List, size_t Size);
#endif

long GetUTCOffset(void);

#if !defined(UNDER_CE) || defined(__linux__)
bool LK8000GetOpts(const TCHAR *MyCommandLine);
#endif

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


const TCHAR *TaskFileName(unsigned bufferLen, TCHAR buffer[]);
bool UseContestEngine(void);
int  GetWaypointFileFormatType(const TCHAR* wextension);

// LK Utils
void LKBatteryManager();
void ChangeWindCalcSpeed(const int newspeed);
void GotoWaypoint(const int wpnum);
void ToggleBaroAltitude(void);
bool ReducedMapSize(void);

void InitAlarms(void);
void CheckAltitudeAlarms(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated);

void MasterTimeReset(void);
bool DoOptimizeRoute(void);
TCHAR * WhatTimeIsIt(void);
void OutOfMemory(const TCHAR *where, int line);

void CreateDirectoryIfAbsent(const TCHAR *filename);

RECT WindowResize(unsigned int x, unsigned int y);


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
			const unsigned int sizedest, const int line = 0, const TCHAR *filename = NULL);
#define LK_tcsncpy(dest, src, numofchars) {;LK_tcsncpy_internal(dest, src, numofchars, sizeof(dest), __LINE__, _T(__FILE__));}
#else
void LK_tcsncpy_internal(TCHAR *dest, const TCHAR *src, const unsigned int numofchars);
#define LK_tcsncpy(dest, src, numofchars) LK_tcsncpy_internal(dest, src, numofchars)
#endif

#endif
