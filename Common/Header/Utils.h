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


#ifdef PNA
bool SetBacklight();
bool SetSoundVolume();
#endif

#ifdef PNA
BOOL GetFontPath(TCHAR *pPos);
void CreateRecursiveDirectory(TCHAR *fontpath);
#endif
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
int  roundupdivision(int a, int b);
double LowPassFilter(double y_last, double x_in, double fact);


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

bool AngleInRange(double Angle0, double Angle1, double x, bool is_signed=false);
double AngleLimit180(double theta);
double AngleLimit360(double theta);
void LatLon2Flat(double lon, double lat, int *scx, int *scy);

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
TCHAR *strsep_r(TCHAR *s, TCHAR *delim, TCHAR **lasts);

void SaveRegistryToFile(const TCHAR* szFile); 
void LoadRegistryFromFile(const TCHAR* szFile); 

/* =====================================================
   Interface Files !
   ===================================================== */

void StatusFileInit(void);

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

void LocalPath(TCHAR* buf, const TCHAR* file = TEXT(""));
void LocalPathS(char* buf, const TCHAR* file = TEXT(""));
TCHAR *LKGetLocalPath(void);

void ExpandLocalPath(TCHAR* filein);
void ContractLocalPath(TCHAR* filein);

void ConvertTToC(CHAR* pszDest, const TCHAR* pszSrc);
void ConvertCToT(TCHAR* pszDest, const CHAR* pszSrc);

void propGetFontSettings(TCHAR *Name, LOGFONT* lplf);
void propGetFontSettingsFromString(TCHAR *Buffer, LOGFONT* lplf);
#if 0
int propGetScaleList(double *List, size_t Size);
#endif

long GetUTCOffset(void);
int TextToLineOffsets(TCHAR* text, int* LineOffsets, int maxLines);
void RestoreRegistry(void);
void StoreRegistry(void);
void LK8000GetOpts();

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
TCHAR *MsgToken(const unsigned int tindex);

void InitCustomHardware(void);
void DeInitCustomHardware(void);


void SetSourceRectangle(RECT fromRect);
RECT WINAPI DrawWireRects(LPRECT lprcTo, UINT nMilliSecSpeed);

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
double TrueAirSpeed( double delta_press, double hr, double temp, double abs_press );

double HexStrToDouble(TCHAR *Source, TCHAR **Stop);

// Fast convert from Hex string To integer
int HexStrToInt(TCHAR *Source);

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

void MasterTimeReset(void);
bool DoOptimizeRoute(void);
TCHAR * WhatTimeIsIt(void);
void OutOfMemory(char *where, int line);

void MemCheckPoint();
void MemLeakCheck();
void MyCompactHeaps();
unsigned long FindFreeSpace(const TCHAR *path);
bool MatchesExtension(const TCHAR *filename, const TCHAR* extension);
BOOL PlayResource (const TCHAR* lpName);
void CreateDirectoryIfAbsent(TCHAR *filename);

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

bool RotateScreen(short angle);

int GetTextWidth(HDC hDC, TCHAR *text);
void ExtTextOutClip(HDC hDC, int x, int y, TCHAR *text, int width);
void UpdateConfBB(void);
void UpdateConfIP(void);
void SetInitialModeTypes(void);

bool	InitLDRotary(ldrotary_s *buf);
void	InitWindRotary(windrotary_s *wbuf);

void	SetOverColorRef();
TCHAR*  GetSizeSuffix(void);
void	LKRunStartEnd(bool);

bool	LockMode(short lmode);
double	GetMacCready(int wpindex, short wpmode);

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
