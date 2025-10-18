/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"
#include "InputEvents.h"
#include "Dialogs.h"
#include <time.h>
#include "NavFunctions.h"
#include "LocalPath.h"

extern int NumLoggerBuffered;

// #define DEBUG_REPLAY 1

bool ReplayLogger::ReadLine(TCHAR *buffer) {
  static FILE *fp = NULL;
  if (!buffer) {
    if (fp) {
      fclose(fp);
      fp= NULL;
    }
    return false;
  }
  if (!fp) {
    if (_tcslen(FileName)>0) {

      TCHAR szFilePath[MAX_PATH];
      LocalPath(szFilePath, _T(LKD_LOGS), FileName);

      fp = _tfopen(szFilePath, TEXT("rt"));
    }
  }
  if (fp==NULL) {
    return false;
  }

  if (_fgetts(buffer, 200, fp)==NULL) {
    buffer[0] = TEXT('\0');
    return false;
  }
  return true;
}


bool ReplayLogger::ScanBuffer(TCHAR *buffer, double *Time, double *Latitude,
			      double *Longitude, double *Altitude)
{
  int DegLat, DegLon;
  int MinLat, MinLon;
  TCHAR NoS, EoW;
  int iAltitude;
  int bAltitude;
  int Hour=0;
  int Minute=0;
  int Second=0;

  int lfound=0;
  int found=0;

  if ((lfound =
       _stscanf(buffer,
	       TEXT("B%02d%02d%02d%02d%05d%c%03d%05d%cA%05d%05d"),
	       &Hour, &Minute, &Second,
	       &DegLat, &MinLat, &NoS, &DegLon, &MinLon,
	       &EoW, &iAltitude, &bAltitude
	       )) != EOF) {

    if (lfound==11) {
      *Latitude = DegLat+MinLat/60000.0;
      if (NoS==_T('S')) {
	*Latitude *= -1;
      }

      *Longitude = DegLon+MinLon/60000.0;
      if (EoW==_T('W')) {
	*Longitude *= -1;
      }
      if (bAltitude>0) *Altitude = bAltitude;
	else *Altitude = iAltitude;
      *Time = Hour*3600+Minute*60+Second;
    }
  }

  TCHAR event[200];
  TCHAR misc[200];

  found = _stscanf(buffer,
		   TEXT("LPLT event=%[^ ] %[A-Za-z0-9 \\/().,]"),
		   event,misc);
  if (found>0) {
    pt2Event fevent = InputEvents::findEvent(event);
    if (fevent) {
      if (found==2) {
	TCHAR *mmisc = StringMallocParse(misc);
	if (mmisc!=NULL) {
		fevent(mmisc);
		free(mmisc);
	}
      } else {
	fevent(TEXT("\0"));
      }
    }

  }
  return (lfound==11 && (*Latitude!=0 && *Longitude!=0) );
}


bool ReplayLogger::ReadPoint(double *Time,
			     double *Latitude,
			     double *Longitude,
			     double *Altitude)
{
  TCHAR buffer[200]={0};
/*
  // This is creating problems with the interpolator and calculations based on differential times
  // such as variometer derived from altitude differences. Probably due to vario lowpass filters.
  while(ReadLine(buffer)) {
    if(ScanBuffer(buffer, Time, Latitude, Longitude, Altitude))
      return true;
  }
  return false;
*/
  bool found=false;
  while(ReadLine(buffer) &&!found) {
    if(ScanBuffer(buffer, Time, Latitude, Longitude, Altitude))
      found=true;
  }
  return found;
}



TCHAR ReplayLogger::FileName[MAX_PATH+1];
bool ReplayLogger::Enabled = false;
double ReplayLogger::TimeScale = 1.0;

bool ReplayLogger::IsEnabled(void) {
  return Enabled;
}


typedef struct _LOGGER_INTERP_POINT
{
  double lat;
  double lon;
  double alt;
  double t;
} LOGGER_INTERP_POINT;

/*
  ps = (1 u u^2 u^3)[0  1 0 0] p0
  [-t 0 t 0] p1
  [2t t-3 3-2t -t] p2
  [-t 2-t t-2 t] p3

*/

class CatmullRomInterpolator {
public:
  CatmullRomInterpolator() {
    Reset();
  }
  void Reset() {
    num=0;
    for (int i=0; i<4; i++) {
      p[i].t= 0;
    }
  }

  LOGGER_INTERP_POINT p[4];

  bool Update(double t, double lon, double lat, double alt) {
    if (num<4) { num++; }
	if ( p[3].t >t ) {
		for (int i=0; i<4; i++) {
			p[i].lat = lat;
			p[i].lon = lon;
			p[i].alt = alt;
			p[i].t   = t;
		}
		return false;
	}

    for (int i=0; i<3; i++) {
      p[i].lat = p[i+1].lat;
      p[i].lon = p[i+1].lon;
      p[i].alt = p[i+1].alt;
      p[i].t   = p[i+1].t;
    }
    p[3].lat = lat;
    p[3].lon = lon;
    p[3].alt = alt;
    p[3].t   = t;

	return true; // 100827
  }

  bool Ready() {
    return (num==4);
  }
  double GetSpeed(double time) {
    if (p[1].t == p[0].t ) return 0.0;
    if (p[2].t == p[1].t ) return 0.0;
    if (Ready()) {
      double u= (time-p[1].t)/(p[2].t-p[1].t);
      double s0;
      DistanceBearing(p[0].lat, p[0].lon,
                      p[1].lat, p[1].lon, &s0, NULL);

      s0/= (p[1].t-p[0].t);
      double s1;
      DistanceBearing(p[1].lat, p[1].lon,
                      p[2].lat, p[2].lon, &s1, NULL);
      s1/= (p[2].t-p[1].t);
      u = max(0.0,min(1.0,u));
      return s1*u+s0*(1.0-u);
    } else {
      return 0.0;
    }
  }
  void Interpolate(double time, double *lon, double *lat, double *alt) {
    if (!Ready()) {
      *lon = p[num].lon;
      *lat = p[num].lat;
      *alt = p[num].alt;
      return;
    }
    double t=0.98;
    BUGSTOP_LKASSERT( p[2].t != p[1].t );
    if ( p[2].t == p[1].t ) {
      *lat = p[1].lat;
      *lon = p[1].lon;
      *alt = p[1].alt;
      return;
    }

    double u= (time-p[1].t)/(p[2].t-p[1].t);

    if (u<0.0) {
      *lat = p[1].lat;
      *lon = p[1].lon;
      *alt = p[1].alt;
      return;
    }
    if (u>1.0) {
      *lat = p[2].lat;
      *lon = p[2].lon;
      *alt = p[2].alt;
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

    *lat = (p[0].lat*c[0] + p[1].lat*c[1] + p[2].lat*c[2] + p[3].lat*c[3]);
    *lon = (p[0].lon*c[0] + p[1].lon*c[1] + p[2].lon*c[2] + p[3].lon*c[3]);
    *alt = (p[0].alt*c[0] + p[1].alt*c[1] + p[2].alt*c[2] + p[3].alt*c[3]);

  }
  double GetMinTime(void) {
    return p[0].t;
  }
  double GetMaxTime(void) {
    return max(0.0,max(p[0].t, max(p[1].t, max(p[2].t, p[3].t))));
  }
  double GetAverageTime(void) {
    double tav= 0;
    if (num>0) {
      for (int i=0; i<num; i++) {
        tav += p[i].t/num;
      }
    }
    return tav;
  }
  bool NeedData(double tthis) {
    return (!Ready())||(p[2].t<=tthis+0.1);
  }
private:
  int num;
};


// IGC time of replay running log
double ReplayTime=0;

bool ReplayLogger::UpdateInternal(void) {
  static bool init=true;

  if (!Enabled) {
    init = true;
    ReadLine(NULL); // close file
    Enabled = true;
  }

  static CatmullRomInterpolator cli;

  time_t rawtime;
  struct tm tm_temp= {};
  struct tm * timeinfo;
  time (&rawtime);
  timeinfo = gmtime_r(&rawtime, &tm_temp);

  static double time_lstart = 0;

  if (init) {
    time_lstart = 0;
  }
  static double time=0;
  double deltatimereal;

  bool finished = false;

  double timelast = time;

  time = (timeinfo->tm_hour*3600+timeinfo->tm_min*60+timeinfo->tm_sec-time_lstart);
  deltatimereal = time-timelast;

  if (init) {
#if DEBUG_REPLAY
    StartupStore(_T("........ REPLAY: UpdateInternal init\n"));
#endif
    time_lstart = time;
    time = 0;
    deltatimereal = 0;
    ReplayTime = 0;
    cli.Reset();
  }

  ReplayTime += TimeScale*deltatimereal;

#if DEBUG_REPLAY
  TCHAR tutc[20];
  Units::TimeToText(tutc, (int)ReplayTime);
  StartupStore(_T("........ REPLAY: UTC h%s\n"),tutc);
#endif

  double mintime = cli.GetMinTime(); // li_lat.GetMinTime();
  if (ReplayTime<mintime) {
	ReplayTime = mintime;
  }

  // if need a new point
  while (cli.NeedData(ReplayTime)&&(!finished)) {

    double t1, Lat1, Lon1, Alt1;
    finished = !ReadPoint(&t1,&Lat1,&Lon1,&Alt1);

    if (!finished && (t1>0)) {
	if (!cli.Update(t1,Lon1,Lat1,Alt1)) {
		break;
	}
    }
  }

  if (!finished) {

    double LatX, LonX, AltX, SpeedX, BearingX;
    double LatX1, LonX1, AltX1;

    cli.Interpolate(ReplayTime, &LonX, &LatX, &AltX);
    cli.Interpolate(ReplayTime+0.1, &LonX1, &LatX1, &AltX1);

    SpeedX = cli.GetSpeed(ReplayTime);
    DistanceBearing(LatX, LonX, LatX1, LonX1, NULL, &BearingX);

    if ((SpeedX>0) && (LatX != LatX1) && (LonX != LonX1)) {

      LockFlightData();
      if (init) {
        flightstats.Reset();
      }
      GPS_INFO.Latitude = LatX;
      GPS_INFO.Longitude = LonX;
      GPS_INFO.Speed = SpeedX;
      GPS_INFO.TrackBearing = BearingX;
      GPS_INFO.Altitude = AltX;
      GPS_INFO.Time = ReplayTime;
      UnlockFlightData();
    } else {
      // This is required in case the integrator fails,
      // which can occur due to parsing faults
      ReplayTime = cli.GetMaxTime();
    }
  }

  // quit if finished.
  if (finished) {
    #if DEBUG_REPLAY
    StartupStore(_T("......... REPLAY: FINISHED\n"));
    #endif
    Stop();
  }
  init = false;

  return !finished;
}


void ReplayLogger::Stop(void) {
  ReadLine(NULL); // close the file
  if (Enabled) {
    LockFlightData();
    GPS_INFO.Speed = 0;
    UnlockFlightData();
  }
  Enabled = false;
}


void ReplayLogger::Start(void) {
  if (Enabled) {
    Stop();
  }
  flightstats.Reset();
  if (!UpdateInternal()) {
    MessageBoxX(
	// LKTOKEN  _@M201_ = "Could not open IGC file!"
		MsgToken<201>(),
	// LKTOKEN  _@M305_ = "Flight replay"
		MsgToken<305>(),
		mbOk);
  }
}


TCHAR* ReplayLogger::GetFilename(void) {
  return FileName;
}


void ReplayLogger::SetFilename(const TCHAR *name) {
  if (!name) {
    return;
  }
  if (_tcscmp(FileName,name)!=0) {
    lk::strcpy(FileName,name);
  }
}

bool ReplayLogger::Update(void) {
  if (!Enabled)
    return false;

  Enabled = UpdateInternal();
  return Enabled;
}
