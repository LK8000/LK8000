/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Parser.cpp,v 8.12 2010/12/12 16:14:28 root Exp root $

*/

#include "externs.h"
#include "Baro.h"
#include "Logger.h"
#include "Geoid.h"
#if defined(PNA) && defined(UNDER_CE)
#include "LKHolux.h"
#include "LKRoyaltek3200.h"
#endif


extern double EastOrWest(double in, TCHAR EoW);
extern double NorthOrSouth(double in, TCHAR NoS);
extern double MixedFormatToDegrees(double mixed);
extern bool UpdateBaroSource( NMEA_INFO* pGPS, const short parserid, const PDeviceDescriptor_t d, const double fAlt);
extern int NAVWarn(TCHAR c);

extern double trackbearingminspeed; // minimal speed to use gps bearing, init by UpdateMonitor
void CheckBackTarget(int flarmslot);


unsigned LastRMZHB=0;	 // common to both devA and devB.
int NMEAParser::StartDay = -1;


// #define DEBUGSEQ	1
// #define DEBUGBARO	1

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
  RMCAvailable = false;
  TASAvailable = false; // 100411
  RMZDelayed = 3; // wait for this to be zero before using RMZ.
 
  GGAtime=0;
  RMCtime=0;
  GLLtime=0; 
  LastTime = 0;
}

#ifdef UNDER_CE
BOOL NMEAParser::ParseGPS_POSITION(int Idx, const GPS_POSITION& loc, NMEA_INFO& GPSData) {
    LKASSERT(!ReplayLogger::IsEnabled());

    PDeviceDescriptor_t pdev = devX(Idx)
    if(pdev) {
      pdev->HB = LKHearthBeats;
      return pdev->nmeaParser.ParseGPS_POSITION_internal(loc, GPSData);
    }
    return FALSE;
}

BOOL NMEAParser::ParseGPS_POSITION_internal(const GPS_POSITION& loc, NMEA_INFO& GPSData) {

    gpsValid = !(loc.dwFlags & GPS_DATA_FLAGS_HARDWARE_OFF);
    if (!gpsValid) {
        return TRUE;
    }
    GPSCONNECT=TRUE;

    switch (loc.FixType) {
        case GPS_FIX_UNKNOWN:
            gpsValid = false;
            break;
        case GPS_FIX_2D:
        case GPS_FIX_3D:
            gpsValid = true;
            break;
        default:
            break;
    }

    if (loc.dwValidFields & GPS_VALID_SATELLITE_COUNT) {
        nSatellites = loc.dwSatelliteCount;
    }

    if(!activeGPS) {
        return TRUE;
    }

    GPSData.SatellitesUsed = nSatellites;
    GPSData.NAVWarning = !gpsValid;
    
    if (loc.dwValidFields & GPS_VALID_UTC_TIME) {
        GPSData.Hour = loc.stUTCTime.wHour;
        GPSData.Minute = loc.stUTCTime.wMinute;
        GPSData.Second = loc.stUTCTime.wSecond;
        GPSData.Month = loc.stUTCTime.wMonth;
        GPSData.Day = loc.stUTCTime.wDay;
        GPSData.Year = loc.stUTCTime.wYear;
        if(!TimeHasAdvanced(TimeModify(&GPSData, StartDay), &GPSData)) {
            return FALSE;
        }
    }
    if (loc.dwValidFields & GPS_VALID_LATITUDE) {
        GPSData.Latitude = loc.dblLatitude;
    }
    if (loc.dwValidFields & GPS_VALID_LONGITUDE) {
        GPSData.Longitude = loc.dblLongitude;
    }
    if (loc.dwValidFields & GPS_VALID_SPEED) {
        GPSData.Speed = KNOTSTOMETRESSECONDS * loc.flSpeed;
    }
    if (loc.dwValidFields & GPS_VALID_HEADING) {
      	if (GPSData.Speed>trackbearingminspeed) {
            GPSData.TrackBearing = AngleLimit360(loc.flHeading);
        }
    }
    if (loc.dwValidFields & GPS_VALID_MAGNETIC_VARIATION) {

    }
    
    if (loc.dwValidFields & GPS_VALID_ALTITUDE_WRT_SEA_LEVEL) {
        GPSData.Altitude = loc.flAltitudeWRTSeaLevel;
        GPSData.Altitude += (GPSAltitudeOffset/1000); // BUGFIX 100429
    }
    
#if 0    
    if (loc.dwValidFields & GPS_VALID_ALTITUDE_WRT_ELLIPSOID) {
 
        /* MSDN says..
         * "flAltitudeWRTEllipsoid : Altitude, in meters, with respect to the WGS84 ellipsoid. "
         * "flAltitudeWRTSeaLevel : Altitude, in meters, with respect to sea level. "
         * 
         * But when I get this structure, flAltitudeWRTSeaLevel field has proper value. But,
         * flAltitudeWRTEllipsoid field has different meaning value.
         * 
         * But flAltitudeWRTEllipsoid field contains just geodial separation.
         */
    } 
#endif
    
    if (loc.dwValidFields & GPS_VALID_POSITION_DILUTION_OF_PRECISION) {

    }
    if (loc.dwValidFields & GPS_VALID_HORIZONTAL_DILUTION_OF_PRECISION) {

    }
    if (loc.dwValidFields & GPS_VALID_VERTICAL_DILUTION_OF_PRECISION) {

    }
    if (loc.dwValidFields & GPS_VALID_SATELLITES_USED_PRNS) {

    }
    if (loc.dwValidFields & GPS_VALID_SATELLITES_IN_VIEW) {
 
    }
    if (loc.dwValidFields & GPS_VALID_SATELLITES_IN_VIEW_PRNS) {

    }
    if (loc.dwValidFields & GPS_VALID_SATELLITES_IN_VIEW_ELEVATION) {

    }
    if (loc.dwValidFields & GPS_VALID_SATELLITES_IN_VIEW_AZIMUTH) {

    }
    if (loc.dwValidFields & GPS_VALID_SATELLITES_IN_VIEW_SIGNAL_TO_NOISE_RATIO) {

    }
    if(gpsValid && !GPSData.NAVWarning && GPSData.SatellitesUsed == 0) {
        GPSData.SatellitesUsed = -1;
    }
    
    TriggerGPSUpdate();
    
    return TRUE;
}
#endif

BOOL NMEAParser::ParseNMEAString_Internal(TCHAR *String, NMEA_INFO *pGPS)
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
          return PDSXT(&String[7], params + 1, n_params, pGPS);
        }
#endif 


      if(_tcscmp(params[0] + 1,TEXT("PTAS1"))==0)
        {
          return PTAS1(&String[7], params + 1, n_params, pGPS);
        }

      // FLARM sentences

      if(_tcscmp(params[0] + 1,TEXT("PFLAV"))==0)
        {
          return PFLAV(&String[7], params + 1, n_params, pGPS);
        }

      if(_tcscmp(params[0] + 1,TEXT("PFLAA"))==0)
        {
          return PFLAA(&String[7], params + 1, n_params, pGPS);
        }

      if(_tcscmp(params[0] + 1,TEXT("PFLAU"))==0)
        {
          return PFLAU(&String[7], params + 1, n_params, pGPS);
        }

      if(_tcscmp(params[0] + 1,TEXT("PGRMZ"))==0)
	{
	  return RMZ(&String[7], params + 1, n_params, pGPS);
	}
      if(_tcscmp(params[0] + 1,TEXT("PLKAS"))==0)
        {
          return PLKAS(&String[7], params + 1, n_params, pGPS);
        }
      return FALSE;
    }

  if(_tcscmp(params[0] + 3,TEXT("GSA"))==0)
    {
      return GSA(&String[7], params + 1, n_params, pGPS);
    }
  if(_tcscmp(params[0] + 3,TEXT("GLL"))==0)
    {
      //    return GLL(&String[7], params + 1, n_params, pGPS);
      return FALSE;
    }
  if(_tcscmp(params[0] + 3,TEXT("RMB"))==0)
    {
      //return RMB(&String[7], params + 1, n_params, pGPS);
          return FALSE;
      }
  if(_tcscmp(params[0] + 3,TEXT("RMC"))==0)
    {
      return RMC(&String[7], params + 1, n_params, pGPS);
    }
  if(_tcscmp(params[0] + 3,TEXT("GGA"))==0)
    {
      return GGA(&String[7], params + 1, n_params, pGPS);
    }
  if(_tcscmp(params[0] + 3,TEXT("VTG"))==0)
    {
      return VTG(&String[7], params + 1, n_params, pGPS);
    }
  if(_tcscmp(params[0] + 1,TEXT("HCHDG"))==0)
    {
      return HCHDG(&String[7], params + 1, n_params, pGPS);
    }

  return FALSE;
}


//
// Make time absolute, over 86400seconds when day is changing
// We need a valid date to use it. We are relying on StartDay.
//
#ifdef OLD_TIME_MODIFY
double NMEAParser::TimeModify(double FixTime, NMEA_INFO* pGPS)
{
  static  int day_difference=0, previous_months_day_difference=0;
  double hours, mins,secs;
  
  hours = FixTime / 10000;
  pGPS->Hour = (int)hours;

  mins = FixTime / 100;
  mins = mins - (pGPS->Hour*100);
  pGPS->Minute = (int)mins;

  secs = FixTime - (pGPS->Hour*10000) - (pGPS->Minute*100);
  pGPS->Second = (int)secs;

  FixTime = secs + (pGPS->Minute*60) + (pGPS->Hour*3600);
#else
double TimeModify(const TCHAR* StrTime, NMEA_INFO* pGPS, int& StartDay) {
    double secs = 0.0;

    if (_istdigit(StrTime[0]) && _istdigit(StrTime[1])) {
        pGPS->Hour = (StrTime[0] - '0')*10 + (StrTime[1] - '0');
    }
    if (_istdigit(StrTime[2]) && _istdigit(StrTime[3])) {
        pGPS->Minute = (StrTime[2] - '0')*10 + (StrTime[3] - '0');
    }
    if (_istdigit(StrTime[4]) && _istdigit(StrTime[5])) {
        pGPS->Second = (StrTime[4] - '0')*10 + (StrTime[5] - '0');
    }

    if (StrTime[6] == '.') {
        int i = 7;
        while (_istdigit(StrTime[i])) {
            double tmp = (StrTime[i] - '0')*0.1;
            for (int j = 7; j < i; ++j) {
                tmp *= 0.1;
            }
            secs += tmp;
            ++i;
        }
    }
    return secs + TimeModify(pGPS, StartDay);
}

double TimeModify(NMEA_INFO* pGPS, int& StartDay) {
  static int day_difference = 0, previous_months_day_difference = 0;

  double FixTime = (double) (pGPS->Second + (pGPS->Minute * 60) + (pGPS->Hour * 3600));
#endif
  if ((StartDay== -1) && (pGPS->Day != 0)) {
    StartupStore(_T(". First GPS DATE: %d-%d-%d  %s%s"), pGPS->Year, pGPS->Month, pGPS->Day,WhatTimeIsIt(),NEWLINE);
    StartDay = pGPS->Day;
    day_difference=0;
    previous_months_day_difference=0;
  }
  if (StartDay != -1) {
    if (pGPS->Day < StartDay) {
      // detect change of month (e.g. day=1, startday=26)
      previous_months_day_difference=day_difference+1;
      day_difference=0;
      StartDay = pGPS->Day;
      StartupStore(_T(". Change GPS DATE to NEW MONTH: %d-%d-%d  (%d days running)%s"), 
	pGPS->Year, pGPS->Month, pGPS->Day,previous_months_day_difference,NEWLINE);
    }
    if ( (pGPS->Day-StartDay)!=day_difference) {
      StartupStore(_T(". Change GPS DATE: %d-%d-%d  %s%s"), pGPS->Year, pGPS->Month, pGPS->Day,WhatTimeIsIt(),NEWLINE);
    }

    day_difference = pGPS->Day-StartDay;
    if ((day_difference+previous_months_day_difference)>0) {
      // Add seconds to fix time so time doesn't wrap around when
      // going past midnight in UTC
      FixTime += (day_difference+previous_months_day_difference) * 86400;
    }
  }
  return FixTime;
}

bool NMEAParser::TimeHasAdvanced(double ThisTime, NMEA_INFO *pGPS) {

  // If simulating, we might be in the future already.
  // We CANNOT check for <= because this check may be done by several GGA RMC GLL etc. sentences
  // among the same quantum time
  if(ThisTime< LastTime) {
    #if TESTBENCH
    StartupStore(_T("... TimeHasAdvanced BACK in time: Last=%f This=%f   %s\n"), LastTime, ThisTime,WhatTimeIsIt());
    #endif
    LastTime = ThisTime;
    StartDay = -1; // reset search for the first day
    MasterTimeReset();
    return false;
  } else {
    pGPS->Time = ThisTime;
    LastTime = ThisTime;
    return true;
  }
}

BOOL NMEAParser::GSA(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *pGPS)
{

  #if LOGFRECORD
  int iSatelliteCount =0;

  if (!activeGPS) return TRUE;

  // satellites are in items 4-15 of GSA string (4-15 is 1-indexed)
  // but 1st item in string is not passed, so start at item 3
  for (int i = 0; i < MAXSATELLITES; i++)
  {
    if (3+i < (int) nparams) {
      pGPS->SatelliteIDs[i] = (int)(StrToDouble(params[2+i], NULL)); // 2 because params is 0-index
      if (pGPS->SatelliteIDs[i] > 0)
	iSatelliteCount ++;
    }
  }
  return TRUE;
  #else
  return TRUE;
  #endif
}

// we need to parse GLL as well because it can mark the start of a new quantum data
// followed by values with no data, ex. altitude, vario, etc.
BOOL NMEAParser::GLL(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *pGPS)
{
  if(nparams < 6) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid GLL sentence, nparams=%u%s"),(unsigned)nparams,NEWLINE));
    // max index used is 5...
    return FALSE;
  }
  
  gpsValid = !NAVWarn(params[5][0]);
  GPSCONNECT=TRUE;

  if (!activeGPS) return TRUE;

  pGPS->NAVWarning = !gpsValid;
  
  // use valid time with invalid fix
  GLLtime = StrToDouble(params[4],NULL);
  if (!RMCAvailable &&  !GGAAvailable && (GLLtime>0)) {
#ifndef OLD_TIME_MODIFY
	double ThisTime = TimeModify(params[4], pGPS, StartDay);
#else
	double ThisTime = TimeModify(GLLtime, pGPS);
#endif 
	if (!TimeHasAdvanced(ThisTime, pGPS)) return FALSE; 
  }
  if (!gpsValid) return FALSE;
  
  double tmplat;
  double tmplon;
  
  tmplat = MixedFormatToDegrees(StrToDouble(params[0], NULL));
  tmplat = NorthOrSouth(tmplat, params[1][0]);
  
  tmplon = MixedFormatToDegrees(StrToDouble(params[2], NULL));
  tmplon = EastOrWest(tmplon,params[3][0]);
  
  if (!((tmplat == 0.0) && (tmplon == 0.0))) {
	pGPS->Latitude = tmplat;
	pGPS->Longitude = tmplon;
  } else {
    
  }
  return TRUE;

} // END GLL



BOOL NMEAParser::RMB(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *pGPS)
{
  (void)pGPS;
  (void)String;
  (void)params;
  (void)nparams;
  /* we calculate all this stuff now 
  TCHAR ctemp[MAX_NMEA_LEN];

  pGPS->NAVWarning = NAVWarn(params[0][0]);

  pGPS->CrossTrackError = NAUTICALMILESTOMETRES * StrToDouble(params[1], NULL);
  pGPS->CrossTrackError = LeftOrRight(pGPS->CrossTrackError,params[2][0]);

  _tcscpy(ctemp, params[4]);
  ctemp[WAY_POINT_ID_SIZE] = '\0';
  _tcscpy(pGPS->WaypointID,ctemp);

  pGPS->WaypointDistance = NAUTICALMILESTOMETRES * StrToDouble(params[9], NULL);
  pGPS->WaypointBearing = StrToDouble(params[10], NULL);
  pGPS->WaypointSpeed = KNOTSTOMETRESSECONDS * StrToDouble(params[11], NULL);
  */

  return TRUE;

} // END RMB



BOOL NMEAParser::VTG(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *pGPS)
{
  if(nparams < 5) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid VTG sentence, nparams=%u%s"),(unsigned)nparams,NEWLINE));
    // max index used is 4...
    return FALSE;
  }
  
  // GPSCONNECT = TRUE; // 121127  NO! VTG gives no position fix
  if (!activeGPS) return TRUE;

  if (RMCAvailable) return FALSE;
  double speed=0;

  // if no valid fix, we dont get speed either!
  if (gpsValid)
  {
	speed = StrToDouble(params[4], NULL);

	pGPS->Speed = KNOTSTOMETRESSECONDS * speed;
  
	if (ISCAR)
	if (pGPS->Speed>trackbearingminspeed) {
		pGPS->TrackBearing = AngleLimit360(StrToDouble(params[0], NULL));
	}
  }

  // if we are here, no RMC is available but if no GGA also, we are in troubles: to check!
  if (!GGAAvailable) {
	TriggerGPSUpdate();
  }

  return TRUE;

} // END VTG





BOOL NMEAParser::RMC(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *pGPS)
{
  if(nparams < 9) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid RMC sentence, nparams=%u%s"),(unsigned)nparams,NEWLINE));
    // max index used is 8...
    return FALSE;
  }
  
  TCHAR *Stop;
  static bool logbaddate=true;
  double speed=0;

  gpsValid = !NAVWarn(params[1][0]);

  GPSCONNECT = TRUE;    
  RMCAvailable=true; // 100409

  #ifdef PNA
  if (DeviceIsGM130) {

	double ps = GM130BarPressure();
	RMZAltitude = (1 - pow(fabs(ps / QNH),  0.190284)) * 44307.69;
	// StartupStore(_T("....... Pressure=%.0f QNH=%.2f Altitude=%.1f\n"),ps,QNH,RMZAltitude);

	RMZAvailable = TRUE;

	UpdateBaroSource(pGPS, BARO__GM130, NULL,   RMZAltitude);
  }
  if (DeviceIsRoyaltek3200) {
	if (Royaltek3200_ReadBarData()) {
		double ps = Royaltek3200_GetPressure();
		RMZAltitude = (1 - pow(fabs(ps / QNH),  0.190284)) * 44307.69;

		#if 0
		pGPS->TemperatureAvailable=true;
		pGPS->OutsideAirTemperature = Royaltek3200_GetTemperature();
		#endif
	}

	RMZAvailable = TRUE;

	UpdateBaroSource(pGPS, BARO__ROYALTEK3200,  NULL,  RMZAltitude);

  }
  #endif // PNA

  if (!activeGPS) {
	// Before ignoring anything else, commit RMZ altitude otherwise it will be ignored!
	if(RMZAvailable) {
		UpdateBaroSource(pGPS, isFlarm? BARO__RMZ_FLARM:BARO__RMZ, NULL, RMZAltitude);
	}
	return TRUE;
  }

  // if no valid fix, we dont get speed either!
  if (gpsValid)
  {
	// speed is in knots, 2 = 3.7kmh
	speed = StrToDouble(params[6], NULL);
  }
  
  pGPS->NAVWarning = !gpsValid;

  // say we are updated every time we get this,
  // so infoboxes get refreshed if GPS connected
  // the RMC sentence marks the start of a new fix, so we force the old data to be saved for calculations
        // note that Condor sends no date..
        if ((_tcslen(params[8]) <6) && !DevIsCondor) {
		#if TESTBENCH
		StartupStore(_T(".... RMC date field empty, skip sentence!\n"));
		#endif
		return TRUE;
	}

	// Even with no valid position, we let RMC set the time and date if valid
	long gy, gm, gd;
	gy = _tcstol(&params[8][4], &Stop, 10) + 2000;   
	params[8][4] = '\0';
	gm = _tcstol(&params[8][2], &Stop, 10); 
	params[8][2] = '\0';
	gd = _tcstol(&params[8][0], &Stop, 10); 

	// SeeYou PC is sending NMEA sentences with RMC date 2072-02-27
	if ( ((gy > 1980) && (gy <2100) ) && (gm != 0) && (gd != 0) ) { 
		pGPS->Year = gy;
		pGPS->Month = gm;
		pGPS->Day = gd;

force_advance:
		RMCtime = StrToDouble(params[0],NULL);
#ifndef OLD_TIME_MODIFY
		double ThisTime = TimeModify(params[0], pGPS, StartDay);
#else
		double ThisTime = TimeModify(RMCtime, pGPS);
#endif
		// RMC time has priority on GGA and GLL etc. so if we have it we use it at once
		if (!TimeHasAdvanced(ThisTime, pGPS)) {
			#if DEBUGSEQ
			StartupStore(_T("..... RMC time not advanced, skipping \n")); // 31C
			#endif
			return FALSE;
		}
			
	}  else {
		if (DevIsCondor) {
			#if DEBUGSEQ
			StartupStore(_T(".. Condor not sending valid date, using 1.1.2012%s"),NEWLINE);
			#endif
			gy=2012; gm=1; gd=1;
			goto force_advance;
		}

		if (gpsValid && logbaddate) { // 091115
			StartupStore(_T("------ NMEAParser:RMC Receiving an invalid or null DATE from GPS%s"),NEWLINE);
			StartupStore(_T("------ NMEAParser: Date received is y=%ld m=%ld d=%ld%s"),gy,gm,gd,NEWLINE); // 100422
			StartupStore(_T("------ This message will NOT be repeated. %s%s"),WhatTimeIsIt(),NEWLINE);
			DoStatusMessage(MsgToken(875));
			logbaddate=false;
		}
		gy=2012; gm=2; gd=30;	// an impossible date!
		goto force_advance;
		 
	}

  if (gpsValid) { 
	double tmplat;
	double tmplon;

	tmplat = MixedFormatToDegrees(StrToDouble(params[2], NULL));
	tmplat = NorthOrSouth(tmplat, params[3][0]);
	  
	tmplon = MixedFormatToDegrees(StrToDouble(params[4], NULL));
	tmplon = EastOrWest(tmplon,params[5][0]);
  
	if (!((tmplat == 0.0) && (tmplon == 0.0))) {
		pGPS->Latitude = tmplat;
		pGPS->Longitude = tmplon;
	}
  
	pGPS->Speed = KNOTSTOMETRESSECONDS * speed;
  
	if (pGPS->Speed>trackbearingminspeed) {
		pGPS->TrackBearing = AngleLimit360(StrToDouble(params[7], NULL));
	}
  } // gpsvalid 091108

#ifdef WIN32
  // As soon as we get a fix for the first time, set the
  // system clock to the GPS time.
  static bool sysTimeInitialised = false;
  
  if (!pGPS->NAVWarning && (gpsValid)) {
	if (SetSystemTimeFromGPS) {
		if (!sysTimeInitialised) {
			if ( ( pGPS->Year > 1980 && pGPS->Year<2100) && ( pGPS->Month > 0) && ( pGPS->Hour > 0)) {
        
				sysTimeInitialised =true; // Attempting only once
				SYSTEMTIME sysTime;
				// ::GetSystemTime(&sysTime);
				int hours = (int)pGPS->Hour;
				int mins = (int)pGPS->Minute;
				int secs = (int)pGPS->Second;
				sysTime.wYear = (unsigned short)pGPS->Year;
				sysTime.wMonth = (unsigned short)pGPS->Month;
				sysTime.wDay = (unsigned short)pGPS->Day;
				sysTime.wHour = (unsigned short)hours;
				sysTime.wMinute = (unsigned short)mins;
				sysTime.wSecond = (unsigned short)secs;
				sysTime.wMilliseconds = 0;
				::SetSystemTime(&sysTime);
			}
		}
	}
  }
#else
#warning "Set system clock to the GPS time not implemented."
#endif

  if(RMZAvailable) {
	UpdateBaroSource(pGPS, BARO__RMZ, NULL,  RMZAltitude);
  }
  if (!GGAAvailable) {
	// update SatInUse, some GPS receiver dont emmit GGA sentance
	if (!gpsValid) { 
		pGPS->SatellitesUsed = 0;
	} else {
		pGPS->SatellitesUsed = -1;
	}
  }
  
  if ( !GGAAvailable || (GGAtime == RMCtime)  )  {
	#if DEBUGSEQ
	StartupStore(_T("... RMC trigger gps, GGAtime==RMCtime\n")); // 31C
	#endif
	TriggerGPSUpdate(); 
  }

  return TRUE;

} // END RMC




BOOL NMEAParser::GGA(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *pGPS)
{
  if(nparams < 11) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid GGA sentence, nparams=%u%s"),(unsigned)nparams,NEWLINE));
    // max index used is 10...
    return FALSE;
  }

  GGAAvailable = TRUE;
  GPSCONNECT = TRUE;     // 091208

  // this will force gps invalid but will NOT assume gps valid!
  nSatellites = (int)(min(16.0, StrToDouble(params[6], NULL)));
  if (nSatellites==0) {
	gpsValid = false;
  }

    /*
     * Fix quality : 
     *  0 = invalid
     *  1 = GPS fix (SPS)
     *  2 = DGPS fix
     *  3 = PPS fix
     *  4 = Real Time Kinematic
     *  5 = Float RTK
     *  6 = estimated (dead reckoning) (2.3 feature)
     *  7 = Manual input mode
     *  8 = Simulation mode
     */
  
  double ggafix = StrToDouble(params[5],NULL);
  if ( ggafix==0 || ggafix>5 ) {
	#ifdef DEBUG_GPS
	if (ggafix>5) StartupStore(_T("------ GGA DEAD RECKON fix skipped%s"),NEWLINE);
	#endif
#ifdef YDEBUG
    // in debug we need accept manual or simulated fix
    gpsValid = (ggafix == 7 || ggafix == 8); 
#else
	gpsValid=false;
#endif    
  } else {
	gpsValid=true;
  }

  if (!activeGPS) {
	if(RMZAvailable && !RMCAvailable)
	{
		UpdateBaroSource(pGPS, isFlarm? BARO__RMZ_FLARM:BARO__RMZ, NULL, RMZAltitude);
	}
	return TRUE;
  }

  pGPS->SatellitesUsed = nSatellites; // 091208
  pGPS->NAVWarning = !gpsValid; // 091208

  GGAtime=StrToDouble(params[0],NULL);
  // Even with invalid fix, we might still have valid time
  // I assume that 0 is invalid, and I am very sorry for UTC time 00:00 ( missing a second a midnight).
  // is better than risking using 0 as valid, since many gps do not respect any real nmea standard
  //
  // Update 121215: do not update time with GGA if RMC is found, because at 00UTC only RMC will set the date change!
  // Remember that we trigger update of calculations when we get GGA.
  // So what happens if the gps sequence is GGA and then RMC?
  //    2359UTC:
  //           GGA , old date, trigger gps calc
  //           RMC,  old date
  //    0000UTC:
  //           GGA, old date even if new date will come for the same quantum,
  //                GGAtime>0, see (*)
  //                BANG! oldtime from RMC is 2359, new time from GGA is 0, time is in the past!
  //
  // If the gps is sending first RMC, this problem does not appear of course.
  //
  // (*) IMPORTANT> GGAtime at 00UTC will most likely be >0! Because time is in hhmmss.ss  .ss is always >0!!
  // We check GGAtime, RMCtime, GLLtime etc. for 0 because in case of error the gps will send 000000.000 !!
  //
  if ( (!RMCAvailable && (GGAtime>0)) || ((GGAtime>0) && (GGAtime == RMCtime))  ) {  // RMC already came in same time slot

	#if DEBUGSEQ
	StartupStore(_T("... GGA update time = %f RMCtime=%f\n"),GGAtime,RMCtime); // 31C
	#endif
#ifndef OLD_TIME_MODIFY
	double ThisTime = TimeModify(params[0], pGPS, StartDay);
#else
	double ThisTime = TimeModify(GGAtime, pGPS);
#endif
	if (!TimeHasAdvanced(ThisTime, pGPS)) {
		#if DEBUGSEQ
		StartupStore(_T(".... GGA time not advanced, skip\n")); // 31C
		#endif
		return FALSE;
	}
  }
  if (gpsValid) {
	double tmplat;
	double tmplon;
	tmplat = MixedFormatToDegrees(StrToDouble(params[1], NULL));
	tmplat = NorthOrSouth(tmplat, params[2][0]);
	tmplon = MixedFormatToDegrees(StrToDouble(params[3], NULL));
	tmplon = EastOrWest(tmplon,params[4][0]);
	if (!((tmplat == 0.0) && (tmplon == 0.0))) {
		pGPS->Latitude = tmplat;
		pGPS->Longitude = tmplon;
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

  if(RMZAvailable)
  {
	UpdateBaroSource(pGPS, isFlarm? BARO__RMZ_FLARM:BARO__RMZ, NULL, RMZAltitude);
  }

  // If  no gps fix, at this point we trigger refresh and quit
  if (!gpsValid) { 
	#if DEBUGSEQ
	StartupStore(_T("........ GGA no gps valid, triggerGPS!\n")); // 31C
	#endif
	TriggerGPSUpdate(); 
	return FALSE;
  }

  // "Altitude" should always be GPS Altitude.
  pGPS->Altitude = ParseAltitude(params[8], params[9]);
  pGPS->Altitude += (GPSAltitudeOffset/1000); // BUGFIX 100429
 
  if (_tcslen(params[10])>0) {
    // No real need to parse this value,
    // but we do assume that no correction is required in this case
//    double GeoidSeparation = ParseAltitude(params[10], params[11]);
  } else {
	if (UseGeoidSeparation) {
            double GeoidSeparation = LookupGeoidSeparation(pGPS->Latitude, pGPS->Longitude);
            pGPS->Altitude -= GeoidSeparation;
	}
  }

  // if RMC would be Triggering update, we loose the relative altitude, which is coming AFTER rmc! 
  // This was causing old altitude recorded in new pos fix.
  // 120428:
  // GGA will trigger gps if there is no RMC,  
  // or if GGAtime is the same as RMCtime, which means that RMC already came and we are last in the sequence
  if ( !RMCAvailable || (GGAtime == RMCtime)  )  {
	#if DEBUGSEQ
	StartupStore(_T("... GGA trigger gps, GGAtime==RMCtime\n")); // 31C
	#endif
	TriggerGPSUpdate(); 
  }
  return TRUE;

} // END GGA




// LK8000 IAS , in m/s*10  example: 346 for 34.6 m/s  which is = 124.56 km/h
BOOL NMEAParser::PLKAS(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *pGPS)
{
  (void)pGPS;

  if(nparams < 1) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid PLKAS sentence, nparams=%u%s"),(unsigned)nparams,NEWLINE));
    // max index used is 0...
    return FALSE;
  }
  
  double vias=StrToDouble(params[0],NULL)/10.0;
  if (vias >1) {
    pGPS->TrueAirspeed = vias*AirDensityRatio(pGPS->Altitude);
    pGPS->IndicatedAirspeed = vias;
  } else {
    pGPS->TrueAirspeed = 0;
    pGPS->IndicatedAirspeed = 0;
  }

  pGPS->AirspeedAvailable = TRUE;

  return FALSE;
}


BOOL NMEAParser::RMZ(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *pGPS)
{
  (void)pGPS;

  if(nparams < 2) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid RMZ sentence, nparams=%u%s"),(unsigned)nparams,NEWLINE));
    // max index used is 1...
    return FALSE;
  }
  
  // We want to wait for a couple of run so we are sure we are receiving RMC GGA etc.
  if (RMZDelayed--) {
	#if DEBUGBARO
	StartupStore(_T("...RMZ delayed, not processed (%d)\n"),RMZDelayed);
	#endif
	return FALSE;
  }
  RMZDelayed=0;

  RMZAltitude = ParseAltitude(params[0], params[1]);
  RMZAltitude = AltitudeToQNHAltitude(RMZAltitude);
  RMZAvailable = TRUE;
  LastRMZHB=LKHearthBeats; // this is common to both ports!

  // If we have a single RMZ with no gps fix data, we still manage the baro altitude.
  if (!RMCAvailable && !GGAAvailable) {
	UpdateBaroSource(pGPS, isFlarm? BARO__RMZ_FLARM:BARO__RMZ, NULL, RMZAltitude);
  }
  return FALSE;

}


// TASMAN instruments support for Tasman Flight Pack model Fp10
BOOL NMEAParser::PTAS1(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *pGPS)
{
  if(nparams < 4) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid PTAS1 sentence, nparams=%u%s"),(unsigned)nparams,NEWLINE));
    // max index used is 3...
    return FALSE;
  }
  
  double wnet,baralt,vtas;

  wnet = (StrToDouble(params[0],NULL)-200)/(10*TOKNOTS);
  baralt = (StrToDouble(params[2],NULL)-2000)/TOFEET;
  vtas = StrToDouble(params[3],NULL)/TOKNOTS;
  
  pGPS->AirspeedAvailable = TRUE;
  pGPS->TrueAirspeed = vtas;
  pGPS->VarioAvailable = TRUE;
  pGPS->Vario = wnet;
  UpdateBaroSource(pGPS, BARO__TASMAN, NULL,  AltitudeToQNHAltitude(baralt));
  pGPS->IndicatedAirspeed = vtas/AirDensityRatio(baralt);
 
  TASAvailable = true; // 100411 
  TriggerVarioUpdate();

  return FALSE;
}


BOOL NMEAParser::HCHDG(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *pGPS)
{
  if(nparams < 1) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid HCHDG sentence, nparams=%u%s"),(unsigned)nparams,NEWLINE));
    // max index used is 0...
    return FALSE;
  }
  
  (void)pGPS;
  double mag=0;
  mag=StrToDouble(params[0],NULL);
  if (mag>=0 && mag<=360) {
      pGPS->MagneticHeading=mag;
      pGPS->MagneticHeadingAvailable=TRUE;
  }
  return FALSE;
}


#ifdef DSX
// warning, TODO FIX, calling AddMessage from wrong thread? CHECK
BOOL NMEAParser::PDSXT(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *pGPS)
{
  TCHAR mbuf[300];

  if ( _tcslen(params[0]) >0) 
	_stprintf(mbuf,_T("MESSAGE FROM <%s>: %s"), params[0], params[1]);
  else
	_stprintf(mbuf,_T("MESSAGE: %s"),params[1] );
  Message::AddMessage(30000, 3, mbuf);
  LKSound(TEXT("LK_TONEUP.WAV"));

  return TRUE;


}
#endif


