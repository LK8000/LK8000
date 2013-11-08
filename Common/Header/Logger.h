#if !defined(AFX_LOGGER_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_LOGGER_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
void DoLogger(TCHAR *strAssetNumber);
void StartLogger(void);
void LogPoint(double Lattitude, double Longditude, double Altitude,
              double BaroAltitude);
void AddDeclaration(double Lattitude, double Longditude, TCHAR *ID);
void StartDeclaration(int numturnpoints);
void EndDeclaration(void);
void LoggerHeader(void);
void LoggerNote(const TCHAR *text);
void LoggerDeviceDeclare();
void EW_Strings(double Lattitude, double Longditude, TCHAR *ID);
void EW_Download(TCHAR *strAssetNumber);

void UpdateLogBook(bool welandedforsure);
bool UpdateLogBookTXT(bool welandedforsure);
bool UpdateLogBookCSV(bool welandedforsure);
bool UpdateLogBookLST(bool welandedforsure);
void ResetLogBook(void);


extern bool DeclaredToDevice;
bool CheckDeclaration(void);

class ReplayLogger {
 public:
  static bool Update(void);
  static void Stop(void);
  static void Start(void);
  static TCHAR* GetFilename(void);
  static void SetFilename(const TCHAR *name);
  static bool IsEnabled(void);
  static double TimeScale;
 private:
  static bool UpdateInternal(void);
  static bool ReadLine(TCHAR *buffer);
  static bool Enabled;
  static bool ScanBuffer(TCHAR *buffer, double *Time, double *Latitude,
			 double *Longitude, double *Altitude);
  static bool ReadPoint(double *Time, 
			double *Latitude, 
			double *Longitude, 
			double *Altitude);
  static TCHAR FileName[MAX_PATH+1];
};

bool LoggerClearFreeSpace();
void StopLogger(void);
bool IGCWriteRecord(char *szIn);
bool LoggerGActive();
#define MAX_IGC_BUFF 255

#if LOGFRECORD
bool LogFRecordToFile(int SatelliteIDs[], short Hour, short Minute, short Second, bool bAlways);
bool LogFRecord(int SatelliteIDs[], bool bAlways ) ;
void SetFRecordLastTime(double dTime);
double GetFRecordLastTime(void);
void ResetFRecord(void);
#endif

#endif


