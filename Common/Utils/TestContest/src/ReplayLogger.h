/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#ifndef __REPLAYLOGGER_H__
#define __REPLAYLOGGER_H__

#include <windows.h>
#include <deque>


class CReplayLogger {
public:
  typedef void (*FGPSHandler)(void *user, unsigned time, double latitude, double longitude, short altitude);

private:
  class CCatmullRomInterpolator;
  typedef std::pair<void *, FGPSHandler> TGPSHandlerData;
  typedef std::deque<TGPSHandlerData> CGPSHanlders;

  static CReplayLogger _instance;

  bool _enabled;
  double _timeScale;
  TCHAR _fileName[MAX_PATH+1];
  bool _updated;
  CGPSHanlders _gpsHandlers;

  CReplayLogger();
  CReplayLogger(const CReplayLogger &);             /**< @brief Disallowed */
  CReplayLogger &operator=(const CReplayLogger &);  /**< @brief Disallowed */
  ~CReplayLogger() {}

  bool ReadLine(unsigned bufferLen, TCHAR buffer[]) const;
  bool ScanBuffer(const TCHAR *line, double &time, double &latitude, double &longitude, double &altitude) const;
  bool ReadPoint(double &time, double &latitude, double &longitude, double &altitude) const;
  bool UpdateInternal();

public:
  static CReplayLogger &Instance() { return _instance; }

  void Filename(const TCHAR *name);
  const TCHAR *Filename() const { return _fileName; }
  void TimeScale(double timeScale) { _timeScale = timeScale; }
  double TimeScale() const { return _timeScale; }

  void Register(void *user, FGPSHandler gpsHandler);

  void Start();
  void Stop();
  bool IsEnabled() const { return _enabled; }

  bool Update();
  bool Updated() { return _updated; }
};


#endif /* __REPLAYLOGGER_H__ */
