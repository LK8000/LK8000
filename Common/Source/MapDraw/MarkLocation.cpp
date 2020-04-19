/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKProcess.h"
#include "Waypointparser.h"
#include "utils/stringext.h"


extern int GetVirtualWaypointMarkerSlot(void);


void MarkLocation(const double lon, const double lat, const double altitude) {
  char message[210];

  FILE *stream;
  TCHAR tstring[50];
  bool dopreambol=false;
  TCHAR fname[MAX_PATH];
  LocalPath(fname,TEXT(LKD_WAYPOINTS));
  _tcscat(fname,_T(DIRSEP));
  _stprintf(tstring,_T("LK%04d%02d%02d.cup"), GPS_INFO.Year,GPS_INFO.Month,GPS_INFO.Day);
  _tcscat(fname,tstring);

  stream = _tfopen(fname,TEXT("r"));
  if (stream == NULL)
	dopreambol=true;
  else
	fclose(stream);

  stream = _tfopen(fname,TEXT("a+"));
  if (stream != NULL){
	if (dopreambol) {
		// file was created empty, we need to add preambol header for CUP
		strcpy(message,"name,code,country,lat,lon,elev,style,rwdir,rwlen,freq,desc\r\n");
		fwrite(message,strlen(message),1,stream);
	}

	char marktime[10], slat[20], slon[20], snear[50];
	Units::TimeToTextSimple(tstring,LocalTime());
	TCHAR2ascii(tstring,marktime,10);

	LatitudeToCUPString(lat,tstring);
	TCHAR2ascii(tstring,slat,20);
	LongitudeToCUPString(lon,tstring);
	TCHAR2ascii(tstring,slon,20);

	int j=FindNearestFarVisibleWayPoint(lon,lat,15000,WPT_UNKNOWN);
	if (j>0) {
        _tcscpy(tstring,WayPointList[j].Name); // Name is sized NAME_SIZE, 30, so ok with tstring[50]
        tstring[19]='\0'; // sized 20 chars
		TCHAR2ascii(tstring,snear,50);
	} else {
		strcpy(snear,"unknown");
	}

	sprintf(message,"MK%s%02d,LK8000,,%s,%s,%d.0m,1,,,,Created on %02d-%02d-%04d at h%s near: %s\r\n",
		marktime,GPS_INFO.Second,slat,slon, iround((int)altitude),
		GPS_INFO.Day,GPS_INFO.Month,GPS_INFO.Year, marktime, snear );

	fwrite(message,strlen(message),1,stream);
	fclose(stream);



	j=GetVirtualWaypointMarkerSlot();

	WayPointCalc[j].WpType=WPT_TURNPOINT;
	WayPointList[j].Latitude=lat;
	WayPointList[j].Longitude=lon;
	WayPointList[j].Altitude=altitude;
	WayPointList[j].Visible=TRUE;
	WayPointList[j].FarVisible=TRUE;

    utf2TCHAR(marktime, tstring,50);
	_stprintf(WayPointList[j].Name,_T("MK%s%02d"),tstring,GPS_INFO.Second);
	utf2TCHAR(snear, tstring, 50);
	TCHAR comment[60];
	_stprintf(comment, _T("Near: %s"), tstring);
	SetWaypointComment(WayPointList[j], comment);


	// Force updating DoRange otherwise it will pass up to 3 minutes
	// before this marker appears in the 2.3 tps page
	LastDoRangeWaypointListTime=0;
  }

}
