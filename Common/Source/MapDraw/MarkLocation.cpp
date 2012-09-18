/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Process.h"
#include "Waypointparser.h"


extern void LatitudeToCUPString(double Latitude, TCHAR *Buffer);
extern void LongitudeToCUPString(double Latitude, TCHAR *Buffer);
extern void unicodetoascii(TCHAR *text, int tsize, char *atext);


#if USETOPOMARKS
void MarkLocation(const double lon, const double lat)
#else
void MarkLocation(const double lon, const double lat, const double altitude)
#endif
{
  #if USETOPOMARKS
  LockTerrainDataGraphics();
  if (topo_marks) {
    topo_marks->addPoint(lon, lat);
    topo_marks->triggerUpdateCache = true;
  }
  UnlockTerrainDataGraphics();
  #endif

  char message[160];

  FILE *stream;
  TCHAR tstring[50];
  bool dopreambol=false;
  TCHAR fname[MAX_PATH];
  LocalPath(fname,TEXT(LKD_WAYPOINTS));
  _tcscat(fname,_T("\\")); 
  wsprintf(tstring,_T("LK%04d%02d%02d.cup"), GPS_INFO.Year,GPS_INFO.Month,GPS_INFO.Day);
  _tcscat(fname,tstring);

  stream = _wfopen(fname,TEXT("r"));
  if (stream == NULL)
	dopreambol=true;
  else
	fclose(stream);

  stream = _wfopen(fname,TEXT("a+"));
  if (stream != NULL){
	if (dopreambol) {
		// file was created empty, we need to add preambol header for CUP
		strcpy(message,"name,code,country,lat,lon,elev,style,rwdir,rwlen,freq,desc\r\n");
		fwrite(message,strlen(message),1,stream);
	}

	char marktime[10], slat[20], slon[20], snear[50];
	Units::TimeToTextSimple(tstring,TimeLocal((int)GPS_INFO.Time));
	unicodetoascii(tstring,_tcslen(tstring),marktime);

	LatitudeToCUPString(lat,tstring);
	unicodetoascii(tstring,_tcslen(tstring),slat);
	LongitudeToCUPString(lon,tstring);
	unicodetoascii(tstring,_tcslen(tstring),slon);

	int j=FindNearestFarVisibleWayPoint(lon,lat,15000,WPT_UNKNOWN);
	if (j>0) {
        	wcscpy(tstring,WayPointList[j].Name); // Name is sized NAME_SIZE, 30, so ok with tstring[50]
        	tstring[19]='\0'; // sized 20 chars
		unicodetoascii(tstring,_tcslen(tstring),snear);
	} else {
		strcpy(snear,"unknown");
	}

	sprintf(message,"MK%s%02d,LK8000,,%s,%s,%d.0m,1,,,,Created on %02d-%02d-%04d at h%s near: %s\r\n",
		marktime,GPS_INFO.Second,slat,slon, iround((int)altitude),
		GPS_INFO.Day,GPS_INFO.Month,GPS_INFO.Year, marktime, snear );

	fwrite(message,strlen(message),1,stream);
	fclose(stream);

extern int GetVirtualWaypointMarkerSlot(void);

	j=GetVirtualWaypointMarkerSlot();

	WayPointList[j].Latitude=lat;
	WayPointList[j].Longitude=lon;
	WayPointList[j].Altitude=altitude;
	WayPointList[j].Visible=TRUE;
	WayPointList[j].FarVisible=TRUE;

	wsprintf(WayPointList[j].Name,_T("MK%S%02d"),marktime,GPS_INFO.Second);
	wsprintf(WayPointList[j].Comment,_T("Near: %S"),snear);

	WayPointCalc[j].WpType=WPT_TURNPOINT;

	// Force updating DoRange otherwise it will pass up to 3 minutes
	// before this marker appears in the 2.3 tps page
	LastDoRangeWaypointListTime=0; 
  }

}

