/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Parser.cpp,v 8.12 2010/12/12 16:14:28 root Exp root $

*/

#include "StdAfx.h"

#include "options.h"
#include "externs.h"
#include "Utils.h"
#include "Utils2.h"
#include "externs.h"
#include "Logger.h"
#ifndef NOFLARMGAUGE
#include "GaugeFLARM.h"
#endif
#include "Parser.h"
#include "device.h"
#include "Geoid.h"
//#include "FlarmIdFile.h"
#include "TeamCodeCalculation.h"
#include "Message.h"
#include "Cpustats.h"

#include "FlarmCalculations.h"
FlarmCalculations flarmCalculations;

#ifdef __MINGW32__
#ifndef max
#define max(x, y)   (x > y ? x : y)
#define min(x, y)   (x < y ? x : y)
#endif
#endif

extern bool EnableCalibration;

static double EastOrWest(double in, TCHAR EoW);
static double NorthOrSouth(double in, TCHAR NoS);
//static double LeftOrRight(double in, TCHAR LoR);
static double MixedFormatToDegrees(double mixed);
static int NAVWarn(TCHAR c);
void CheckBackTarget(int flarmslot);

NMEAParser nmeaParser1;
NMEAParser nmeaParser2;


int NMEAParser::StartDay = -1;

NMEAParser::NMEAParser() {
  _Reset();
}

void NMEAParser::_Reset(void) {
  nSatellites = 0;
  gpsValid = false;
  isFlarm = false;
  activeGPS = true;
  GGAAvailable = FALSE;
  RMZAvailable = FALSE;
  RMZAltitude = 0;
  RMAAvailable = FALSE;
  RMCAvailable = false;
  TASAvailable = false; // 100411
  RMAAltitude = 0;
  
  LastTime = 0;
  NmeaTime=0;
}

void NMEAParser::Reset(void) {

  // clear status
  nmeaParser1._Reset();
  nmeaParser2._Reset();

  // trigger updates
  TriggerGPSUpdate();
  TriggerVarioUpdate();
}


//#define DEBUGNPM 1
// run every 5 seconds, approx.
void NMEAParser::UpdateMonitor(void) 
{
  short active;
  static short lastactive=0;
  static bool  lastvalidBaro=false;
  short invalidGps=0;
  // short invalidBaro=0;  // not really used now
  short validBaro=0; 

  // does anyone have GPS?
  if (nmeaParser1.gpsValid || nmeaParser2.gpsValid) {
	if (nmeaParser1.gpsValid && nmeaParser2.gpsValid) {
		// both valid, just use first
		nmeaParser2.activeGPS = false;
		nmeaParser1.activeGPS = true;
		active=1;
	} else {
		nmeaParser1.activeGPS = nmeaParser1.gpsValid;
		nmeaParser2.activeGPS = nmeaParser2.gpsValid;
		active= nmeaParser1.activeGPS ? 1 : 2;
	}
  } else {
	// assume device 1 is active
	nmeaParser2.activeGPS = false;
	nmeaParser1.activeGPS = true;
	active=1;
  }
 #if 1	// TODO better check if ok
  if (nmeaParser2.activeGPS==true && active==1) {
	StartupStore(_T("... GPS Update error: port 1 and 2 are active!%s"),NEWLINE);
	FailStore(_T("... GPS Update error: port 1 and 2 are active!%s"),NEWLINE);
	nmeaParser2.activeGPS=false; // force it off
	active=1; 
  }
 #endif

  // wait for some seconds before monitoring, after startup
  if (LKHearthBeats<20) return;
  // Check Port 1 with no serial activity in last seconds
  if ( (LKHearthBeats-ComPortHB[0])>10 ) {
	#ifdef DEBUGNPM
	StartupStore(_T("... GPS Port 1 : no activity LKHB=%.0f CBHB=%.0f %s"),LKHearthBeats, ComPortHB[0],NEWLINE);
	#endif
	// if this is active and supposed to have a valid fix.., but no HB..
	if ( (active==1) && (nmeaParser1.gpsValid) ) {
		StartupStore(_T("... GPS Port 1 no hearthbeats, but still gpsValid: forced invalid%s"),NEWLINE);
	}
	nmeaParser1.gpsValid=false;
	invalidGps=1;
  } else {
	// We have hearth beats, is baro available?
	if ( devIsBaroSource(devA()) || nmeaParser1.RMZAvailable || nmeaParser1.RMAAvailable || nmeaParser1.TASAvailable ) // 100411
		validBaro++;
  }
  // now check also port 2
  if ( (LKHearthBeats-ComPortHB[1])>10 ) {
	#ifdef DEBUGNPM
	StartupStore(_T("... GPS Port 2 : no activity LKHB=%.0f CBHB=%.0f %s"),LKHearthBeats, ComPortHB[1],NEWLINE);
	#endif
	if ( (active==2) && (nmeaParser2.gpsValid) ) {
		StartupStore(_T("... GPS port 2 no hearthbeats, but still gpsValid: forced invalid%s"),NEWLINE);
	}
	nmeaParser2.gpsValid=false;
	invalidGps++;
  } else {
	// We have hearth beats, is baro available?
	if ( devIsBaroSource(devB()) || nmeaParser2.RMZAvailable || nmeaParser2.RMAAvailable || nmeaParser2.TASAvailable   )  // 100411
		validBaro++;
  }

  #ifdef DEBUGNPM
  if (invalidGps==2) {
	StartupStore(_T("... GPS no gpsValid available on port 1 and 2, active=%d%s"),active,NEWLINE);
  }
  #endif

  // do we really still have a baro altitude available?
  // If some baro source disappeared, let's reset it for safety. Parser will re-enable them immediately if available.
  // Assuming here that if no Baro is available, no airdata is available also
  //
  if (validBaro==0) {
	if ( GPS_INFO.BaroAltitudeAvailable ) {
		StartupStore(_T("... GPS no active baro source, and still BaroAltitudeAvailable, forced off%s"),NEWLINE);
		if (EnableNavBaroAltitude) {
	// LKTOKEN  _@M122_ = "BARO ALTITUDE NOT AVAILABLE, USING GPS ALTITUDE" 
			DoStatusMessage(gettext(TEXT("_@M122_")));
			PortMonitorMessages++;	// 100911
		} else
	// LKTOKEN  _@M121_ = "BARO ALTITUDE NOT AVAILABLE" 
			DoStatusMessage(gettext(TEXT("_@M121_")));
		GPS_INFO.BaroAltitudeAvailable=false;
		GPS_INFO.AirspeedAvailable=false;
		GPS_INFO.VarioAvailable=false;
		GPS_INFO.NettoVarioAvailable=false;
		lastvalidBaro=false;
	}
  } else {
	if ( lastvalidBaro==false) {
		StartupStore(_T("... GPS baro source back available%s"),NEWLINE);
		if (EnableNavBaroAltitude)
	// LKTOKEN  _@M755_ = "USING AVAILABLE BARO ALTITUDE" 
			DoStatusMessage(gettext(TEXT("_@M755_")));
		else
	// LKTOKEN  _@M120_ = "BARO ALTITUDE IS AVAILABLE" 
			DoStatusMessage(gettext(TEXT("_@M120_")));
		lastvalidBaro=true;
	}


  }

  // Following diagnostics only
  if (active == lastactive) return;
  if (lastactive==0) {
	lastactive=active;
	StartupStore(_T(". GPS NMEA init delegated to port %d%s"),active,NEWLINE);
	return;
  }

  lastactive=active;
  // in case of no gps at all, port 1 is selected but we dont want to tell unless really working
  if (PortMonitorMessages<10) { // 100221 do not overload pilot with messages!
	StartupStore(_T("... GPS NMEA source changed to port %d %s"),active,NEWLINE);
	if (nmeaParser1.gpsValid || nmeaParser2.gpsValid){
		TCHAR vbuf[100]; _stprintf(vbuf,_T("%s %d"),
	// LKTOKEN  _@M277_ = "FALLBACK USING GPS ON PORT" 
		gettext(TEXT("_@M277_")),active);
		DoStatusMessage(vbuf);
	}
	PortMonitorMessages++;
  } else {
	if (PortMonitorMessages==10) { // 100221
		StartupStore(_T("... GOING SILENT on too many Com reportings.%s"),NEWLINE);
	// LKTOKEN  _@M317_ = "GOING SILENT ON COM REPORTING" 
		DoStatusMessage(gettext(TEXT("_@M317_")));
		PortMonitorMessages++;
	} else
		PortMonitorMessages++;
  }


}


BOOL NMEAParser::ParseNMEAString(int device,
				 TCHAR *String, NMEA_INFO *GPS_INFO)
{
  switch (device) {
  case 0: 
    return nmeaParser1.ParseNMEAString_Internal(String, GPS_INFO);
  case 1:
    return nmeaParser2.ParseNMEAString_Internal(String, GPS_INFO);
  };
  return FALSE;
}


/*
 * Copy a provided string into the supplied buffer, terminate on
 * the checksum separator, split into an array of parameters,
 * and return the number of parameters found.
 */
size_t NMEAParser::ExtractParameters(const TCHAR *src, TCHAR *dst, TCHAR **arr, size_t sz)
{
  TCHAR c, *p;
  size_t i = 0;

  _tcscpy(dst, src);
  p = _tcschr(dst, _T('*'));
  if (p)
    *p = _T('\0');

  p = dst;
  do {
    arr[i++] = p;
    p = _tcschr(p, _T(','));
    if (!p)
      break;
    c = *p;
    *p++ = _T('\0');
  } while (i != sz && c != _T('\0'));

  return i;
}


/*
 * Same as ExtractParameters, but also validate the length of
 * the string and the NMEA checksum.
 */
size_t NMEAParser::ValidateAndExtract(const TCHAR *src, TCHAR *dst, size_t dstsz, TCHAR **arr, size_t arrsz)
{
  int len = _tcslen(src);

  if (len <= 6 || len >= (int) dstsz)
    return 0;
  if (!NMEAChecksum(src))
    return 0;

  return ExtractParameters(src, dst, arr, arrsz);
}


BOOL NMEAParser::ParseNMEAString_Internal(TCHAR *String, NMEA_INFO *GPS_INFO)
{
  TCHAR ctemp[MAX_NMEA_LEN];
  TCHAR *params[MAX_NMEA_PARAMS];
  size_t n_params;


  n_params = ValidateAndExtract(String, ctemp, MAX_NMEA_LEN, params, MAX_NMEA_PARAMS);
  if (n_params < 1 || params[0][0] != '$')
    return FALSE;
//  if (EnableLogNMEA) 091108 removed LogNMEA and place in calling function
//    LogNMEA(String);

  if(params[0][1] == 'P')
    {
      //Proprietary String

#ifdef DSX
      if(_tcscmp(params[0] + 1,TEXT("PDSXT"))==0)
        {
          return PDSXT(&String[7], params + 1, n_params, GPS_INFO);
        }
#endif 


      if(_tcscmp(params[0] + 1,TEXT("PTAS1"))==0)
        {
          return PTAS1(&String[7], params + 1, n_params, GPS_INFO);
        }

      // FLARM sentences
      if(_tcscmp(params[0] + 1,TEXT("PFLAA"))==0)
        {
          return PFLAA(&String[7], params + 1, n_params, GPS_INFO);
        }

      if(_tcscmp(params[0] + 1,TEXT("PFLAU"))==0)
        {
          return PFLAU(&String[7], params + 1, n_params, GPS_INFO);
        }

      if(_tcscmp(params[0] + 1,TEXT("PGRMZ"))==0)
	{
	  return RMZ(&String[7], params + 1, n_params, GPS_INFO);
	}
      return FALSE;
    }

  if(_tcscmp(params[0] + 3,TEXT("GSA"))==0)
    {
      return GSA(&String[7], params + 1, n_params, GPS_INFO);
    }
  if(_tcscmp(params[0] + 3,TEXT("GLL"))==0)
    {
      //    return GLL(&String[7], params + 1, n_params, GPS_INFO);
      return FALSE;
    }
  if(_tcscmp(params[0] + 3,TEXT("RMB"))==0)
    {
      //return RMB(&String[7], params + 1, n_params, GPS_INFO);
          return FALSE;
      }
  if(_tcscmp(params[0] + 3,TEXT("RMC"))==0)
    {
      return RMC(&String[7], params + 1, n_params, GPS_INFO);
    }
  if(_tcscmp(params[0] + 3,TEXT("GGA"))==0)
    {
      return GGA(&String[7], params + 1, n_params, GPS_INFO);
    }
  if(_tcscmp(params[0] + 3,TEXT("VTG"))==0)
    {
      return VTG(&String[7], params + 1, n_params, GPS_INFO);
    }

  return FALSE;
}

void NMEAParser::ExtractParameter(const TCHAR *Source, 
				  TCHAR *Destination, 
				  int DesiredFieldNumber)
{
  int dest_index = 0;
  int CurrentFieldNumber = 0;
  int StringLength = _tcslen(Source);
  TCHAR *sptr = (TCHAR*)Source;
  const TCHAR *eptr = Source+StringLength;

  if (!Destination) return;

  while( (CurrentFieldNumber < DesiredFieldNumber) && (sptr<eptr) )
    {
      if (*sptr == ',' || *sptr == '*' )
        {
          CurrentFieldNumber++;
        }
      ++sptr;
    }

  Destination[0] = '\0'; // set to blank in case it's not found..

  if ( CurrentFieldNumber == DesiredFieldNumber )
    {
      while( (sptr < eptr)    &&
             (*sptr != ',') &&
             (*sptr != '*') &&
             (*sptr != '\0') )
        {
          Destination[dest_index] = *sptr;
          ++sptr; ++dest_index;
        }
      Destination[dest_index] = '\0';
    }
}


double EastOrWest(double in, TCHAR EoW)
{
  if(EoW == 'W')
    return -in;
  else
    return in;
}

double NorthOrSouth(double in, TCHAR NoS)
{
  if(NoS == 'S')
    return -in;
  else
    return in;
}

/*
double LeftOrRight(double in, TCHAR LoR)
{
  if(LoR == 'L')
    return -in;
  else
    return in;
}
*/

int NAVWarn(TCHAR c)
{
  if(c=='A')
    return FALSE;
  else
    return TRUE;
}

double NMEAParser::ParseAltitude(TCHAR *value, const TCHAR *format)
{
  double alt = StrToDouble(value, NULL);

  if (format[0] == _T('f') || format[0] == _T('F'))
    alt /= TOFEET;

  return alt;
}

double MixedFormatToDegrees(double mixed)
{
  double mins, degrees;

  degrees = (int) (mixed/100);
  mins = (mixed - degrees*100)/60;

  return degrees+mins;
}

// AND.. what if your gps is sending a 00:00 date and time?? 091129
double NMEAParser::TimeModify(double FixTime, NMEA_INFO* GPS_INFO)
{
  double hours, mins,secs;
  
  hours = FixTime / 10000;
  GPS_INFO->Hour = (int)hours;

  mins = FixTime / 100;
  mins = mins - (GPS_INFO->Hour*100);
  GPS_INFO->Minute = (int)mins;

  secs = FixTime - (GPS_INFO->Hour*10000) - (GPS_INFO->Minute*100);
  GPS_INFO->Second = (int)secs;

  FixTime = secs + (GPS_INFO->Minute*60) + (GPS_INFO->Hour*3600);

  if ((StartDay== -1) && (GPS_INFO->Day != 0)) {
    StartDay = GPS_INFO->Day;
  }
  if (StartDay != -1) {
    if (GPS_INFO->Day < StartDay) {
      // detect change of month (e.g. day=1, startday=31)
      StartDay = GPS_INFO->Day-1;
    }
    int day_difference = GPS_INFO->Day-StartDay;
    if (day_difference>0) {
      // Add seconds to fix time so time doesn't wrap around when
      // going past midnight in UTC
      FixTime += day_difference * 86400;
    }
  }
  return FixTime;
}

// convert to double , missing wraparound midnight
double NMEAParser::TimeConvert(double FixTime, NMEA_INFO* GPS_INFO)
{
  double hours, mins,secs;
  hours = FixTime / 10000;
  NmeaHours = (int)hours;
  mins = FixTime / 100;
  mins = mins - (NmeaHours*100);
  NmeaMinutes = (int)mins;
  secs = FixTime - (NmeaHours*10000) - (NmeaMinutes*100);
  NmeaSeconds = (int)secs;
  FixTime = secs + (NmeaMinutes*60) + (NmeaHours*3600);

  if ((StartDay== -1) && (GPS_INFO->Day != 0)) {
	StartDay = GPS_INFO->Day;
  }
  if (StartDay != -1) {
	if (GPS_INFO->Day < StartDay) {
		// detect change of month (e.g. day=1, startday=31)
		StartDay = GPS_INFO->Day-1;
	}
	int day_difference = GPS_INFO->Day-StartDay;
	if (day_difference>0) {
		FixTime += day_difference * 86400;
	}
  }

  return FixTime;
}

// set time to GPS struct
void NMEAParser::TimeSet(NMEA_INFO* GPS_INFO)
{

  GPS_INFO->Hour = NmeaHours;
  GPS_INFO->Minute = NmeaMinutes;
  GPS_INFO->Second = NmeaSeconds;

}

bool NMEAParser::TimeHasAdvanced(double ThisTime, NMEA_INFO *GPS_INFO) {

  // If simulating, we might be in the future already...
  if(ThisTime< LastTime) {
    LastTime = ThisTime;
    StartDay = -1; // reset search for the first day
    return false;
  } else {
    GPS_INFO->Time = ThisTime;
    LastTime = ThisTime;
    return true;
  }
}

BOOL NMEAParser::GSA(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *GPS_INFO)
{
  int iSatelliteCount =0;

  GSAAvailable = TRUE; // 100213 MISSING BUGFIX 
  if (!activeGPS) return TRUE; // 100213 BUGFIX

  if (ReplayLogger::IsEnabled()) {
    return TRUE;
  }

  // satellites are in items 4-15 of GSA string (4-15 is 1-indexed)
  // but 1st item in string is not passed, so start at item 3
  for (int i = 0; i < MAXSATELLITES; i++)
  {
    if (3+i < (int) nparams) {
      GPS_INFO->SatelliteIDs[i] = (int)(StrToDouble(params[2+i], NULL)); // 2 because params is 0-index
      if (GPS_INFO->SatelliteIDs[i] > 0)
	iSatelliteCount ++;
    }
  }
  return TRUE;
}

// we need to parse GLL as well because it can mark the start of a new quantum data
// followed by values with no data, ex. altitude, vario, etc.
BOOL NMEAParser::GLL(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *GPS_INFO)
{

  gpsValid = !NAVWarn(params[5][0]);

  if (!activeGPS) return TRUE;

  if (ReplayLogger::IsEnabled()) {
    // block actual GPS signal
	InterfaceTimeoutReset();
	return TRUE;
  }
  
  GPS_INFO->NAVWarning = !gpsValid;
  
  // use valid time with invalid fix
  double glltime = StrToDouble(params[4],NULL);
  if (glltime>0) {
	#ifdef NEWTRIGGERGPS
	double ThisTime = TimeConvert(glltime, GPS_INFO); // 091208
	#else
	double ThisTime = TimeModify(glltime, GPS_INFO);
	#endif

	#ifndef NEWTRIGGERGPS
	if (!TimeHasAdvanced(ThisTime, GPS_INFO)) return FALSE; // 091208
	#endif

	#ifdef NEWTRIGGERGPS
	// is time advanced to a new quantum?
	if (ThisTime >NmeaTime) {
		// yes so lets trigger the gps event
		TriggerGPSUpdate();
		Sleep(50); // 091208
		NmeaTime=ThisTime;
		TimeSet(GPS_INFO); // 091208
		TimeHasAdvanced(ThisTime,GPS_INFO); // 091208
		//StartupStore(_T(".............. trigger from GLL\n"));
	}
	#endif
  }
  if (!gpsValid) return FALSE;  // 091108 addon BUGFIX GLL time with no valid signal
  
  double tmplat;
  double tmplon;
  
  tmplat = MixedFormatToDegrees(StrToDouble(params[0], NULL));
  tmplat = NorthOrSouth(tmplat, params[1][0]);
  
  tmplon = MixedFormatToDegrees(StrToDouble(params[2], NULL));
  tmplon = EastOrWest(tmplon,params[3][0]);
  
  if (!((tmplat == 0.0) && (tmplon == 0.0))) {
	GPS_INFO->Latitude = tmplat;
	GPS_INFO->Longitude = tmplon;
  } else {
    
  }
  return TRUE;
}


BOOL NMEAParser::RMB(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *GPS_INFO)
{
  (void)GPS_INFO;
  (void)String;
  (void)params;
  (void)nparams;
  /* we calculate all this stuff now 
  TCHAR ctemp[MAX_NMEA_LEN];

  GPS_INFO->NAVWarning = NAVWarn(params[0][0]);

  GPS_INFO->CrossTrackError = NAUTICALMILESTOMETRES * StrToDouble(params[1], NULL);
  GPS_INFO->CrossTrackError = LeftOrRight(GPS_INFO->CrossTrackError,params[2][0]);

  _tcscpy(ctemp, params[4]);
  ctemp[WAY_POINT_ID_SIZE] = '\0';
  _tcscpy(GPS_INFO->WaypointID,ctemp);

  GPS_INFO->WaypointDistance = NAUTICALMILESTOMETRES * StrToDouble(params[9], NULL);
  GPS_INFO->WaypointBearing = StrToDouble(params[10], NULL);
  GPS_INFO->WaypointSpeed = KNOTSTOMETRESSECONDS * StrToDouble(params[11], NULL);
  */

  return TRUE;
}

#if (!defined(WINDOWSPC) || (WINDOWSPC==0)) 
bool SetSystemTimeFromGPS = true; // 091105 was false
#else
bool SetSystemTimeFromGPS = false; // 091129
#endif

BOOL NMEAParser::VTG(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *GPS_INFO)
{
  GPSCONNECT = TRUE;
  if (RMCAvailable) return FALSE;
  double speed=0;

  // if no valid fix, we dont get speed either!
  if (gpsValid)
  {
	speed = StrToDouble(params[4], NULL);
	// speed is in knots, 2 = 3.7kmh
	if (speed>2.0) {
		GPS_INFO->MovementDetected = TRUE;
		if (ReplayLogger::IsEnabled()) {
			// stop logger replay if aircraft is actually moving.
			ReplayLogger::Stop();
		}
	} else {
		GPS_INFO->MovementDetected = FALSE;
		if (ReplayLogger::IsEnabled()) {
			// block actual GPS signal if not moving and a log is being replayed
			return TRUE;
		}
	}

	GPS_INFO->Speed = KNOTSTOMETRESSECONDS * speed;
  
	if (GPS_INFO->Speed>1.0) {
		GPS_INFO->TrackBearing = AngleLimit360(StrToDouble(params[0], NULL));
	}
  }

  // if we are here, no RMC is available but if no GGA also, we are in troubles: to check!
  if (!GGAAvailable) {
	TriggerGPSUpdate();
  }

  return TRUE;

}

BOOL NMEAParser::RMC(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *GPS_INFO)
{
  TCHAR *Stop;
  static bool logbaddate=true;
  double speed=0;

  gpsValid = !NAVWarn(params[1][0]);

  GPSCONNECT = TRUE;    
  RMCAvailable=true; // 100409

  if (!activeGPS) return TRUE; // 091205 BUGFIX true

  // if no valid fix, we dont get speed either!
  if (gpsValid)
  {
	speed = StrToDouble(params[6], NULL);
	// speed is in knots, 2 = 3.7kmh
	if (speed>2.0) {
		GPS_INFO->MovementDetected = TRUE;
		if (ReplayLogger::IsEnabled()) {
			// stop logger replay if aircraft is actually moving.
			ReplayLogger::Stop();
		}
	} else {
		GPS_INFO->MovementDetected = FALSE;
		if (ReplayLogger::IsEnabled()) {
			// block actual GPS signal if not moving and a log is being replayed
			return TRUE;
		}
	}
  }
  
  GPS_INFO->NAVWarning = !gpsValid;

  // say we are updated every time we get this,
  // so infoboxes get refreshed if GPS connected
  // the RMC sentence marks the start of a new fix, so we force the old data to be saved for calculations
#ifndef NEWTRIGGERGPS
  if (!GGAAvailable) { 
	TriggerGPSUpdate();
  }
#endif

	// Even with no valid position, we let RMC set the time and date if valid
	long gy, gm, gd;
	gy = _tcstol(&params[8][4], &Stop, 10) + 2000;   
	params[8][4] = '\0';
	gm = _tcstol(&params[8][2], &Stop, 10); 
	params[8][2] = '\0';
	gd = _tcstol(&params[8][0], &Stop, 10); 

#if NOSIM
	if ( ((gy > 1980) && (gy <2100) ) && (gm != 0) && (gd != 0) ) { 
#else
#ifdef _SIM_
	// SeeYou PC is sending NMEA sentences with RMC date 2072-02-27
	if ( ((gy > 1980) && (gy <2100) ) && (gm != 0) && (gd != 0) ) { // 100422
#else
	if ( ((gy > 2000) && (gy <2020) ) && (gm != 0) && (gd != 0) ) { // 100422
#endif
#endif
		GPS_INFO->Year = gy;
		GPS_INFO->Month = gm;
		GPS_INFO->Day = gd;
#ifdef NEWTRIGGERGPS
		double ThisTime = TimeConvert(StrToDouble(params[0],NULL), GPS_INFO); // 091208
#else
		double ThisTime = TimeModify(StrToDouble(params[0],NULL), GPS_INFO);
#endif

#ifndef NEWTRIGGERGPS
		if (!TimeHasAdvanced(ThisTime, GPS_INFO))
			return FALSE;
#endif
#ifdef NEWTRIGGERGPS
		// is time advanced to a new quantum?
		if (ThisTime >NmeaTime) {
			// yes so lets trigger the gps event
			TriggerGPSUpdate();
			// and only then advance the time in the GPSINFO
			Sleep(50); // 091208
			NmeaTime=ThisTime;
			TimeSet(GPS_INFO); // 091208
			TimeHasAdvanced(ThisTime, GPS_INFO); // 091208
			StartupStore(_T(".............. trigger from RMC\n"));
		}
#endif
			
	}  else {
		if (gpsValid && logbaddate) { // 091115
			StartupStore(_T("------ NMEAParser:RMC Receiving an invalid or null DATE from GPS%s"),NEWLINE);
			StartupStore(_T("------ NMEAParser: Date received is y=%d m=%d d=%d%s"),gy,gm,gd,NEWLINE); // 100422
			StartupStore(_T("------ This message will NOT be repeated.%s"),NEWLINE);
			// DoStatusMessage(_T("WARNING: GPS IS SENDING INVALID DATE, AND PROBABLY WRONG TIME")); // REMOVE FIXV2
			// LKTOKEN 875  WARNING: GPS IS SENDING INVALID DATE, AND PROBABLY WRONG TIME")); 
			DoStatusMessage(gettext(TEXT("_@M875_")));
			logbaddate=false;
		}
	}
//  } // 091108

  if (gpsValid) {   // 091108 BUGFIX set latlon and speed ONLY if valid gpsdata, missing check!
	double tmplat;
	double tmplon;

	tmplat = MixedFormatToDegrees(StrToDouble(params[2], NULL));
	tmplat = NorthOrSouth(tmplat, params[3][0]);
	  
	tmplon = MixedFormatToDegrees(StrToDouble(params[4], NULL));
	tmplon = EastOrWest(tmplon,params[5][0]);
  
	if (!((tmplat == 0.0) && (tmplon == 0.0))) {
		GPS_INFO->Latitude = tmplat;
		GPS_INFO->Longitude = tmplon;
	}
  
	GPS_INFO->Speed = KNOTSTOMETRESSECONDS * speed;
  
	if (GPS_INFO->Speed>1.0) {
		// JMW don't update bearing unless we're moving
		GPS_INFO->TrackBearing = AngleLimit360(StrToDouble(params[7], NULL));
	}
  } // gpsvalid 091108
    
  // Altair doesn't have a battery-backed up realtime clock,
  // so as soon as we get a fix for the first time, set the
  // system clock to the GPS time.
  static bool sysTimeInitialised = false;
  
  if (!GPS_INFO->NAVWarning && (gpsValid)) {
	if (SetSystemTimeFromGPS) {
		if (!sysTimeInitialised) {
			if ( ( GPS_INFO->Year > 1980 && GPS_INFO->Year<2100) && ( GPS_INFO->Month > 0) && ( GPS_INFO->Hour > 0)) {
        
				sysTimeInitialised =true; // Attempting only once
				SYSTEMTIME sysTime;
				// ::GetSystemTime(&sysTime);
				int hours = (int)GPS_INFO->Hour;
				int mins = (int)GPS_INFO->Minute;
				int secs = (int)GPS_INFO->Second;
				sysTime.wYear = (unsigned short)GPS_INFO->Year;
				sysTime.wMonth = (unsigned short)GPS_INFO->Month;
				sysTime.wDay = (unsigned short)GPS_INFO->Day;
				sysTime.wHour = (unsigned short)hours;
				sysTime.wMinute = (unsigned short)mins;
				sysTime.wSecond = (unsigned short)secs;
				sysTime.wMilliseconds = 0;
				::SetSystemTime(&sysTime);
			}
		}
	}
  }

  if (!ReplayLogger::IsEnabled()) {      
	if(RMZAvailable) {
		// JMW changed from Altitude to BaroAltitude
		GPS_INFO->BaroAltitudeAvailable = true;
		GPS_INFO->BaroAltitude = RMZAltitude;
	}
	else if(RMAAvailable) {
	// JMW changed from Altitude to BaroAltitude
		GPS_INFO->BaroAltitudeAvailable = true;
		GPS_INFO->BaroAltitude = RMAAltitude;
	}
  }
  if (!GGAAvailable) {
	// update SatInUse, some GPS receiver dont emmit GGA sentance
	if (!gpsValid) { 
		GPS_INFO->SatellitesUsed = 0;
	} else {
		GPS_INFO->SatellitesUsed = -1;
	}
  }
  
  return TRUE;
}

BOOL NMEAParser::GGA(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *GPS_INFO)
{

  if (ReplayLogger::IsEnabled()) {
        return TRUE;
  }

  GGAAvailable = TRUE;
  GPSCONNECT = TRUE;     // 091208

  // this will force gps invalid but will NOT assume gps valid!
  nSatellites = (int)(min(16.0, StrToDouble(params[6], NULL)));
  if (nSatellites==0) {
	gpsValid = false;
  }

  double ggafix = StrToDouble(params[5],NULL);
  if ( ggafix==0 || ggafix>5 ) {
	#ifdef DEBUG_GPS
	if (ggafix>5) StartupStore(_T("------ GGA DEAD RECKON fix skipped%s"),NEWLINE);
	#endif
	gpsValid=false;
  } else {
	gpsValid=true;
  }

  // don't set any GPS_INFO if not activeGPS!!
  if (!activeGPS) return TRUE;

  GPS_INFO->SatellitesUsed = nSatellites; // 091208
  GPS_INFO->NAVWarning = !gpsValid; // 091208

//  // GPS_INFO->SatellitesUsed = (int)(min(16,StrToDouble(params[6], NULL))); 091205 091208 moved up
//  GPS_INFO->SatellitesUsed = nSatellites; // 091205

  double ggatime=StrToDouble(params[0],NULL);
  // Even with invalid fix, we might still have valid time
  // I assume that 0 is invalid, and I am very sorry for UTC time 00:00 ( missing a second a midnight).
  // is better than risking using 0 as valid, since many gps do not respect any real nmea standard
  if (ggatime>0) { 
	#ifdef NEWTRIGGERGPS
	double ThisTime = TimeConvert(ggatime, GPS_INFO);
	#else
	double ThisTime = TimeModify(ggatime, GPS_INFO);
	#endif

	#ifndef NEWTRIGGERGPS
	if (!TimeHasAdvanced(ThisTime, GPS_INFO))
		return FALSE;
	#endif

	#ifdef NEWTRIGGERGPS
		// is time advanced to a new quantum?
		if (ThisTime >NmeaTime) {
			// yes so lets trigger the gps event
			TriggerGPSUpdate();
			Sleep(50); // 091208
			NmeaTime=ThisTime;
			TimeSet(GPS_INFO); // 091208
			TimeHasAdvanced(ThisTime, GPS_INFO); // 091208
			StartupStore(_T(".............. trigger from GGA\n"));
		}
	#endif
  }
  if (gpsValid) {
	double tmplat;
	double tmplon;
	tmplat = MixedFormatToDegrees(StrToDouble(params[1], NULL));
	tmplat = NorthOrSouth(tmplat, params[2][0]);
	tmplon = MixedFormatToDegrees(StrToDouble(params[3], NULL));
	tmplon = EastOrWest(tmplon,params[4][0]);
	if (!((tmplat == 0.0) && (tmplon == 0.0))) {
		GPS_INFO->Latitude = tmplat;
		GPS_INFO->Longitude = tmplon;
	} 
	else {
		#ifdef DEBUG_GPS
		StartupStore(_T("++++++ GGA gpsValid with invalid posfix!%s"),NEWLINE);
		#endif
		gpsValid=false;
	}
  }
 
  // GGA is marking now triggering end of data, so OK to use baro
  // Even with invalid fix we might have valid baro data of course

  // any NMEA sentence with time can now trigger gps update: the first to detect new time will make trigger.
  // we assume also that any sentence with no time belongs to current time.
  // note that if no time from gps, no use of vario and baro data, but also no fix available.. so no problems
  if(RMZAvailable) {
	GPS_INFO->BaroAltitudeAvailable = true;
	GPS_INFO->BaroAltitude = RMZAltitude;
  }
  else if(RMAAvailable) {
	GPS_INFO->BaroAltitudeAvailable = true;
	GPS_INFO->BaroAltitude = RMAAltitude;
  }

#ifndef NEWTRIGGERGPS
  if (!gpsValid) { // 091108 addon BUGFIX GCA
	// in old mode, GGA had priority over RMC for triggering, so this was needed in case of no signal 
	TriggerGPSUpdate(); // 091205 TESTFIX
	return FALSE; // 091108 addon BUGFIX GCA
  }
#endif

  // "Altitude" should always be GPS Altitude.
  GPS_INFO->Altitude = ParseAltitude(params[8], params[9]);
  GPS_INFO->Altitude += (GPSAltitudeOffset/1000); // BUGFIX 100429
  
  double GeoidSeparation;

  if (_tcslen(params[10])>0) {
    // No real need to parse this value,
    // but we do assume that no correction is required in this case
    GeoidSeparation = ParseAltitude(params[10], params[11]);
  } else {
	if (UseGeoidSeparation) {
		GeoidSeparation = LookupGeoidSeparation(GPS_INFO->Latitude, GPS_INFO->Longitude);
		GPS_INFO->Altitude -= GeoidSeparation;
	}
  }

#ifndef NEWTRIGGERGPS
  // if RMC would be Triggering update, we loose the relative altitude, which is coming AFTER rmc! 
  // This was causing old altitude recorded in new pos fix.
  TriggerGPSUpdate(); 
#endif
  return TRUE;
}


BOOL NMEAParser::RMZ(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *GPS_INFO)
{
  (void)GPS_INFO;

  RMZAltitude = ParseAltitude(params[0], params[1]);
  #if NEWQNH
  RMZAltitude = AltitudeToQNHAltitude(RMZAltitude); // 100129 BUGFIX
  #endif
  RMZAvailable = TRUE;

  // if no device declared to have baro, we can use RMZ even if not activeGPS
  // OR if the declared device failed to provide baro!! 
  if (!devHasBaroSource() || !GPS_INFO->BaroAltitudeAvailable) {
    if (!ReplayLogger::IsEnabled()) {      
      GPS_INFO->BaroAltitudeAvailable = true;
      GPS_INFO->BaroAltitude = RMZAltitude;
    }
  }

  return FALSE;
}


BOOL NMEAParser::RMA(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *GPS_INFO)
{
  (void)GPS_INFO;

  RMAAltitude = ParseAltitude(params[0], params[1]);
  #if NEWQNH	
  RMAAltitude = AltitudeToQNHAltitude(RMAAltitude); 
  #endif
  RMAAvailable = TRUE;
  GPS_INFO->BaroAltitudeAvailable = true;

  if (!devHasBaroSource() || !GPS_INFO->BaroAltitudeAvailable) { // 100213
    if (!ReplayLogger::IsEnabled()) {      
      // JMW no in-built baro sources, so use this generic one
      GPS_INFO->BaroAltitudeAvailable = true;
      GPS_INFO->BaroAltitude = RMAAltitude;
    }
  }

  return FALSE;
}


BOOL NMEAParser::NMEAChecksum(const TCHAR *String)
{
  if (!CheckSum) return TRUE;
  unsigned char CalcCheckSum = 0;
  unsigned char ReadCheckSum;
  int End;
  int i;
  TCHAR c1,c2;
  unsigned char v1 = 0,v2 = 0;
  TCHAR *pEnd;

  pEnd = _tcschr((TCHAR*)String,'*');
  if(pEnd == NULL)
    return FALSE;

  // Fix problem of EW micrologger missing a digit in checksum
  // now we have *XY 
  // line is terminating with 0a (\n) so count is 4 not 3!
  if(_tcslen(pEnd)<4) {
	// no checksum, only a * ?
	if (_tcslen(pEnd)==1) {
		return FALSE;
	}
	// try to recover the missing digit
	c1 = _T('0');
	c2 = pEnd[1];
  } else {
	c1 = pEnd[1], c2 = pEnd[2];
  }

  //  iswdigit('0'); // what's this for?

  if(_istdigit(c1))
    v1 = (unsigned char)(c1 - '0');
  if(_istdigit(c2))
    v2 = (unsigned char)(c2 - '0');
  if(_istalpha(c1))
    v1 = (unsigned char)(c1 - 'A' + 10);
  if(_istalpha(c2))
    v2 = (unsigned char)(c2 - 'A' + 10);

  ReadCheckSum = (unsigned char)((v1<<4) + v2);          

  End =(int)( pEnd - String);

  for(i=1;i<End;i++)
    {
      CalcCheckSum = (unsigned char)(CalcCheckSum ^ String[i]);
    }

  if(CalcCheckSum == ReadCheckSum)
    return TRUE;
  else
    return FALSE;
}

// TASMAN instruments support for Tasman Flight Pack model Fp10
BOOL NMEAParser::PTAS1(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *GPS_INFO)
{
  double wnet,baralt,vtas;

  wnet = (StrToDouble(params[0],NULL)-200)/(10*TOKNOTS);
  baralt = max(0.0, (StrToDouble(params[2],NULL)-2000)/TOFEET);
  vtas = StrToDouble(params[3],NULL)/TOKNOTS;
  
  GPS_INFO->AirspeedAvailable = TRUE;
  GPS_INFO->TrueAirspeed = vtas;
  GPS_INFO->VarioAvailable = TRUE;
  GPS_INFO->Vario = wnet;
  GPS_INFO->BaroAltitudeAvailable = TRUE;
  GPS_INFO->BaroAltitude = AltitudeToQNHAltitude(baralt);
  GPS_INFO->IndicatedAirspeed = vtas/AirDensityRatio(baralt);
 
  TASAvailable = true; // 100411 
  TriggerVarioUpdate();

  return FALSE;
}


double AccelerometerZero=100.0;

void FLARM_RefreshSlots(NMEA_INFO *GPS_INFO) {
  int i;
  bool present = false;
  double passed;
  if (GPS_INFO->FLARM_Available) {

	#ifdef DEBUG_LKT
	StartupStore(_T("... [CALC thread] RefreshSlots\n"));
	#endif

	for (i=0; i<FLARM_MAX_TRAFFIC; i++) {
		if (GPS_INFO->FLARM_Traffic[i].ID>0) {

			if ( GPS_INFO->Time< GPS_INFO->FLARM_Traffic[i].Time_Fix) {
				// time gone back to to Replay mode?
				#ifdef DEBUG_LKT
				StartupStore(_T("...... Refresh Back in time! Removing:%s"),NEWLINE);
				FLARM_DumpSlot(GPS_INFO,i);
				#endif
				if (GPS_INFO->FLARM_Traffic[i].Locked) {
					#ifdef DEBUG_LKT
					StartupStore(_T("...... (it was a LOCKED target, unlocking)%s"),NEWLINE);
					#endif
					LKTargetIndex=-1;
					LKTargetType=LKT_TYPE_NONE;
					// reset Virtual waypoint in any case
					WayPointList[RESWP_FLARMTARGET].Latitude   = RESWP_INVALIDNUMBER;
					WayPointList[RESWP_FLARMTARGET].Longitude  = RESWP_INVALIDNUMBER;
					WayPointList[RESWP_FLARMTARGET].Altitude   = RESWP_INVALIDNUMBER;
				}
				FLARM_EmptySlot(GPS_INFO,i);
				continue;
			}

			passed= GPS_INFO->Time-GPS_INFO->FLARM_Traffic[i].Time_Fix;

			// if time has passed > zombie, then we remove it
			if (passed > LKTime_Zombie) {
				if (GPS_INFO->FLARM_Traffic[i].Locked) {
					#ifdef DEBUG_LKT
					StartupStore(_T("...... Zombie overtime index=%d is LOCKED, no remove\n"),i);
					#endif
					continue;
				}
				#ifdef DEBUG_LKT
				StartupStore(_T("... Refresh Removing old zombie (passed=%f Fix=%f Now=%f):%s"),
					passed,
					GPS_INFO->FLARM_Traffic[i].Time_Fix,
					GPS_INFO->Time,
					NEWLINE);
				FLARM_DumpSlot(GPS_INFO,i);
				#endif
				FLARM_EmptySlot(GPS_INFO,i);
				continue;
			}

			// if time has passed > ghost, then it is a zombie
			// Ghosts are not visible on map and radar, only in infopages
			if (passed > LKTime_Ghost) {
				if (GPS_INFO->FLARM_Traffic[i].Status == LKT_ZOMBIE) continue;
				#ifdef DEBUG_LKT
				StartupStore(_T("... Refresh Change to zombie:%s"),NEWLINE);
				FLARM_DumpSlot(GPS_INFO,i);
				#endif
				GPS_INFO->FLARM_Traffic[i].Status = LKT_ZOMBIE;
				continue;
			}

			// if time has passed > real, than it is a ghost
			// Shadows are shown on map as reals.
			if (passed > LKTime_Real) {
				if (GPS_INFO->FLARM_Traffic[i].Status == LKT_GHOST) continue;
				#ifdef DEBUG_LKT
				StartupStore(_T("... Refresh Change to ghost:%s"),NEWLINE);
				FLARM_DumpSlot(GPS_INFO,i);
				#endif
				GPS_INFO->FLARM_Traffic[i].Status = LKT_GHOST;
				present = true;
				continue;
			}

			// Then it is real traffic
			present=true;
			GPS_INFO->FLARM_Traffic[i].Status = LKT_REAL; // 100325 BUGFIX missing

			/*
			} else {
				if (GPS_INFO->FLARM_Traffic[i].AlarmLevel>0) {
					GaugeFLARM::Suppress = false; // NO USE
				}
				present = true;
			}
			*/
		}
	}
  }
  #ifndef NOFLARMGAUGE
  GaugeFLARM::TrafficPresent(present);
  #endif
}

// Reset a flarm slot
void FLARM_EmptySlot(NMEA_INFO *GPS_INFO,int i) {

#ifdef DEBUG_LKT
  StartupStore(_T("... --- EmptySlot %d : ID=%0x <%s> Cn=<%s>\n"),  i, 
	GPS_INFO->FLARM_Traffic[i].ID, 
	GPS_INFO->FLARM_Traffic[i].Name,
	GPS_INFO->FLARM_Traffic[i].Cn);
#endif

  if (i<0 || i>=FLARM_MAX_TRAFFIC) return;
  GPS_INFO->FLARM_Traffic[i].ID= 0;
  GPS_INFO->FLARM_Traffic[i].Name[0] = 0;
  GPS_INFO->FLARM_Traffic[i].Cn[0] = 0;
  GPS_INFO->FLARM_Traffic[i].Speed=0;
  GPS_INFO->FLARM_Traffic[i].Altitude=0;
  GPS_INFO->FLARM_Traffic[i].Status = LKT_EMPTY;
  GPS_INFO->FLARM_Traffic[i].AlarmLevel=0;
  GPS_INFO->FLARM_Traffic[i].RelativeNorth=0;
  GPS_INFO->FLARM_Traffic[i].RelativeEast=0;
  GPS_INFO->FLARM_Traffic[i].RelativeAltitude=0;
  GPS_INFO->FLARM_Traffic[i].IDType=0;
  GPS_INFO->FLARM_Traffic[i].TrackBearing=0;
  GPS_INFO->FLARM_Traffic[i].TurnRate=0;
  GPS_INFO->FLARM_Traffic[i].ClimbRate=0;
  GPS_INFO->FLARM_Traffic[i].Type=0;
  GPS_INFO->FLARM_Traffic[i].Time_Fix=0;
  GPS_INFO->FLARM_Traffic[i].Average30s=0;
  GPS_INFO->FLARM_Traffic[i].Locked = false;
  GPS_INFO->FLARM_Traffic[i].UpdateNameFlag = false;

}

#ifdef DEBUG_LKT
void FLARM_DumpSlot(NMEA_INFO *GPS_INFO,int i) {
  TCHAR dump[256];
  _stprintf(dump, _T("... DumpSlot (%d) status=%d id=<%lx> Name=<%s> Cn=<%s> Speed=%.0f rAlt=%.0f  %s"),
	i,
	GPS_INFO->FLARM_Traffic[i].Status,
	GPS_INFO->FLARM_Traffic[i].ID,
	GPS_INFO->FLARM_Traffic[i].Name,
	GPS_INFO->FLARM_Traffic[i].Cn,
	GPS_INFO->FLARM_Traffic[i].Speed,
	GPS_INFO->FLARM_Traffic[i].Altitude, 
	NEWLINE);
  StartupStore(dump);
}
#endif	

#include "InputEvents.h"

double FLARM_NorthingToLatitude = 0.0;
double FLARM_EastingToLongitude = 0.0;


BOOL NMEAParser::PFLAU(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *GPS_INFO)
{
  static int old_flarm_rx = 0;
  static bool sayflarmavailable=true; // 100325

  GPS_INFO->FLARM_Available = true;
  isFlarm = true;
  if ( sayflarmavailable ) {
	// LKTOKEN  _@M279_ = "FLARM DETECTED" 
	DoStatusMessage(gettext(TEXT("_@M279_")));
	sayflarmavailable=false;
  }

  // calculate relative east and north projection to lat/lon

  double delta_lat = 0.01;
  double delta_lon = 0.01;

  double dlat;
  DistanceBearing(GPS_INFO->Latitude, GPS_INFO->Longitude,
                  GPS_INFO->Latitude+delta_lat, GPS_INFO->Longitude,
                  &dlat, NULL);
  double dlon;
  DistanceBearing(GPS_INFO->Latitude, GPS_INFO->Longitude,
                  GPS_INFO->Latitude, GPS_INFO->Longitude+delta_lon,
                  &dlon, NULL);

  if ((fabs(dlat)>0.0)&&(fabs(dlon)>0.0)) {
    FLARM_NorthingToLatitude = delta_lat / dlat;
    FLARM_EastingToLongitude = delta_lon / dlon;
  } else {
    FLARM_NorthingToLatitude=0.0;
    FLARM_EastingToLongitude=0.0;
  }

  swscanf(String,
	  TEXT("%hu,%hu,%hu,%hu"),
	  &GPS_INFO->FLARM_RX, // number of received FLARM devices
	  &GPS_INFO->FLARM_TX, // Transmit status
	  &GPS_INFO->FLARM_GPS, // GPS status
	  &GPS_INFO->FLARM_AlarmLevel); // Alarm level of FLARM (0-3)

  // process flarm updates

  if ((GPS_INFO->FLARM_RX) && (old_flarm_rx==0)) {
    // traffic has appeared..
    InputEvents::processGlideComputer(GCE_FLARM_TRAFFIC);
  }
  if (GPS_INFO->FLARM_RX > old_flarm_rx) {
    // re-set suppression of gauge, as new traffic has arrived
    //    GaugeFLARM::Suppress = false;
  }
  if ((GPS_INFO->FLARM_RX==0) && (old_flarm_rx)) {
    // traffic has disappeared..
    InputEvents::processGlideComputer(GCE_FLARM_NOTRAFFIC);
  }
  // TODO feature: add another event for new traffic.

  old_flarm_rx = GPS_INFO->FLARM_RX;

  return FALSE;
}


int FLARM_FindSlot(NMEA_INFO *GPS_INFO, long Id)
{
  int i;
  for (i=0; i<FLARM_MAX_TRAFFIC; i++) {

	// find position in existing slot
	if (Id==GPS_INFO->FLARM_Traffic[i].ID) {
		//#ifdef DEBUG_LKT
		//StartupStore(_T("... FindSlot ID=%lx found in slot %d\n"),Id,i);
		//#endif
		return i;
	}
	// find old empty slot
  }
  // not found, so try to find an empty slot
  for (i=0; i<FLARM_MAX_TRAFFIC; i++) {
	if (GPS_INFO->FLARM_Traffic[i].ID<=0) { // 100327 <= was ==
		// this is a new target
		#ifndef NOFLARMGAUGE
		GaugeFLARM::Suppress = false;
		#endif
		#ifdef DEBUG_LKT
		StartupStore(_T("... FLARM ID=%lx assigned NEW SLOT=%d\n"),Id,i);
		#endif
		return i;
	}
  }
  // remove a zombie to make place
  int toremove=-1;
  for (i=0; i<FLARM_MAX_TRAFFIC; i++) {
	if ( (GPS_INFO->FLARM_Traffic[i].ID>0) && (GPS_INFO->FLARM_Traffic[i].Status==LKT_ZOMBIE) &&
		(!GPS_INFO->FLARM_Traffic[i].Locked) ) { 
		// if this is the first zombie, assign it and continue searching
		if (toremove==-1) {
			toremove=i;
		} else {
			// if this zombie is older than previous one
			if ( GPS_INFO->FLARM_Traffic[i].Time_Fix < GPS_INFO->FLARM_Traffic[toremove].Time_Fix ) {
				toremove=i;
			}
		}
	}
  }
  // did we find a zombie to remove?
  if (toremove>=0) {
	#ifdef DEBUG_LKT
	StartupStore(_T("... Removing OLDEST zombie:%s"),NEWLINE);
	FLARM_DumpSlot(GPS_INFO,toremove);
	#endif
	FLARM_EmptySlot(GPS_INFO,toremove);
	return toremove;
  }
  // remove a ghost to make place
  toremove=-1;
  for (i=0; i<FLARM_MAX_TRAFFIC; i++) {
	if ( (GPS_INFO->FLARM_Traffic[i].ID>0) && (GPS_INFO->FLARM_Traffic[i].Status==LKT_GHOST) &&
		(!GPS_INFO->FLARM_Traffic[i].Locked) ) { 
		// if this is the first ghost, assign it and continue searching
		if (toremove==-1) {
			toremove=i;
		} else {
			// if this ghost is older than previous one
			if ( GPS_INFO->FLARM_Traffic[i].Time_Fix < GPS_INFO->FLARM_Traffic[toremove].Time_Fix ) {
				toremove=i;
			}
		}
	}
  }
  // did we find a ghost to remove?
  if (toremove>=0) {
	#ifdef DEBUG_LKT
	StartupStore(_T("... Removing OLDEST ghost:%s"),NEWLINE);
	FLARM_DumpSlot(GPS_INFO,toremove);
	#endif
	FLARM_EmptySlot(GPS_INFO,toremove);
	return toremove;
  }

  #ifdef DEBUG_LKT
  StartupStore(_T("... ID=<%lx> NO SPACE in slots!\n"),Id);
  #endif
  // still not found and no empty slots left, buffer is full
  return -1;
}


#ifdef DSX
// warning, TODO FIX, calling AddMessage from wrong thread? CHECK
BOOL NMEAParser::PDSXT(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *GPS_INFO)
{
  TCHAR mbuf[300];

  if ( _tcslen(params[0]) >0) 
	wsprintf(mbuf,_T("MESSAGE FROM <%s>: %s"), params[0], params[1]);
  else
	wsprintf(mbuf,_T("MESSAGE: %s"),params[1] );
  Message::Lock(); // 091211
  Message::AddMessage(30000, 3, mbuf);
  Message::Unlock();
  #ifndef DISABLEAUDIO
  if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_TONEUP"));
  #endif

  return TRUE;


}
#endif

BOOL NMEAParser::PFLAA(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *GPS_INFO)
{
  int flarm_slot = 0;

  isFlarm = true;

  // 5 id, 6 digit hex
  long ID;
  swscanf(params[5],TEXT("%lx"), &ID);
//  unsigned long uID = ID;

  flarm_slot = FLARM_FindSlot(GPS_INFO, ID);
  if (flarm_slot<0) {
    // no more slots available,
	#ifdef DEBUG_LKT
	StartupStore(_T("... NO SLOTS for Flarm traffic, too many ids!%s"),NEWLINE);
	#endif
	return FALSE;
  }

  // before changing timefix, see if it was an old target back locked in!
  CheckBackTarget(flarm_slot);
  // and then set time of fix to current time
  GPS_INFO->FLARM_Traffic[flarm_slot].Time_Fix = GPS_INFO->Time;

  TCHAR nString[MAX_NMEA_LEN+1];
  unsigned int i, j;
  for (i=0, j=0; i<_tcslen(String); i++) {
	// if not a comma, copy and proceed
	if (String[i] != _T(',')) {
		nString[j++]=String[i];
		continue;
	}
	// there was a comma, but the next one is not a comma, so ok..
	if (String[i+1] != _T(',') ) {
		nString[j++]=String[i];
		continue;
	}
	// We have a bad ,, case that scanf cannot bear with, so we add a 0
	nString[j++] = String[i];
	nString[j++] = _T('0');
  }
  nString[j]=_T('\0');

  //#ifdef DEBUG_LKT
  //StartupStore(_T("PFLAA: %s%s"),nString,NEWLINE);
  //#endif

  _stscanf(nString,
	  TEXT("%hu,%lf,%lf,%lf,%hu,%lx,%lf,%lf,%lf,%lf,%hu"),
	  &GPS_INFO->FLARM_Traffic[flarm_slot].AlarmLevel, // unsigned short 0
	  &GPS_INFO->FLARM_Traffic[flarm_slot].RelativeNorth, //  1	
	  &GPS_INFO->FLARM_Traffic[flarm_slot].RelativeEast, //   2
	  &GPS_INFO->FLARM_Traffic[flarm_slot].RelativeAltitude, //  3
	  &GPS_INFO->FLARM_Traffic[flarm_slot].IDType, // unsigned short     4
	  &GPS_INFO->FLARM_Traffic[flarm_slot].ID, // 6 char hex
	  &GPS_INFO->FLARM_Traffic[flarm_slot].TrackBearing, // double       6
	  &GPS_INFO->FLARM_Traffic[flarm_slot].TurnRate, // double           7
	  &GPS_INFO->FLARM_Traffic[flarm_slot].Speed, // double              8 m/s
	  &GPS_INFO->FLARM_Traffic[flarm_slot].ClimbRate, // double          9 m/s
	  &GPS_INFO->FLARM_Traffic[flarm_slot].Type); // unsigned short     10
  // 1 relativenorth, meters  
  GPS_INFO->FLARM_Traffic[flarm_slot].Latitude = 
    GPS_INFO->FLARM_Traffic[flarm_slot].RelativeNorth *FLARM_NorthingToLatitude + GPS_INFO->Latitude;
  // 2 relativeeast, meters
  GPS_INFO->FLARM_Traffic[flarm_slot].Longitude = 
    GPS_INFO->FLARM_Traffic[flarm_slot].RelativeEast *FLARM_EastingToLongitude + GPS_INFO->Longitude;

  // we need to compare with BARO altitude FLARM relative Alt difference!
  if (GPS_INFO->BaroAltitude>0) // just to be sure
	GPS_INFO->FLARM_Traffic[flarm_slot].Altitude = GPS_INFO->FLARM_Traffic[flarm_slot].RelativeAltitude + GPS_INFO->BaroAltitude;
  else
	GPS_INFO->FLARM_Traffic[flarm_slot].Altitude = GPS_INFO->FLARM_Traffic[flarm_slot].RelativeAltitude + GPS_INFO->Altitude;



  GPS_INFO->FLARM_Traffic[flarm_slot].Average30s = flarmCalculations.Average30s(
	  GPS_INFO->FLARM_Traffic[flarm_slot].ID,
	  GPS_INFO->Time,
	  GPS_INFO->FLARM_Traffic[flarm_slot].Altitude);

  TCHAR *name = GPS_INFO->FLARM_Traffic[flarm_slot].Name;
  //TCHAR *cn = GPS_INFO->FLARM_Traffic[flarm_slot].Cn;
  // If there is no name yet, or if we have a pending update event..
  if (!_tcslen(name) || GPS_INFO->FLARM_Traffic[flarm_slot].UpdateNameFlag ) {

	#ifdef DEBUG_LKT
	if (GPS_INFO->FLARM_Traffic[flarm_slot].UpdateNameFlag ) {
		StartupStore(_T("... UpdateNameFlag for slot %d\n"),flarm_slot);
	} else {
		StartupStore(_T("... First lookup name for slot %d\n"),flarm_slot);
	}
	#endif

	GPS_INFO->FLARM_Traffic[flarm_slot].UpdateNameFlag=false; // clear flag first
	TCHAR *fname = LookupFLARMDetails(GPS_INFO->FLARM_Traffic[flarm_slot].ID);
	if (fname) {
		_tcsncpy(name,fname,MAXFLARMNAME);
		name[MAXFLARMNAME]=0;

		//  Now we have the name, so lookup also for the Cn
		// This will return either real Cn or Name, again
		TCHAR *cname = LookupFLARMCn(GPS_INFO->FLARM_Traffic[flarm_slot].ID);
		if (cname) {
			int cnamelen=_tcslen(cname);
			if (cnamelen<=MAXFLARMCN) {
				_tcscpy( GPS_INFO->FLARM_Traffic[flarm_slot].Cn, cname);
			} else {
				// else probably it is the Name again, and we create a fake Cn
				GPS_INFO->FLARM_Traffic[flarm_slot].Cn[0]=cname[0];
				GPS_INFO->FLARM_Traffic[flarm_slot].Cn[1]=cname[cnamelen-2];
				GPS_INFO->FLARM_Traffic[flarm_slot].Cn[2]=cname[cnamelen-1];
				GPS_INFO->FLARM_Traffic[flarm_slot].Cn[3]=_T('\0');
			}
		} else {
			_tcscpy( GPS_INFO->FLARM_Traffic[flarm_slot].Cn, _T("Err"));
		}

		#ifdef DEBUG_LKT
		StartupStore(_T("... PFLAA Name to FlarmSlot=%d ID=%lx Name=<%s> Cn=<%s>\n"),
			flarm_slot,
	  		GPS_INFO->FLARM_Traffic[flarm_slot].ID,
			GPS_INFO->FLARM_Traffic[flarm_slot].Name,
			GPS_INFO->FLARM_Traffic[flarm_slot].Cn);
		#endif
	} else {
		// Else we NEED to set a name, otherwise it will constantly search for it over and over..
		name[0]=_T('?');
		name[1]=_T('\0');
		GPS_INFO->FLARM_Traffic[flarm_slot].Cn[0]=_T('?');
		GPS_INFO->FLARM_Traffic[flarm_slot].Cn[1]=_T('\0');
		
		#ifdef DEBUG_LKT
		StartupStore(_T("... New FlarmSlot=%d ID=%lx with no name, assigned a \"?\"\n"),
			flarm_slot,
	  		GPS_INFO->FLARM_Traffic[flarm_slot].ID);
		#endif
	}
  }

  #ifdef DEBUG_LKT
  StartupStore(_T("... PFLAA GPS_INFO slot=%d ID=%lx name=<%s> cn=<%s> rAlt=%.0f Track=%.0f Speed=%.0f Climb=%.1f Baro=%f FlAlt=%f\n"),
	flarm_slot,
	GPS_INFO->FLARM_Traffic[flarm_slot].ID,
	GPS_INFO->FLARM_Traffic[flarm_slot].Name,
	GPS_INFO->FLARM_Traffic[flarm_slot].Cn,
	GPS_INFO->FLARM_Traffic[flarm_slot].RelativeAltitude,
	GPS_INFO->FLARM_Traffic[flarm_slot].TrackBearing,
	GPS_INFO->FLARM_Traffic[flarm_slot].Speed,
	GPS_INFO->FLARM_Traffic[flarm_slot].ClimbRate,
	GPS_INFO->BaroAltitude,
	GPS_INFO->FLARM_Traffic[flarm_slot].Altitude);
  #endif

  //  update Virtual Waypoint for target FLARM
  if (flarm_slot == LKTargetIndex) {
	WayPointList[RESWP_FLARMTARGET].Latitude   = GPS_INFO->FLARM_Traffic[LKTargetIndex].Latitude;
	WayPointList[RESWP_FLARMTARGET].Longitude  = GPS_INFO->FLARM_Traffic[LKTargetIndex].Longitude;
	WayPointList[RESWP_FLARMTARGET].Altitude   = GPS_INFO->FLARM_Traffic[LKTargetIndex].Altitude;
  }


  return FALSE;
}

void NMEAParser::TestRoutine(NMEA_INFO *GPS_INFO) {
#ifdef DEBUG
#ifndef GNAV
  static int i=90;
  static TCHAR t1[] = TEXT("1,1,1,1");
  static TCHAR t2[] = TEXT("1,300,500,220,2,DD927B,0,-4.5,30,-1.4,1");
  static TCHAR t3[] = TEXT("0,0,1200,50,2,DD9146,270,-4.5,30,-1.4,1");
  //  static TCHAR b50[] = TEXT("0,.1,.0,0,0,1.06,0,-222");
  //  static TCHAR t4[] = TEXT("-3,500,1024,50");

  //  nmeaParser1.ParseNMEAString_Internal(TEXT("$PTAS1,201,200,02583,000*2A"), GPS_INFO);
  //  nmeaParser1.ParseNMEAString_Internal(TEXT("$GPRMC,082430.00,A,3744.09096,S,14426.16069,E,0.520294.90,301207,,,A*77"), GPS_INFO);
  //  nmeaParser1.ParseNMEAString_Internal(TEXT("$GPGGA,082430.00,3744.09096,S,1426.16069,E,1,08,1.37,157.6,M,-4.9,M,,*5B"), GPS_INFO);

  QNH=1013.25;
  double h;
  double altraw= 5.0;
  h = AltitudeToQNHAltitude(altraw);
  QNH = FindQNH(altraw, 50.0);
  h = AltitudeToQNHAltitude(altraw);

  i++;

  if (i>100) {
    i=0;
  }
  if (i<50) {
    GPS_INFO->FLARM_Available = true;
    TCHAR ctemp[MAX_NMEA_LEN];
    TCHAR *params[MAX_NMEA_PARAMS];
    size_t nr;
    nr = nmeaParser1.ExtractParameters(t1, ctemp, params, MAX_NMEA_PARAMS);
    nmeaParser1.PFLAU(t1, params, nr, GPS_INFO);
    nr = nmeaParser1.ExtractParameters(t2, ctemp, params, MAX_NMEA_PARAMS);
    nmeaParser1.PFLAA(t2, params, nr, GPS_INFO);
    nr = nmeaParser1.ExtractParameters(t3, ctemp, params, MAX_NMEA_PARAMS);
    nmeaParser1.PFLAA(t3, params, nr, GPS_INFO);
  }
#endif
#endif
}


bool	EnableLogNMEA = false;

// New LogNMEA
void LogNMEA(TCHAR* text) {

  TCHAR	buffer[LKSIZEBUFFERPATH];
  char snmea[LKSIZENMEA];

  static char	fpname[LKSIZEBUFFERPATH];
  static bool	doinit=true;
  static bool	wasWriting=false;
  static bool	alreadyWarned=false;
  static FILE *logfp;

  if (!EnableLogNMEA) {
	if (wasWriting) {
		fclose(logfp);
		wasWriting=false;
	}
	return;
  }

  if (doinit) {
	LocalPath(buffer,TEXT(LKD_LOGS));
	sprintf(fpname,"%S\\NMEA_%04d-%02d-%02d-%02d-%02d-%02d.txt",buffer,GPS_INFO.Year,GPS_INFO.Month, GPS_INFO.Day,
		GPS_INFO.Hour, GPS_INFO.Minute, GPS_INFO.Second);
	doinit=false;
  }

  if (!wasWriting) {
	logfp=fopen(fpname,"a");
	if (logfp == NULL) {
		if (!alreadyWarned) {
			DoStatusMessage(_T("ERR-049 Cannot open NMEA log"));
			alreadyWarned=true;
		}
		return;
	}
	wasWriting=true;
  }

  sprintf(snmea,"%S",text);
  short l=strlen(snmea);
  if (l<6) return;
  if ( snmea[l-3]==0x0d && snmea[l-2]==0x0d) {
	snmea[l-2]=0x0a;
	snmea[l-1]=0;
  }

  fprintf(logfp,"%s",snmea);
  
}


bool NMEAParser::PortIsFlarm(int device) {
  switch (device) {
  case 0: 
    return nmeaParser1.isFlarm;
  case 1:
    return nmeaParser2.isFlarm;
  default:
    return false;
  };
}

// Warn about an old locked zombie back visible
void CheckBackTarget(int flarmslot) {
  if ( !GPS_INFO.FLARM_Traffic[flarmslot].Locked ) return;
  if ( GPS_INFO.FLARM_Traffic[flarmslot].Status != LKT_ZOMBIE ) return;

  // if more than 15 minutes ago, warn pilot with full message and sound
  if ( (GPS_INFO.Time - GPS_INFO.FLARM_Traffic[flarmslot].Time_Fix) >=900) {
	// LKTOKEN  _@M674_ = "TARGET BACK VISIBLE" 
	DoStatusMessage(gettext(TEXT("_@M674_")));
	#ifndef DISABLEAUDIO
	if (EnableSoundModes) LKSound(_T("TARGVISIBLE.WAV"));
	#endif
  } else {
	// otherwise a simple sound
	#ifndef DISABLEAUDIO
	if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_DRIP"));
	#endif
  }
}

