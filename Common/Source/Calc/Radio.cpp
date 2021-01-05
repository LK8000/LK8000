/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Radio.h,v 1.1 2020/22/12 
*/

#include "externs.h"
#include "Radio.h"
#include "Util/TruncateString.hpp"


BOOL ValidFrequency(double Freq)
{
BOOL Valid =FALSE;
int Frac = 	(int)(Freq*1000.0+0.05) - 100*((int) (Freq *10.0+0.05));

  if(Freq >= 118.0)
    if(Freq <= 137.0)
      switch(Frac)
      {
        case 0:
        case 25:
        case 50:
        case 75:
          Valid = TRUE;
        break;

        case 5:
        case 10:
        case 15:
        case 30:
        case 35:
        case 40:
        case 55:
        case 60:
        case 65:
        case 80:
        case 85:
        case 90:
          if(RadioPara.Enabled8_33)
            Valid = TRUE;
        break;

        default:
        break;
      }


  return Valid;
}

double  ExtractFrequency(const TCHAR *text)
{
	if(text == NULL)
		return 0.0;
double fFreq = 0.0;
int iTxtlen = (int)_tcslen(text);
int i,Mhz=0,kHz=0;

	for (i=0; i < iTxtlen; i++)
	{
		if (text[i] == '1')
		{
			Mhz  = _tcstol(&text[i], nullptr, 10);
			if(Mhz >= 118)
				if(Mhz <= 138)
					if((i+3) < iTxtlen)
					{
						if((text[i+3] == '.') || (text[i+3] == ','))
						{
							kHz =0;
							if((i+4) < iTxtlen)
								if((text[i+4] >= '0') && (text[i+4] <= '9'))
									kHz += (text[i+4]-'0') * 100;

							if((i+5) < iTxtlen)
								if((text[i+5] >= '0') && (text[i+5] <= '9'))
									kHz += (text[i+5]-'0') * 10;

							if((i+6) < iTxtlen)
								if((text[i+6] >= '0') && (text[i+6] <= '9'))
									kHz += (text[i+6]-'0');

							fFreq = (double) Mhz+ (double)kHz/1000.0f;
							return fFreq;
						}
					}
		}
	}

 return fFreq;
}

/**
 * Distance in ° between two geographical position
 * 
 * using it to compare tp distance is an aproximation and  is valid 
 *   only to fast solve conflict between to TP with same frequency 
 * 
 * if needed to improve precision dlat calculation can be replaced by
 *   double dlat =  cos(a.latitude * PI/180) * (a.latitude - b.latitude)
 */
static double LatLonDistance(GeoPoint a, GeoPoint b)
{
	double dla = a.latitude - b.latitude;
	double dlo = a.longitude - b.longitude;
	return sqrt(dla*dla + dlo*dlo);
}


bool UpdateStationName(TCHAR (&Name)[NAME_SIZE + 1], double Frequency) {

	double minDist = 9999999;
	int idx = -1;
	
	if(!ValidFrequency(Frequency))
		return 0;

	LockFlightData();
	GeoPoint cur_pos(GPS_INFO.Latitude, GPS_INFO.Longitude);
	UnlockFlightData();

	LockTaskData();
	for (size_t i = NUMRESWP; i < WayPointList.size(); ++i)
	{
		const WAYPOINT& wpt = WayPointList[i];

		assert(wpt.Latitude != RESWP_INVALIDNUMBER);

		if(wpt.Freq[0]) { // ignore TP with empty frequency
			double fWpFreq = StrToDouble(wpt.Freq, nullptr);
			if(fabs(Frequency - fWpFreq ) < 0.001)
			{
				double fDist = LatLonDistance(cur_pos, GeoPoint(wpt.Latitude, wpt.Longitude));
				if(fDist < minDist)
				{
					minDist = fDist;
					idx =i;
				}
			}
		}
	}

	_tcscpy(Name, (idx >= 0) ? WayPointList[idx].Name : _T(""));

	UnlockTaskData();		

	return (idx >= 0);
}


static int SearchNearestStation()
{
	double minDist = 9999999;
	int minIdx = -1;
	
	LockFlightData();
	GeoPoint cur_pos(GPS_INFO.Latitude, GPS_INFO.Longitude);
	UnlockFlightData();

	LockTaskData();			
	for (size_t i = NUMRESWP; i < WayPointList.size(); ++i)
	{
		const WAYPOINT& wpt = WayPointList[i];

		assert(wpt.Latitude != RESWP_INVALIDNUMBER);

		if(wpt.Freq[0]) { // ignore TP with empty frequency
			double fWpFreq = StrToDouble(wpt.Freq, nullptr);
			if(ValidFrequency( fWpFreq))
			{					
				double fDist = LatLonDistance(cur_pos, GeoPoint(wpt.Latitude, wpt.Longitude));
				if(fDist < minDist)
				{
					minDist = fDist;
					minIdx =i;
				}
			}
		}
	}
	UnlockTaskData();	

	return minIdx;
}


int SearchBestStation()
{
	int Idx = BestAlternate;    // begin with Best alternate
	double fFreq=0.0;

	LockTaskData();
	if(ValidWayPointFast(Idx)) 
	{
		fFreq = StrToDouble(WayPointList[Idx].Freq,NULL);
	}
	UnlockTaskData();

	if(!ValidFrequency(fFreq))	// best alternate does not have a radio?
	{
		Idx = SearchNearestStation(); // OK, then search for the nearest with radio!
	}
	return Idx;
}
