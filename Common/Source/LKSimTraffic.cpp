/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"
#include "FlarmCalculations.h"

extern void CheckBackTarget(NMEA_INFO *pGPS, int flarmslot);
extern FlarmCalculations flarmCalculations;

// #define DEBUG_SIMLKT


double SimNewCoordinate(const double coordinate,double offset) {
  return (coordinate + ((double)(GPS_INFO.Second+offset)/1000.0)*(rand()>(RAND_MAX/2)?1:-1));
}

double SimNewAltitude(const double altitude) {
  return (altitude + (rand() % 327) + (rand() % 163) );
}

double SimNewSpeed(const double speed) {

  return (35 + (rand() % 3));
}

//
// The purpose of simulating flarm traffic is NOT to play a videogame against flarm objects:
// we only need some traffic to display for testing on ground during simulations.
//
// >>>>> This is accessing directly the GPS_INFO main struct, writing inside it. <<<<<
// Called by LKSimulator, already locking FlightData . No worry.
//
void SimFlarmTraffic(uint32_t RadioId, double offset)
{
  int flarm_slot = 0;
  bool newtraffic=false;

  GPS_INFO.FLARM_Available=true;
  LastFlarmCommandTime=GPS_INFO.Time; // useless really, we dont call UpdateMonitor from SIM
  

  flarm_slot = FLARM_FindSlot(&GPS_INFO, RadioId);

  if (flarm_slot<0) return;
  if ( GPS_INFO.FLARM_Traffic[flarm_slot].Status == LKT_EMPTY) {
	newtraffic=true;
  }

  // before changing timefix, see if it was an old target back locked in!
  CheckBackTarget(&GPS_INFO, flarm_slot);
  // and then set time of fix to current time
  GPS_INFO.FLARM_Traffic[flarm_slot].Time_Fix = GPS_INFO.Time;

/*
	  TEXT("%hu,%lf,%lf,%lf,%hu,%lx,%lf,%lf,%lf,%lf,%hu"),
	  &GPS_INFO.FLARM_Traffic[flarm_slot].AlarmLevel, // unsigned short 0
	  &GPS_INFO.FLARM_Traffic[flarm_slot].RelativeNorth, //  1	
	  &GPS_INFO.FLARM_Traffic[flarm_slot].RelativeEast, //   2
	  &GPS_INFO.FLARM_Traffic[flarm_slot].RelativeAltitude, //  3
	  &GPS_INFO.FLARM_Traffic[flarm_slot].IDType, // unsigned short     4
	  &GPS_INFO.FLARM_Traffic[flarm_slot].ID, // 6 char hex
	  &GPS_INFO.FLARM_Traffic[flarm_slot].TrackBearing, // double       6
	  &GPS_INFO.FLARM_Traffic[flarm_slot].TurnRate, // double           7
	  &GPS_INFO.FLARM_Traffic[flarm_slot].Speed, // double              8 m/s
	  &GPS_INFO.FLARM_Traffic[flarm_slot].ClimbRate, // double          9 m/s
	  &GPS_INFO.FLARM_Traffic[flarm_slot].Type); // unsigned short     10
*/

  // If first time seen this traffic, place it nearby
  if ( newtraffic ) {
	GPS_INFO.FLARM_Traffic[flarm_slot].Latitude  = SimNewCoordinate(GPS_INFO.Latitude, offset);
	GPS_INFO.FLARM_Traffic[flarm_slot].Longitude = SimNewCoordinate(GPS_INFO.Longitude,offset);
	GPS_INFO.FLARM_Traffic[flarm_slot].Altitude = SimNewAltitude(GPS_INFO.Altitude);

	GPS_INFO.FLARM_Traffic[flarm_slot].TrackBearing= (double) (rand()%358);
	GPS_INFO.FLARM_Traffic[flarm_slot].AlarmLevel=0;
	GPS_INFO.FLARM_Traffic[flarm_slot].RadioId = RadioId;
	GPS_INFO.FLARM_Traffic[flarm_slot].TurnRate=0;
	GPS_INFO.FLARM_Traffic[flarm_slot].Speed= SimNewSpeed(GPS_INFO.Speed);

	GPS_INFO.FLARM_Traffic[flarm_slot].Status = LKT_REAL;
  } else {
	GPS_INFO.FLARM_Traffic[flarm_slot].Latitude  += (double)((rand()%16384)/10000000.0)*(rand()>(RAND_MAX/2)?1:-1);
	GPS_INFO.FLARM_Traffic[flarm_slot].Longitude += (double)((rand()%16384)/10000000.0)*(rand()>(RAND_MAX/2)?1:-1);
	GPS_INFO.FLARM_Traffic[flarm_slot].Altitude += (double)(rand()%14)*(rand()>(RAND_MAX/2)?1:-1);
  }

  //
  GPS_INFO.FLARM_Traffic[flarm_slot].Average30s = flarmCalculations.Average30s(
	  GPS_INFO.FLARM_Traffic[flarm_slot].RadioId,
	  GPS_INFO.Time,
	  GPS_INFO.FLARM_Traffic[flarm_slot].Altitude);

  TCHAR *name = GPS_INFO.FLARM_Traffic[flarm_slot].Name;
  //TCHAR *cn = GPS_INFO.FLARM_Traffic[flarm_slot].Cn;
  // If there is no name yet, or if we have a pending update event..
  if (!_tcslen(name) || GPS_INFO.FLARM_Traffic[flarm_slot].UpdateNameFlag ) {

	#ifdef DEBUG_SIMLKT
	if (GPS_INFO.FLARM_Traffic[flarm_slot].UpdateNameFlag ) {
		StartupStore(_T("... UpdateNameFlag for slot %d\n"),flarm_slot);
	} else {
		StartupStore(_T("... First lookup name for slot %d\n"),flarm_slot);
	}
	#endif

	GPS_INFO.FLARM_Traffic[flarm_slot].UpdateNameFlag=false; // clear flag first
	TCHAR *fname = LookupFLARMDetails(GPS_INFO.FLARM_Traffic[flarm_slot].RadioId);
	if (fname) {
		LK_tcsncpy(name,fname,MAXFLARMNAME);

		//  Now we have the name, so lookup also for the Cn
		// This will return either real Cn or Name, again
		TCHAR *cname = LookupFLARMCn(GPS_INFO.FLARM_Traffic[flarm_slot].RadioId);
		if (cname) {
			int cnamelen=_tcslen(cname);
			if (cnamelen<=MAXFLARMCN) {
				_tcscpy( GPS_INFO.FLARM_Traffic[flarm_slot].Cn, cname);
			} else {
				// else probably it is the Name again, and we create a fake Cn
				GPS_INFO.FLARM_Traffic[flarm_slot].Cn[0]=cname[0];
				GPS_INFO.FLARM_Traffic[flarm_slot].Cn[1]=cname[cnamelen-2];
				GPS_INFO.FLARM_Traffic[flarm_slot].Cn[2]=cname[cnamelen-1];
				GPS_INFO.FLARM_Traffic[flarm_slot].Cn[3]=_T('\0');
			}
		} else {
			_tcscpy( GPS_INFO.FLARM_Traffic[flarm_slot].Cn, _T("Err"));
		}

		#ifdef DEBUG_SIMLKT
		StartupStore(_T("... PFLAA Name to FlarmSlot=%d ID=%lx Name=<%s> Cn=<%s>\n"),
			flarm_slot,
	  		GPS_INFO.FLARM_Traffic[flarm_slot].ID,
			GPS_INFO.FLARM_Traffic[flarm_slot].Name,
			GPS_INFO.FLARM_Traffic[flarm_slot].Cn);
		#endif
	} else {
		// Else we NEED to set a name, otherwise it will constantly search for it over and over..
		name[0]=_T('?');
		name[1]=_T('\0');
		GPS_INFO.FLARM_Traffic[flarm_slot].Cn[0]=_T('?');
		GPS_INFO.FLARM_Traffic[flarm_slot].Cn[1]=_T('\0');
		
		#ifdef DEBUG_SIMLKT
		StartupStore(_T("... New FlarmSlot=%d ID=%lx with no name, assigned a \"?\"\n"),
			flarm_slot,
	  		GPS_INFO.FLARM_Traffic[flarm_slot].ID);
		#endif
	}
  }
}

