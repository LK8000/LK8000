/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: MapWindow.cpp,v 8.29 2011/01/06 02:07:52 root Exp root $
*/

#include "externs.h"
#include "MapWindow.h"
#include "Waypointparser.h"
#include "InputEvents.h"
#include <Message.h>
#include "Terrain.h"
#include "LKObjects.h"
#include "Bitmaps.h"
#include "RGB.h"

using std::min;
using std::max;

#define WPCIRCLESIZE        2

extern int iround(double i);

#define DONTDRAWTHEMAP !mode.AnyPan()&&MapSpaceMode!=MSM_MAP
#define MAPMODE8000    !mode.AnyPan()&&MapSpaceMode==MSM_MAP

rectObj MapWindow::screenbounds_latlon;



extern bool userasked;


bool MapWindow::WaypointInTask(int ind) {
  if (!WayPointList) return false;
  return WayPointList[ind].InTask;
}

MapWaypointLabel_t MapWaypointLabelList[200]; 
int MapWaypointLabelListCount=0;

bool MapWindow::WaypointInRange(int i) {
  return ((WayPointList[i].Zoom >= zoom.Scale()*10) 
          || (WayPointList[i].Zoom == 0)) 
    && (zoom.Scale() <= 10);
}

int _cdecl MapWaypointLabelListCompare(const void *elem1, const void *elem2 ){

  // Now sorts elements in task preferentially.
  if (((MapWaypointLabel_t *)elem1)->AltArivalAGL > ((MapWaypointLabel_t *)elem2)->AltArivalAGL)
    return (-1);
  if (((MapWaypointLabel_t *)elem1)->AltArivalAGL < ((MapWaypointLabel_t *)elem2)->AltArivalAGL)
    return (+1);
  return (0);
}


void MapWaypointLabelAdd(TCHAR *Name, int X, int Y, 
			 TextInBoxMode_t Mode, 
			 int AltArivalAGL, bool inTask, bool isLandable, bool isAirport, bool isExcluded, int index, short style){
  MapWaypointLabel_t *E;

  if ((X<MapWindow::MapRect.left-WPCIRCLESIZE)
      || (X>MapWindow::MapRect.right+(WPCIRCLESIZE*3))
      || (Y<MapWindow::MapRect.top-WPCIRCLESIZE)
      || (Y>MapWindow::MapRect.bottom+WPCIRCLESIZE)){
    return;
  }

  if (MapWaypointLabelListCount >= (( (signed int)(sizeof(MapWaypointLabelList)/sizeof(MapWaypointLabel_t)))-1)){  // BUGFIX 100207
    return;
  }

  E = &MapWaypointLabelList[MapWaypointLabelListCount];

  _tcscpy(E->Name, Name);
  E->Pos.x = X;
  E->Pos.y = Y;
  E->Mode = Mode;
  E->AltArivalAGL = AltArivalAGL;
  E->inTask = inTask;
  E->isLandable = isLandable;
  E->isAirport  = isAirport;
  E->isExcluded = isExcluded;
  E->index = index;
  E->style = style;

  MapWaypointLabelListCount++;

}




double MapWindow::GetApproxScreenRange() {
  return (zoom.Scale() * max(MapRect.right-MapRect.left,
                         MapRect.bottom-MapRect.top))
    *1000.0/GetMapResolutionFactor();
}

bool MapWindow::IsDisplayRunning() {
  return (THREADRUNNING && GlobalRunning && ProgramStarted);
}


bool MapWindow::PointInRect(const double &lon, const double &lat,
                            const rectObj &bounds) {
  if ((lon> bounds.minx) &&
      (lon< bounds.maxx) &&
      (lat> bounds.miny) &&
      (lat< bounds.maxy)) 
    return true;
  else
    return false;
}


bool MapWindow::PointVisible(const double &lon, const double &lat) {
  if ((lon> screenbounds_latlon.minx) &&
      (lon< screenbounds_latlon.maxx) &&
      (lat> screenbounds_latlon.miny) &&
      (lat< screenbounds_latlon.maxy)) 
    return true;
  else
    return false;
}


bool MapWindow::PointVisible(const POINT &P)
{
  if(( P.x >= MapRect.left ) 
     &&
     ( P.x <= MapRect.right ) 
     &&
     ( P.y >= MapRect.top  ) 
     &&
     ( P.y <= MapRect.bottom  ) 
     )
    return TRUE;
  else
    return FALSE;
}


// colorcode is taken from a 5 bit AsInt union
void MapWindow::TextColor(HDC hDC, short colorcode) {

	switch (colorcode) {
	case TEXTBLACK: 
		if (BlackScreen) // 091109
		  SetTextColor(hDC,RGB_WHITE);  // black 
		else
		  SetTextColor(hDC,RGB_BLACK);  // black 
	  break;
	case TEXTWHITE: 
		if (BlackScreen) // 091109
		  SetTextColor(hDC,RGB_LIGHTYELLOW);  // white
		else
		  SetTextColor(hDC,RGB_WHITE);  // white
	  break;
	case TEXTGREEN: 
	  SetTextColor(hDC,RGB_GREEN);  // green
	  break;
	case TEXTRED:
	  SetTextColor(hDC,RGB_RED);  // red
	  break;
	case TEXTBLUE:
	  SetTextColor(hDC,RGB_BLUE);  // blue
	  break;
	case TEXTYELLOW:
	  SetTextColor(hDC,RGB_YELLOW);  // yellow
	  break;
	case TEXTCYAN:
	  SetTextColor(hDC,RGB_CYAN);  // cyan
	  break;
	case TEXTMAGENTA:
	  SetTextColor(hDC,RGB_MAGENTA);  // magenta
	  break;
	case TEXTLIGHTGREY: 
	  SetTextColor(hDC,RGB_LIGHTGREY);  // light grey
	  break;
	case TEXTGREY: 
	  SetTextColor(hDC,RGB_GREY);  // grey
	  break;
	case TEXTLIGHTGREEN:
	  SetTextColor(hDC,RGB_LIGHTGREEN);  //  light green
	  break;
	case TEXTLIGHTRED:
	  SetTextColor(hDC,RGB_LIGHTRED);  // light red
	  break;
	case TEXTLIGHTYELLOW:
	  SetTextColor(hDC,RGB_LIGHTYELLOW);  // light yellow
	  break;
	case TEXTLIGHTCYAN:
	  SetTextColor(hDC,RGB_LIGHTCYAN);  // light cyan
	  break;
	case TEXTORANGE:
	  SetTextColor(hDC,RGB_ORANGE);  // orange
	  break;
	case TEXTLIGHTORANGE:
	  SetTextColor(hDC,RGB_LIGHTORANGE);  // light orange
	  break;
	case TEXTLIGHTBLUE:
	  SetTextColor(hDC,RGB_LIGHTBLUE);  // light blue
	  break;
	default:
	  SetTextColor(hDC,RGB_MAGENTA);  // magenta so we know it's wrong: nobody use magenta..
	  break;
	}

}

