/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#include "ReplayLogger.h"
//#include "InputEvents.h"
#include "Utils.h"
#include <tchar.h>
#include <iostream>

using std::min;
using std::max;


//extern int NumLoggerBuffered;

CReplayLogger CReplayLogger::_instance;


/*
  ps = (1 u u^2 u^3)[0  1 0 0] p0
  [-t 0 t 0] p1
  [2t t-3 3-2t -t] p2
  [-t 2-t t-2 t] p3

*/

class CReplayLogger::CCatmullRomInterpolator {
  struct TInterpPoint {
    double lat;
    double lon;
    double alt;
    double t;
  };

  int _num;
  double _tzero;
  TInterpPoint _p[4];

  bool Ready() const { return _num == 4; }

public:
  CCatmullRomInterpolator() { Reset(); }

  void Reset();
  bool Update(double t, double lon, double lat, double alt);

  bool NeedData(double tthis) const { return !Ready() || _p[2].t <= tthis + 0.1; }

  double GetSpeed(double time) const;
  void Interpolate(double time, double &lon, double &lat, double &alt) const;
  double GetMinTime() const { return _p[0].t; }
  double GetMaxTime() const { return max(0.0, max(_p[0].t, max(_p[1].t, max(_p[2].t, _p[3].t)))); }
  double GetAverageTime() const;
};


void CReplayLogger::CCatmullRomInterpolator::Reset()
{
  _num = 0;
  for (int i=0; i<4; i++) {
    _p[i].t = 0;
  }
}


bool CReplayLogger::CCatmullRomInterpolator::Update(double t, double lon, double lat, double alt)
{
  if(_num<4)
    _num++;

  if(_p[3].t > t) {
    for(int i=0; i<4; i++) {
      _p[i].lat = lat;
      _p[i].lon = lon;
      _p[i].alt = alt;
      _p[i].t   = t;
    }
    return false;
  }

  for(int i=0; i<3; i++) {
    _p[i].lat = _p[i+1].lat;
    _p[i].lon = _p[i+1].lon;
    _p[i].alt = _p[i+1].alt;
    _p[i].t   = _p[i+1].t;
  }
  _p[3].lat = lat;
  _p[3].lon = lon;
  _p[3].alt = alt;
  _p[3].t   = t;

  return true; // 100827
}


double CReplayLogger::CCatmullRomInterpolator::GetSpeed(double time) const
{
  // if(Ready()) {
  //   double u = (time-_p[1].t) / (_p[2].t-_p[1].t);
  //   double s0;
  //   DistanceBearing(_p[0].lat, _p[0].lon, _p[1].lat, _p[1].lon, &s0, NULL);
  //   s0 /= _p[1].t - _p[0].t;
  //   double s1;
  //   DistanceBearing(_p[1].lat, _p[1].lon, _p[2].lat, _p[2].lon, &s1, NULL);
  //   s1 /= _p[2].t - _p[1].t;
  //   u = max(0.0, min(1.0,u));
  //   return s1*u + s0*(1.0-u);
  // }
  // else {
    return 0.0;
  // }
}


void CReplayLogger::CCatmullRomInterpolator::Interpolate(double time, double &lon, double &lat, double &alt) const
{
  if(!Ready()) {
    lon = _p[_num].lon;
    lat = _p[_num].lat;
    alt = _p[_num].alt;
    return;
  }
  double t = 0.98;
  double u = (time-_p[1].t)/(_p[2].t-_p[1].t);

  if(u < 0.0) {
    lat = _p[1].lat;
    lon = _p[1].lon;
    alt = _p[1].alt;
    return;
  }
  if(u > 1.0) {
    lat = _p[2].lat;
    lon = _p[2].lon;
    alt = _p[2].alt;
    return;
  }

  double u2 = u*u;
  double u3 = u2*u;
  double c[4]= {-t*u3+2*t*u2-t*u,
                (2-t)*u3+(t-3)*u2+1,
                (t-2)*u3+(3-2*t)*u2+t*u,
                t*u3-t*u2};
  /*
    double c[4] = {-t*u+2*t*u2-t*u3,
    1+(t-3)*u2+(2-t)*u3,
    t*u+(3-2*t)*u2+(t-2)*u3,
    -t*u2+t*u3};
  */

  lat = (_p[0].lat*c[0] + _p[1].lat*c[1] + _p[2].lat*c[2] + _p[3].lat*c[3]);
  lon = (_p[0].lon*c[0] + _p[1].lon*c[1] + _p[2].lon*c[2] + _p[3].lon*c[3]);
  alt = (_p[0].alt*c[0] + _p[1].alt*c[1] + _p[2].alt*c[2] + _p[3].alt*c[3]);
}


double CReplayLogger::CCatmullRomInterpolator::GetAverageTime() const
{
  double tav= 0;
  if(_num > 0) {
    for(int i=0; i<_num; i++) {
      tav += _p[i].t/_num;
    }
  }
  return tav;
}



CReplayLogger::CReplayLogger():
  _enabled(false), _timeScale(1.0), _updated(false)
{
}


bool CReplayLogger::ReadLine(unsigned bufferLen, TCHAR buffer[]) const
{
  static FILE *fp = NULL;
  if (!buffer) {
    if (fp) {
      fclose(fp);
      fp = NULL;
    }
    return false;
  }
  if (!fp) {
    if (_tcslen(_fileName)>0) {
      fp = _tfopen(_fileName, TEXT("rt"));
    }
  }
  if (fp==NULL) {
    return false;
  }

  if (fgetws(buffer, bufferLen, fp)==NULL) {
    _tcscat(buffer,TEXT("\0"));
    return false;
  }
  return true;
}


bool CReplayLogger::ScanBuffer(const TCHAR *line, double &time,
                               double &latitude, double &longitude, double &altitude) const
{
  int degLat=0, degLon=0;
  int minLat=0, minLon=0;
  TCHAR NoS, EoW;
  int iAltitude=0;
  int bAltitude=0;
  int hour=0;
  int minute=0;
  int second=0;

  int lfound=0;
  int found=0;

  if ((lfound =
       swscanf(line,
	       TEXT("B%02d%02d%02d%02d%05d%c%03d%05d%cA%05d%05d"),
	       &hour, &minute, &second,
	       &degLat, &minLat, &NoS, &degLon, &minLon,
	       &EoW, &iAltitude, &bAltitude
	       )) != EOF) {

    if (lfound==11) {
      latitude = degLat+minLat/60000.0;
      if (NoS==_T('S')) {
	latitude *= -1;
      }

      longitude = degLon+minLon/60000.0;
      if (EoW==_T('W')) {
	longitude *= -1;
      }
      if (bAltitude>0)
        altitude = bAltitude;
      else
        altitude = iAltitude;
      time = hour*3600+minute*60+second;
    }
  }

  TCHAR event[200];
  TCHAR misc[200];

  found = _stscanf(line,
		   TEXT("LPLT event=%[^ ] %[A-Za-z0-9 \\/().,]"),
		   event,misc);
  if (found>0) {
    // pt2Event fevent = InputEvents::findEvent(event);
    // if (fevent) {
    //   if (found==2) {
    //     TCHAR *mmisc = StringMallocParse(misc);
    //     fevent(mmisc);
    //     free(mmisc);
    //   } else {
    //     fevent(TEXT("\0"));
    //   }
    // }

  }
  return (lfound==11);
}


bool CReplayLogger::ReadPoint(double &time, double &latitude, double &longitude, double &altitude) const
{
  const unsigned BUFFER_LEN = 200;
  TCHAR buffer[BUFFER_LEN];
  while(ReadLine(BUFFER_LEN, buffer)) {
    if(ScanBuffer(buffer, time, latitude, longitude, altitude)) {
      for(CGPSHanlders::const_iterator it=_gpsHandlers.begin(); it!=_gpsHandlers.end(); ++it)
        (it->second)(it->first, time, latitude, longitude, altitude);
      return true;
    }
  }
  return false;
}


bool CReplayLogger::UpdateInternal()
{
  static bool init=true;

  if (!_enabled) {
    init = true;
    ReadLine(0, 0); // close file
    _enabled = true;
  }

  static CCatmullRomInterpolator cli;

  SYSTEMTIME st;
  GetLocalTime(&st);
  static double time_lstart = 0;
  static double time=0;

  if (init) {
    time_lstart = 0;
    time = (st.wHour*3600+st.wMinute*60+st.wSecond-time_lstart);
  }
  double deltatimereal;
  static double tthis=0;

  bool finished = false;

  double timelast = time;
  //  time = (st.wHour*3600+st.wMinute*60+st.wSecond-time_lstart);
  time += 1;
  deltatimereal = time-timelast;

  if (init) {
    time_lstart = time;
    //    time = 0;
    deltatimereal = 0;
    tthis = 0;
    cli.Reset();
  }

  tthis += _timeScale * deltatimereal;

  double mintime = cli.GetMinTime(); // li_lat.GetMinTime();
  if (tthis<mintime) { tthis = mintime; }

  // if need a new point
  while (cli.NeedData(tthis)&&(!finished)) {

    double t1, lat1, lon1, alt1;
    finished = !ReadPoint(t1, lat1, lon1, alt1);

    if (!finished && (t1>0)) {
	if (!cli.Update(t1, lon1, lat1, alt1)) {
		break;
	}
        _updated = true;
    }
  }

  if (!finished) {

    // double latX, lonX, altX, speedX, bearingX;
    // double latX1, lonX1, altX1;

    // cli.Interpolate(tthis, lonX, latX, altX);
    // cli.Interpolate(tthis+0.1, lonX1, latX1, altX1);

    // speedX = cli.GetSpeed(tthis);
    // DistanceBearing(latX, lonX, latX1, lonX1, NULL, &bearingX);

    // if ((speedX>0) && (latX != latX1) && (lonX != lonX1)) {

      // LockFlightData();
      // if (init) {
      //   flightstats.Reset();
      // }
      // GPS_INFO.Latitude = latX;
      // GPS_INFO.Longitude = lonX;
      // GPS_INFO.Speed = speedX;
      // GPS_INFO.TrackBearing = bearingX;
      // GPS_INFO.Altitude = altX;
      // #if NEWQNH
      // GPS_INFO.BaroAltitude = AltitudeToQNHAltitude(altX); // 100129
      // #else
      // GPS_INFO.BaroAltitude = altX;
      // #endif
      // GPS_INFO.Time = tthis;
      // UnlockFlightData();
    // } else {
      // This is required in case the integrator fails,
      // which can occur due to parsing faults
    //   tthis = cli.GetMaxTime();
    // }
  }

  // quit if finished.
  if (finished) {
    Stop();
  }
  init = false;

  return !finished;
}


void CReplayLogger::Filename(const TCHAR *name)
{
  if(!name) {
    return;
  }
  if (_tcscmp(_fileName, name) != 0) {
    _tcscpy(_fileName, name);
  }
}


void CReplayLogger::Register(void *user, FGPSHandler gpsHandler)
{
  _gpsHandlers.push_back(std::make_pair(user, gpsHandler));
}


void CReplayLogger::Start()
{
  if(_enabled) {
    Stop();
  }
  //  flightstats.Reset();
  if(!UpdateInternal()) {
    // MessageBoxX(
    //     // LKTOKEN  _@M201_ = "Could not open IGC file!"
    //     	MsgToken(201),
    //     // LKTOKEN  _@M305_ = "Flight replay"
    //     	MsgToken(305),
    //     	MB_OK| MB_ICONINFORMATION);
  }
}


void CReplayLogger::Stop()
{
  ReadLine(0, 0); // close the file
  if(_enabled) {
    // LockFlightData();
    // GPS_INFO.Speed = 0;
    // //    GPS_INFO.Time = 0;
    // UnlockFlightData();
  }
  _enabled = false;
}


bool CReplayLogger::Update()
{
  if (!_enabled)
    return false;

  _updated = false;
  _enabled = UpdateInternal();
  return _enabled;
}
