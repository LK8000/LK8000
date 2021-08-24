/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Utils.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#if !defined(AFX_UTILS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_UTILS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_


struct FlarmId;

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

double StrToDouble(const TCHAR *str, const TCHAR **endptr);

void PExtractParameter(TCHAR *Source, TCHAR *Destination, size_t dest_size, int DesiredFieldNumber);

template<size_t dest_size>
void PExtractParameter(TCHAR *Source, TCHAR (&Destination)[dest_size], int DesiredFieldNumber) {
	PExtractParameter(Source, Destination, dest_size, DesiredFieldNumber);
}

void SaveWindToRegistry();
void LoadWindFromRegistry();
void SaveSoundSettings();
void ReadDeviceSettings(const int devIdx, TCHAR *Name);
void WriteDeviceSettings(const int devIdx, const TCHAR *Name);

WORD crcCalc(void *Buffer, size_t size);
void ExtractDirectory(TCHAR *Dest, TCHAR *Source);
unsigned DoSunEphemeris(double lon, double lat);

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

void LK_tsplitpath(const TCHAR* path, TCHAR* drv, TCHAR* dir, TCHAR* name, TCHAR* ext);

void propGetFontSettingsFromString(const TCHAR *Buffer, LOGFONT* lplf);
#if 0
int propGetScaleList(double *List, size_t Size);
#endif

int GetUTCOffset();

#if !defined(UNDER_CE) || defined(__linux__)
bool LK8000GetOpts(const TCHAR *MyCommandLine);
#endif

bool CheckRectOverlap(const RECT *rc1, const RECT *rc2);
int MeasureCPULoad();

void WeightOffset(double wload);
bool PolarWinPilot2XCSoar(double (&dPOLARV)[3], double (&dPOLARW)[3], double (&ww)[2]);
bool ReadWinPilotPolar(void);


void InitCustomHardware(void);
void DeInitCustomHardware(void);

double QNHAltitudeToStaticPressure(double alt);
double StaticPressureToQNHAltitude(double ps);

double QNEAltitudeToStaticPressure(double alt);
double StaticPressureToQNEAltitude(double ps);

double QNEAltitudeToQNHAltitude(double alt);
double QNHAltitudeToQNEAltitude(double alt);

double FindQNH(double alt_raw, double alt_known);

double AirDensity(double qne_altitude);
double AirDensityRatio(double qne_altitude);
double AirDensity(double hr, double temp, double absp);
double AirDensitySinkRate(double ias, double qnhaltitude);
double AirDensitySinkRate(double ias, double qnhaltitude, double gload);
double TrueAirSpeed( double delta_press, double hr, double temp, double abs_press );
double TrueAirSpeed( double ias, double qne_altitude);
double IndicatedAirSpeed( double tas, double qne_altitude);

double HexStrToDouble(TCHAR *Source, TCHAR **Stop);

// Fast convert from Hex string To integer
uint8_t HexDigit(TCHAR c);
int HexStrToInt(const TCHAR *Source);

///////////////////////////////////////////////////////////////////////
// Extract H, M, S from string like "HH:MM:SS"
//   Sec output parameter is optional
void StrToTime(LPCTSTR szString, int *Hour, int *Min, int *Sec = NULL);


const TCHAR *AngleToWindRose(int angle);

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
const TCHAR* WhatTimeIsIt(void);
bool UseAATTarget(void);
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
void	LKRunStartEnd(bool);

bool	LockMode(short lmode);
double	GetMacCready(int wpindex, short wpmode);
void CheckSetMACCREADY(const double value);

double CheckSetBugs(double val);
double CheckSetBallast(double val);
double CalculateLXBalastFactor(double Ballast);
double CalculateBalastFromLX(double Factor);

double CalculateLXBugs(double Bugs);
double CalculateBugsFromLX(double LXBug);

bool   IsThermalBarVisible(void);

extern bool CheckClubVersion(void);
extern void ClubForbiddenMsg(void);

void TaskStartMessage(void);
void TaskFinishMessage(void);

void ToggleDrawTaskFAI(void);

#if USELKASSERT
void LK_tcsncpy_internal(TCHAR *dest, const TCHAR *src, const unsigned int numofchars,
			const unsigned int sizedest, const int line = 0, const TCHAR *filename = NULL);
#define LK_tcsncpy(dest, src, numofchars) {;LK_tcsncpy_internal(dest, src, numofchars, sizeof(dest), __LINE__, _T(__FILE__));}
#else
void LK_tcsncpy_internal(TCHAR *dest, const TCHAR *src, const unsigned int numofchars);
#define LK_tcsncpy(dest, src, numofchars) LK_tcsncpy_internal(dest, src, numofchars)
#endif

#endif
