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

double StrToDouble(const TCHAR *str, const TCHAR **endptr);

void PExtractParameter(TCHAR *Source, TCHAR *Destination, size_t dest_size, int DesiredFieldNumber);

template<size_t dest_size>
void PExtractParameter(TCHAR *Source, TCHAR (&Destination)[dest_size], int DesiredFieldNumber) {
	PExtractParameter(Source, Destination, dest_size, DesiredFieldNumber);
}

unsigned DoSunEphemeris(double lon, double lat);

void TrimRight(TCHAR* str);

// Parse string (new lines etc) and malloc the string
TCHAR* StringMallocParse(const TCHAR* old_string);

void LocalPath(TCHAR* buf, const TCHAR* file = TEXT("")) gcc_nonnull_all;
void LocalPath(TCHAR* buffer, const TCHAR* SubPath, const TCHAR* file) gcc_nonnull_all;

void SystemPath(TCHAR* buf, const TCHAR* file = TEXT("")) gcc_nonnull_all;
void SystemPath(TCHAR* buffer, const TCHAR* SubPath, const TCHAR* file) gcc_nonnull_all;

void RemoveFilePathPrefix(const TCHAR* szPrefix, TCHAR* szFilePath) gcc_nonnull_all;

const TCHAR *LKGetLocalPath();
const TCHAR *LKGetSystemPath();

void LK_tsplitpath(const TCHAR* path, TCHAR* drv, TCHAR* dir, TCHAR* name, TCHAR* ext);

void propGetFontSettingsFromString(const TCHAR *Buffer, LOGFONT* lplf);

int GetUTCOffset();

#if !defined(UNDER_CE) || defined(__linux__)
bool LK8000GetOpts(const TCHAR *MyCommandLine);
#endif

bool CheckRectOverlap(const RECT *rc1, const RECT *rc2);

void WeightOffset(double wload);
bool PolarWinPilot2XCSoar(double (&dPOLARV)[3], double (&dPOLARW)[3], double (&ww)[2]);
bool ReadWinPilotPolar();


void InitCustomHardware();
void DeInitCustomHardware();

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
bool UseContestEngine();
int  GetWaypointFileFormatType(const TCHAR* wextension);

// LK Utils
void LKBatteryManager();
void ChangeWindCalcSpeed(const int newspeed);
void GotoWaypoint(const int wpnum);
void ToggleBaroAltitude();

void InitAlarms();
void CheckAltitudeAlarms(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated);

void MasterTimeReset();
bool DoOptimizeRoute();
const TCHAR* WhatTimeIsIt();
bool UseAATTarget();
void OutOfMemory(const TCHAR *where, int line);

void CreateDirectoryIfAbsent(const TCHAR *filename);

RECT WindowResize(unsigned int x, unsigned int y);


void UpdateConfBB();
void UpdateConfIP();
void UpdateMultimapOrient();
void SetInitialModeTypes();

bool	InitLDRotary(ldrotary_s *buf);
void	InitWindRotary(windrotary_s *wbuf);

void	SetOverColorRef();
void	LKRunStartEnd(bool);

bool	LockMode(short lmode);
double	GetMacCready(int wpindex, short wpmode);

bool CheckMcTimer();
void UpdateMcTimer();

bool CheckSetMACCREADY(double value, DeviceDescriptor_t* Sender);
bool CheckSetBugs(double value, DeviceDescriptor_t* Sender);
bool CheckSetBallast(double val, DeviceDescriptor_t* Sender);

double CalculateLXBalastFactor(double Ballast);
double CalculateBalastFromLX(double Factor);

double CalculateLXBugs(double Bugs);
double CalculateBugsFromLX(double LXBug);

bool IsThermalBarVisible();

bool CheckClubVersion();
void ClubForbiddenMsg();

void TaskStartMessage();
void TaskFinishMessage();

void ToggleDrawTaskFAI();

#if USELKASSERT
void LK_tcsncpy_internal(TCHAR *dest, const TCHAR *src, const unsigned int numofchars,
			const unsigned int sizedest, const int line = 0, const TCHAR *filename = NULL);
#define LK_tcsncpy(dest, src, numofchars) {;LK_tcsncpy_internal(dest, src, numofchars, sizeof(dest), __LINE__, _T(__FILE__));}
#else
void LK_tcsncpy_internal(TCHAR *dest, const TCHAR *src, const unsigned int numofchars);
#define LK_tcsncpy(dest, src, numofchars) LK_tcsncpy_internal(dest, src, numofchars)
#endif

#endif
