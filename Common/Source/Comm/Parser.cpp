/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Parser.cpp,v 8.12 2010/12/12 16:14:28 root Exp root $

*/

#include "externs.h"
#include "Baro.h"
#include "Calc/Vario.h"
#include "Logger.h"
#include "Geoid.h"
#include "GpsWeekNumberFix.h"

#if defined(PNA) && defined(UNDER_CE)
#include "Devices/LKHolux.h"
#include "Devices/LKRoyaltek3200.h"
#endif

using std::string_view_literals::operator""sv;

extern double EastOrWest(double in, TCHAR EoW);
extern double NorthOrSouth(double in, TCHAR NoS);
extern double MixedFormatToDegrees(double mixed);

namespace {

bool NAVWarn(char c) {
  return c != 'A';
}

// minimal speed to use gps bearing
double GetTrackBearingMinSpeed() {
  if (ISCAR) {
    return 0;  // trekking mode/car mode, min speed > 0
  }
  else {
    return 1; // flymode,  min speed >1 knot
  }
}

} // namespace

int NMEAParser::StartDay = -1;


// #define DEBUGSEQ	1
// #define DEBUGBARO	1

NMEAParser::NMEAParser() {
  activeGPS = false;
  Reset();
}

void NMEAParser::Reset() {
  connected = false;
  nSatellites = 0;
  gpsValid = false;
  dateValid = false;
  isFlarm = false;
  GGAAvailable = false;
  RMZAvailable = false;
  LastRMZHB = false;
  RMCAvailable = false;
  TASAvailable = false; // 100411
  RMZDelayed = 3; // wait for this to be zero before using RMZ.
 
  GGAtime=0;
  RMCtime=0;
  GLLtime=0; 
  LastTime = 0;
}

#if defined(PNA) && defined(UNDER_CE)
BOOL NMEAParser::ParseGPS_POSITION(int Idx, const GPS_POSITION& loc, NMEA_INFO& GPSData) {
    LKASSERT(!ReplayLogger::IsEnabled());

    DeviceDescriptor_t* pdev = devGetDeviceOnPort(Idx);
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
    connected = true;

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

    if (gpsValid) {
        lastGpsValid.Update();
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
        GPSData.Speed = Units::From(unKnots, loc.flSpeed);
    }
    if (loc.dwValidFields & GPS_VALID_HEADING) {
      	if (GPSData.Speed > GetTrackBearingMinSpeed()) {
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

BOOL NMEAParser::ParseNMEAString_Internal(DeviceDescriptor_t& d, const char* String, NMEA_INFO* pGPS) {
  if (!String) {
    return FALSE;
  }

  auto wait_ack = d.lock_wait_ack();
  if (wait_ack && wait_ack->check(String)) {
    return TRUE;
  }

  if (String[0] !='$') {
    return FALSE;
  }

  char ctemp[MAX_NMEA_LEN];
  char* params[MAX_NMEA_PARAMS];

  size_t n_params = ValidateAndExtract(String, ctemp, params);
  if (n_params < 1 || params[0][0] != '$') {
    return FALSE;
  }

  if (params[0][1] == 'P') {
    std::string_view token = params[0] + 2;
    // Proprietary String
    if (token == "TAS1"sv) {
      return PTAS1(d, &String[7], params + 1, n_params - 1, pGPS);
    }
    if (token == "FLAV"sv) {
      return PFLAV(&String[7], params + 1, n_params - 1, pGPS);
    }
    if (token == "FLAA"sv) {
      return PFLAA(&String[7], params + 1, n_params - 1, pGPS);
    }
    if (token == "FLAU"sv) {
      return PFLAU(&String[7], params + 1, n_params - 1, pGPS);
    }
    if (token == "GRMZ"sv) {
      return RMZ(d, &String[7], params + 1, n_params - 1, pGPS);
    }
    if (token == "LKAS"sv) {
      return PLKAS(&String[7], params + 1, n_params - 1, pGPS);
    }
    return FALSE;
  }

  if (params[0][1] == 'G') {
    // GNSS String
    std::string_view token = params[0] + 3;
    if (token == "GSA"sv) {
      return GSA(&String[7], params + 1, n_params - 1, pGPS);
    }
    if (token == "GLL"sv) {
      /*
      return GLL(&String[7], params + 1, n_params-1, pGPS);
      */
      return FALSE;
    }
    if (token == "RMB"sv) {
      /*
      return RMB(&String[7], params + 1, n_params-1, pGPS);
      */
      return FALSE;
    }
    if (token == "RMC"sv) {
      return RMC(&String[7], params + 1, n_params - 1, pGPS);
    }
    if (token == "GGA"sv) {
      return GGA(&String[7], params + 1, n_params - 1, pGPS);
    }
    if (token == "VTG"sv) {
      return VTG(&String[7], params + 1, n_params - 1, pGPS);
    }
  }

  if (std::string_view(params[0] + 1) == "HCHDG"sv) {
    return HCHDG(d, &String[7], params + 1, n_params - 1, pGPS);
  }

  return FALSE;
}

void NMEAParser::CheckRMZ() {
  if ((LastRMZHB > 0) && LKHearthBeats > (LastRMZHB+5)) {
    RMZAvailable = false;
  }
}

namespace {
//
// Make time absolute, over 86400seconds when day is changing
// We need a valid date to use it. We are relying on StartDay.
//
template<typename CharT>
double TimeModify(const CharT* StrTime, NMEA_INFO* pGPS, int& StartDay) {
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

}

double TimeModify(const char* FixTime, NMEA_INFO* info, int& StartDay) {
  return TimeModify<char>(FixTime, info, StartDay);
}

double TimeModify(const wchar_t* FixTime, NMEA_INFO* info, int& StartDay) {
  return TimeModify<wchar_t>(FixTime, info, StartDay);
}

double TimeModify(NMEA_INFO* pGPS, int& StartDay) {
  static int day_difference = 0, previous_months_day_difference = 0;

  double FixTime = (double) (pGPS->Second + (pGPS->Minute * 60) + (pGPS->Hour * 3600));

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
    TestLog(_T("... TimeHasAdvanced BACK in time: Last=%f This=%f   %s"), LastTime, ThisTime, WhatTimeIsIt());
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

BOOL NMEAParser::GSA(const char* String, char** params, size_t nparams, NMEA_INFO *pGPS)
{
  return TRUE;
}

// we need to parse GLL as well because it can mark the start of a new quantum data
// followed by values with no data, ex. altitude, vario, etc.
BOOL NMEAParser::GLL(const char* String, char** params, size_t nparams, NMEA_INFO *pGPS)
{
  if(nparams < 6) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid GLL sentence, nparams=%u%s"),(unsigned)nparams,NEWLINE));
    // max index used is 5...
    return FALSE;
  }
  
  gpsValid = !NAVWarn(params[5][0]);
  connected = true;

  if (gpsValid) {
    lastGpsValid.Update();
  }

  if (!activeGPS) return TRUE;

  ScopeLock lock(CritSec_FlightData);

  pGPS->NAVWarning = !gpsValid;
  
  // use valid time with invalid fix
  GLLtime = StrToDouble(params[4],NULL);
  if (!RMCAvailable &&  !GGAAvailable && (GLLtime>0)) {
    double ThisTime = TimeModify(params[4], pGPS, StartDay);
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



BOOL NMEAParser::RMB(const char* String, char** params, size_t nparams, NMEA_INFO *pGPS)
{
  return TRUE;
} // END RMB



BOOL NMEAParser::VTG(const char* String, char** params, size_t nparams, NMEA_INFO *pGPS)
{
  if(nparams < 5) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid VTG sentence, nparams=%u%s"),(unsigned)nparams,NEWLINE));
    // max index used is 4...
    return FALSE;
  }
  
  // GPSCONNECT = TRUE; // 121127  NO! VTG gives no position fix
  if (!activeGPS) return TRUE;

  if (RMCAvailable) return FALSE;

  // if no valid fix, we dont get speed either!
  if (gpsValid) {
    double speed = StrToDouble(params[4], NULL);

    ScopeLock lock(CritSec_FlightData);

    pGPS->Speed = Units::From(unKnots, speed);

    if (ISCAR) {
      if (pGPS->Speed > GetTrackBearingMinSpeed()) {
        pGPS->TrackBearing = AngleLimit360(StrToDouble(params[0], NULL));
      }
    }
  }

  // if we are here, no RMC is available but if no GGA also, we are in troubles: to check!
  if (!GGAAvailable) {
	TriggerGPSUpdate();
  }

  return TRUE;

} // END VTG

BOOL NMEAParser::RMC(const char* String, char** params, size_t nparams, NMEA_INFO *pGPS)
{
  if(nparams < 9) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid RMC sentence, nparams=%u%s"),(unsigned)nparams,NEWLINE));
    // max index used is 8...
    return FALSE;
  }

  double speed=0;

  gpsValid = !NAVWarn(params[1][0]);
  if (gpsValid) {
    lastGpsValid.Update();
  }

  connected = true;
  RMCAvailable=true; // 100409

  #ifdef PNA

  if (DeviceIsGM130) {
    double ps = GM130BarPressure();
    double Altitude = (1 - pow(fabs(ps / QNH),  0.190284)) * 44307.69;

    ScopeLock lock(CritSec_FlightData);
    UpdateBaroSource(pGPS, nullptr, Altitude);
  }

  if (DeviceIsRoyaltek3200) {
    if (Royaltek3200_ReadBarData()) {
      double ps = Royaltek3200_GetPressure();
      double Altitude = (1 - pow(fabs(ps / QNH),  0.190284)) * 44307.69;

      ScopeLock lock(CritSec_FlightData);
      UpdateBaroSource(pGPS, nullptr,  Altitude);
    }
  }

  #endif // PNA

  if (!activeGPS) {
    return TRUE;
  }

  // if no valid fix, we dont get speed either!
  if (gpsValid) {
    // speed is in knots, 2 = 3.7kmh
    speed = StrToDouble(params[6], NULL);
  }
  
  ScopeLock lock(CritSec_FlightData);

  pGPS->NAVWarning = !gpsValid;

  // say we are updated every time we get this,
  // so infoboxes get refreshed if GPS connected
  // the RMC sentence marks the start of a new fix, so we force the old data to be saved for calculations

	if(!gpsValid && !dateValid) {
		// we have valid date with invalid fix only if we have already got valid fix ...
		return TRUE;
	}

	// note that Condor sends no date..
	const size_t size_date = strlen(params[8]);
	if (size_date < 6 && !DevIsCondor) {
		TestLog(_T(".... RMC date field empty, skip sentence!"));
		return TRUE;
	}

	// Even with no valid position, we let RMC set the time and date if valid
	int year, month, day;
	if (parse_rmc_date(params[8], size_date, year, month, day)) {
		pGPS->Year = year;
		pGPS->Month = month;
		pGPS->Day = day;
	} else {
		//.. Condor not sending valid date;
		if (!DevIsCondor) {
			static bool logbaddate = true;
			if (gpsValid && logbaddate) { // 091115
				StartupStore(_T("------ NMEAParser:RMC Receiving an invalid or null DATE from GPS"));
				StartupStore(_T("------ NMEAParser: Date received is \"%04d-%02d-%02d\""), year, month, day); // 100422
				StartupStore(_T("------ This message will NOT be repeated. %s"), WhatTimeIsIt());
				// _@M875_ "WARNING: GPS IS SENDING INVALID DATE, AND PROBABLY WRONG TIME"
				DoStatusMessage(MsgToken<875>());
				logbaddate = false;
			}
		}
	}

	dateValid = true;

	RMCtime = StrToDouble(params[0],NULL);
	double ThisTime = TimeModify(params[0], pGPS, StartDay);
	// RMC time has priority on GGA and GLL etc. so if we have it we use it at once
	if (!TimeHasAdvanced(ThisTime, pGPS)) {
#if DEBUGSEQ
		StartupStore(_T("..... RMC time not advanced, skipping \n")); // 31C
#endif
		return FALSE;
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
  
	pGPS->Speed = Units::From(unKnots, speed);;
  
	if (pGPS->Speed > GetTrackBearingMinSpeed()) {
		pGPS->TrackBearing = AngleLimit360(StrToDouble(params[7], NULL));
	}
  } // gpsvalid 091108

#if defined(PPC2003) || defined(PNA)
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
#endif

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




BOOL NMEAParser::GGA(const char* String, char** params, size_t nparams, NMEA_INFO *pGPS)
{
  if(nparams < 11) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid GGA sentence, nparams=%u <%s>"),(unsigned)nparams,String));
    // max index used is 10...
    return FALSE;
  }

  GGAAvailable = TRUE;
  connected = true;     // 091208

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

  unsigned ggafix = strtoul(params[5], nullptr, 10);
  gpsValid = (ggafix > 0 && ggafix < 6);
  if (ggafix == 6 ) {
    DebugLog(_T("------ GGA DEAD RECKON fix skipped"));
  }
#ifdef YDEBUG
  // in debug we need to accept manual or simulated fix
  gpsValid = gpsValid || (ggafix == 7 || ggafix == 8); 
#endif

  if (gpsValid) {
    lastGpsValid.Update();
  }

  if (!activeGPS) {
    return TRUE;
  }

  // some device don't send sat in use count, set it to "-1" if fix is valid and sat in use is 0
  if(gpsValid && (nSatellites == 0)) {
    nSatellites = -1; // unknown count but valid fix !
  }

  ScopeLock lock(CritSec_FlightData);

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
	double ThisTime = TimeModify(params[0], pGPS, StartDay);
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
		DebugLog(_T("++++++ GGA gpsValid with invalid posfix!"));
		gpsValid=false;
	}
  }

  // any NMEA sentence with time can now trigger gps update: the first to detect new time will make trigger.
  // we assume also that any sentence with no time belongs to current time.
  // note that if no time from gps, no use of vario and baro data, but also no fix available.. so no problems

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
 
  if ((*params[10]) && (params[10] != "0"sv)) {
    // No real need to parse this value,
    // but we do assume that no correction is required in this case
    // double GeoidSeparation = ParseAltitude(params[10], params[11]);
  } else if (UseGeoidSeparation) {
      pGPS->Altitude -= LookupGeoidSeparation(pGPS->Latitude, pGPS->Longitude);
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
BOOL NMEAParser::PLKAS(const char* String, char** params, size_t nparams, NMEA_INFO *pGPS)
{
  if(nparams < 1) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid PLKAS sentence, nparams=%u%s"),(unsigned)nparams,NEWLINE));
    // max index used is 0...
    return FALSE;
  }
  
  double vias=StrToDouble(params[0],NULL)/10.0;

  ScopeLock lock(CritSec_FlightData);
  if (vias > 1) {
    double qne_altitude = QNHAltitudeToQNEAltitude(pGPS->Altitude);
    pGPS->TrueAirspeed = TrueAirSpeed(vias, qne_altitude);
    pGPS->IndicatedAirspeed = vias;
  } else {
    pGPS->TrueAirspeed = 0;
    pGPS->IndicatedAirspeed = 0;
  }

  pGPS->AirspeedAvailable = TRUE;

  return FALSE;
}


BOOL NMEAParser::RMZ(DeviceDescriptor_t& d, const char* String, char **params, size_t nparams, NMEA_INFO *pGPS)
{
  if(nparams < 2) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid RMZ sentence, nparams=%u%s"),(unsigned)nparams,NEWLINE));
    // max index used is 1...
    return FALSE;
  }
  
  // We want to wait for a couple of run so we are sure we are receiving RMC GGA etc.
  if (RMZDelayed--) {
    return FALSE;
  }
  RMZDelayed=0;

  RMZAvailable = true;
  LastRMZHB = LKHearthBeats;

  double Altitude = ParseAltitude(params[0], params[1]);

  ScopeLock lock(CritSec_FlightData);
  UpdateBaroSource(pGPS, &d, QNEAltitudeToQNHAltitude(Altitude));

  return FALSE;
}


// TASMAN instruments support for Tasman Flight Pack model Fp10
BOOL NMEAParser::PTAS1(DeviceDescriptor_t& d, const char* String, char **params, size_t nparams, NMEA_INFO *pGPS) 
{
  if(nparams < 4) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid PTAS1 sentence, nparams=%u%s"),(unsigned)nparams,NEWLINE));
    // max index used is 3...
    return FALSE;
  }

  ScopeLock lock(CritSec_FlightData);

  if(*params[0] != _T('\0')) {
    const double wnet = Units::From(unKnots, (StrToDouble(params[0],NULL) - 200.0) / 10);
    UpdateVarioSource(*pGPS, d, wnet);
  }

  if(*params[2] != _T('\0')) {
    double qne_altitude = Units::From(unFeet, StrToDouble(params[2],NULL) - 2000);
    UpdateBaroSource(pGPS, &d,  QNEAltitudeToQNHAltitude(qne_altitude));

    if(*params[3] != _T('\0')) {
      const double vtas = Units::From(unKnots, StrToDouble(params[3],NULL));
      pGPS->AirspeedAvailable = TRUE;
      pGPS->TrueAirspeed = vtas;
      pGPS->IndicatedAirspeed = IndicatedAirSpeed(vtas, qne_altitude);
    }

    TASAvailable = true; // 100411
  }

  return FALSE;
}


BOOL NMEAParser::HCHDG(DeviceDescriptor_t& d, const char* String, char** params, size_t nparams, NMEA_INFO *pGPS)
{
  if(nparams < 1) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid HCHDG sentence, nparams=%u%s"),(unsigned)nparams,NEWLINE));
    // max index used is 0...
    return FALSE;
  }
  
  double mag = StrToDouble(params[0],NULL);
  if (mag>=0 && mag<=360) {
      ScopeLock lock(CritSec_FlightData);
      pGPS->MagneticHeading.update(d, mag);
  }
  return TRUE;
}


