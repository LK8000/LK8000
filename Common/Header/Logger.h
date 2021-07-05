#if !defined(AFX_LOGGER_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_LOGGER_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

void StartLogger(void);
void LogPoint(const NMEA_INFO& info);

void LoggerDeviceDeclare();

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
bool LoggerGActive();


#define MAX_IGC_BUFF 255

#endif
