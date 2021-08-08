/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
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

	if(Freq >= 118.0 && Freq <= 137.0) {
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
	}

	return Valid;
}

double  ExtractFrequencyPos(const TCHAR *text, size_t *start, size_t *len)
{
	if(start) *start =0;
	if(len)*len =0;
	if(text == nullptr)
		return 0.0;

	double fFreq = 0.0;
	size_t iTxtlen = _tcslen(text);
	
	if(iTxtlen < 3)
		return 0.0;
	
	for (size_t i = 0; (i + 4) < iTxtlen; i++)
	{
		if (text[i] == '1')
		{
			int Mhz  = _tcstol(&text[i], nullptr, 10);
			if(Mhz >= 118 && Mhz <= 138) 
			{
				if((text[i+3] == '.') || (text[i+3] == ','))
				{
					int kHz = _tcstol(&text[i+4], nullptr, 10);


					fFreq = (double)Mhz + (double)kHz / 1000.0;
					if(start) *start = i;
					if(len) 
					{
						*len = iTxtlen-i;
						if(*len >7) *len = 7;
					}
					if(ValidFrequency(fFreq)) return fFreq;
				}
			}
		}
	}

	return fFreq;
}

double  ExtractFrequency(const TCHAR *text)
{ size_t pos, len;
	return  ExtractFrequencyPos(text, &pos,&len);
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


const TCHAR* GetActiveStationSymbol(bool utf8_symbol) 
{
	if(utf8_symbol)
		return(ACTIVE_SYMBOL_UTF8) ;
	else
		return(ACTIVE_SYMBOL);
}

const TCHAR* GetStandyStationSymbol(bool utf8_symbol) 
{
	if(utf8_symbol)
		return(STANDBY_SYMBOL_UTF8) ;
	else
		return(STANDBY_SYMBOL);
}
//#define SEL_STANDBY_SYMBOL(a) (a)?(STANDBY_SYMBOL_UTF8 ) :(STANDBY_SYMBOL)