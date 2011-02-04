/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKDrawWaypoints.cpp,v 1.2 2010/12/11 19:03:52 root Exp root $
*/

#include "StdAfx.h"
#include "options.h"
#include "Cpustats.h"
#include "XCSoar.h"
#include "Utils2.h"
#include "compatibility.h"
#include "MapWindow.h"
#include "Units.h"
#include "McReady.h"
#include "externs.h"
#include "InputEvents.h"
#include <windows.h>
#include <math.h>
#include <tchar.h>
#include "InfoBoxLayout.h"
#include "Logger.h"
#include "Process.h"
#include "RasterTerrain.h" // 091109
#include "LKUtils.h"
#include "LKMapWindow.h"

#if (WINDOWSPC>0)
#include <wingdi.h>
#endif

extern void DrawGlideCircle(HDC hdc, POINT Orig, RECT rc );
extern void MapWaypointLabelAdd(TCHAR *Name, int X, int Y, TextInBoxMode_t Mode, int AltArivalAGL, bool inTask, 
	bool isLandable, bool isAirport, bool isExcluded, int index);
extern int _cdecl MapWaypointLabelListCompare(const void *elem1, const void *elem2 );

extern void DrawMapSpace(HDC hdc, RECT rc);
extern void DrawNearest(HDC hdc, RECT rc);
extern void DrawNearestTurnpoint(HDC hdc, RECT rc);
extern void DrawCommon(HDC hdc, RECT rc);
extern void DrawWelcome8000(HDC hdc, RECT rc);
#ifdef CPUSTATS
extern void DrawCpuStats(HDC hdc, RECT rc);
#endif
#ifdef DRAWDEBUG
extern void DrawDebug(HDC hdc, RECT rc);
#endif

extern void WriteInfo(HDC hdc, bool *showunit, TCHAR *BufferValue, TCHAR *BufferUnit, TCHAR *BufferTitle, 
				short *columnvalue, short *columntitle, short *row1, short *row2, short *row3);

extern int PDABatteryPercent;
extern int PDABatteryTemperature;
extern int MapWaypointLabelListCount;
extern void ConvToUpper(TCHAR *str);

typedef struct{
  TCHAR Name[NAME_SIZE+1];
  POINT Pos;
  TextInBoxMode_t Mode;
  int AltArivalAGL;
  bool inTask;
  bool isLandable; // VENTA5
  bool isAirport; // VENTA5
  bool isExcluded;
  int  index;
}MapWaypointLabel_t;


extern MapWaypointLabel_t MapWaypointLabelList[];

void MapWindow::DrawWaypointsNew(HDC hdc, const RECT rc)
{
  unsigned int i; 
  int bestwp=-1;
  TCHAR Buffer[LKSIZEBUFFER];
  TCHAR Buffer2[LKSIZEBUFFER];
  TCHAR sAltUnit[LKSIZEBUFFERUNIT];
  TextInBoxMode_t TextDisplayMode;

  static int resizer[10]={ 20,20,20,3,5,8,10,12,0 };


  // if pan mode, show full names
  int pDisplayTextType = DisplayTextType;

  if (!WayPointList) return;

  _tcscpy(sAltUnit, Units::GetAltitudeName());

  MapWaypointLabelListCount = 0;

  int arrivalcutoff=0, foundairport=0;
  bool isairport;
  bool islandpoint;

#ifndef MAP_ZOOM
  if (MapScale <=20) for(i=0;i<NumberOfWayPoints;i++) {
#else /* MAP_ZOOM */
  if (MapWindow::zoom.Scale() <=20) for(i=0;i<NumberOfWayPoints;i++) {
#endif /* MAP_ZOOM */

	if (WayPointList[i].Visible == FALSE )	continue; 

	#ifdef USEISLANDABLE
	if (WayPointCalc[i].IsAirport) {
	#else
	if ((WayPointList[i].Flags & AIRPORT) == AIRPORT) {
	#endif
		if (WayPointList[i].Reachable == FALSE)	{ 
			SelectObject(hDCTemp,hBmpAirportUnReachable);
		} else {
			SelectObject(hDCTemp,hBmpAirportReachable);
			if ( arrivalcutoff < (int)(WayPointList[i].AltArivalAGL)) {
				arrivalcutoff = (int)(WayPointList[i].AltArivalAGL);
				bestwp=i; foundairport++;
			}
		}
	} else {
		#ifdef USEISLANDABLE
		if ( WayPointCalc[i].IsOutlanding ) {
		#else
		if ( (WayPointList[i].Flags & LANDPOINT) == LANDPOINT) {
		#endif
			// outlanding
			if (WayPointList[i].Reachable == FALSE)
				SelectObject(hDCTemp,hBmpFieldUnReachable);
			else { 
				SelectObject(hDCTemp,hBmpFieldReachable);
				// get the outlanding as bestwp only if no other choice
				if (foundairport == 0) { 
					// do not set arrivalcutoff: any next reachable airport is better than an outlanding
					if ( arrivalcutoff < (int)(WayPointList[i].AltArivalAGL)) bestwp=i;  
				}
			}
		} else continue; // do not draw icons for normal turnpoints here
	}

	DrawBitmapX(hdc, WayPointList[i].Screen.x-NIBLSCALE(10), WayPointList[i].Screen.y-NIBLSCALE(10), 20,20, hDCTemp,0,0,SRCPAINT);
	DrawBitmapX(hdc, WayPointList[i].Screen.x-NIBLSCALE(10), WayPointList[i].Screen.y-NIBLSCALE(10), 20,20, hDCTemp,20,0,SRCAND);

  } // for all waypoints

  if (foundairport==0 && bestwp>=0)  arrivalcutoff = (int)WayPointList[bestwp].AltArivalAGL;


  for(i=0;i<NumberOfWayPoints;i++)
    {
      if(WayPointList[i].Visible )
	{

#ifdef HAVEEXCEPTIONS
	  __try{
#endif

	    bool irange = false;
	    bool intask = false;
	    bool islandable;	// isairport+islandpoint
	    bool excluded=false;
	    bool dowrite;

	    intask = WaypointInTask(i);
	    dowrite = intask; // initially set only for intask
	    TextDisplayMode.AsInt = 0;

	    // airports are also landpoints. should be better handled
	    isairport=((WayPointList[i].Flags & AIRPORT) == AIRPORT);
	    islandpoint=((WayPointList[i].Flags & LANDPOINT) == LANDPOINT);

	#if FIXISLANDABLE
	islandable=WayPointCalc[i].IsLandable;
	#else
	    if (isairport || islandpoint) islandable=true; else islandable=false;
	#endif

 	    // always in range if MapScale <=10  since no zoom in waypoints is documented and .Zoom is always 0. 
	    irange = WaypointInRange(i); 

#ifndef MAP_ZOOM
	    if(MapScale > 20) { 
#else /* MAP_ZOOM */
	    if(MapWindow::zoom.Scale() > 20) { 
#endif /* MAP_ZOOM */
	      SelectObject(hDCTemp,hInvSmall);
	      irange=false;
	      goto NiklausWirth; // with compliments
	    } 

	    if( islandable ) { 

	      if(WayPointList[i].Reachable){

		TextDisplayMode.AsFlag.Reachable = 1;

		if ( isairport )
		  SelectObject(hDCTemp,hBmpAirportReachable);
		else
		  SelectObject(hDCTemp,hBmpFieldReachable);

		if ((DeclutterLabels<MAPLABELS_ALLOFF)||intask) { 

		  dowrite = true;
		  // exclude outlandings worst than visible airports, only when there are visible reachable airports!
		  if ( isairport==false && islandpoint==true ) {
		    if ( (int)WayPointList[i].AltArivalAGL >=2000 ) { // more filter
		      excluded=true;
		    } else {
		      if ( (bestwp>=0) && (i==(unsigned)bestwp) && (foundairport==0) ) { // this outlanding is the best option
			isairport=true;
			islandpoint=false; // make it an airport TODO paint it as best
		      } else
			{
			  if ( foundairport >0 ) {
			    if ( (int)WayPointList[i].AltArivalAGL <= arrivalcutoff ) {
			      excluded=true;
			    } 
			  }
			}
		    }

		  }  else
		    // do not display airport arrival if close to the best so far.
		    // ex: best arrival is 1200m, include onlye below 1200/4  (prevent division by zero)
		    // This way we only display far points, and skip closer points 
		    // WE NEED MORE INFO ABOUT LANDING POINTS: THE .CUP FORMAT WILL LET US KNOW WHAT IS
		    // BEST TO SHOW AND WHAT IS NOT. Winpilot format is useless here.
		    {
		      dowrite=true;// TEST FIX not necessary probably
		      // it's an airport
		      if ( (bestwp>=0) && (i != (unsigned)bestwp) && (arrivalcutoff>600) ) {
			if ( (arrivalcutoff / ((int)WayPointList[i].AltArivalAGL+1))<4 ) {
			  excluded=true;
			}
		      }
		    } 
		}


	      } else  // landable waypoint is unreachable
		{
		  dowrite=true; 
		  if ( isairport ) {
		    SelectObject(hDCTemp,hBmpAirportUnReachable);
		  } else {
		    SelectObject(hDCTemp,hBmpFieldUnReachable);
		  }
		}
	    } else { // waypoint is an ordinary turnpoint
#ifndef MAP_ZOOM
	      if(MapScale > 4) {
#else /* MAP_ZOOM */
	      if(MapWindow::zoom.Scale() > 4) {
#endif /* MAP_ZOOM */
		if (BlackScreen) 
			SelectObject(hDCTemp,hInvSmall);
		else
			SelectObject(hDCTemp,hSmall);
	      } else {
		if (BlackScreen) 
			SelectObject(hDCTemp,hInvTurnPoint);
		else
			SelectObject(hDCTemp,hTurnPoint);
	      }

	    } // end landable-not landable

	  NiklausWirth:

	    if (intask || (OutlinedTp==(OutlinedTp_t)otAll) ) { 
	      TextDisplayMode.AsFlag.WhiteBold = 1;
	      TextDisplayMode.AsFlag.Color=TEXTWHITE; 
	    }
	

	// No matter of how we thought to draw it, let it up to the user..
	switch(NewMapDeclutter) {
		case 0:
			excluded=false; // no decluttering: show all airports and outlandings
			break;
		case 1:
			if ( isairport ) excluded=false; //  show all airports, declutter outlandings
			break;
		default:
			break; // else work normally
	}


	    // here come both turnpoints and landables..
	    if( intask || irange || dowrite) {  // irange almost always set when MapScale <=10 

	      bool draw_alt = TextDisplayMode.AsFlag.Reachable && ((DeclutterLabels<MAPLABELS_ONLYTOPO) || intask); // 100711 reachable landing point!

	      if (excluded==true) draw_alt=false; // exclude close outlandings

	      switch(pDisplayTextType) {

	     case DISPLAYNAME:
	     case DISPLAYFIRSTTHREE:
	     case DISPLAYFIRSTFIVE:
	     case DISPLAYFIRST8:
	     case DISPLAYFIRST10:
	     case DISPLAYFIRST12:

		dowrite = (DeclutterLabels<MAPLABELS_ONLYTOPO) || intask || islandable;  // 100711
#ifndef MAP_ZOOM
		if ( (islandable && !isairport) && MapScale >=10 ) dowrite=0; // FIX then no need to go further
#else /* MAP_ZOOM */
		if ( (islandable && !isairport) && MapWindow::zoom.Scale() >=10 ) dowrite=0; // FIX then no need to go further
#endif /* MAP_ZOOM */

		// 101215 
		if (DisplayTextType == DISPLAYNAME) {
			_tcscpy(Buffer2,WayPointList[i].Name);
		} else {
			_tcsncpy(Buffer2, WayPointList[i].Name, resizer[DisplayTextType]);
			Buffer2[resizer[DisplayTextType]] = '\0';
		}
		// ConvToUpper(Buffer2); 

		if (draw_alt) {
		  if ( ArrivalValue == (ArrivalValue_t) avAltitude ) {
			if ( (MapBox == (MapBox_t)mbUnboxedNoUnit) || (MapBox == (MapBox_t)mbBoxedNoUnit) )
		  		wsprintf(Buffer, TEXT("%s:%d"), Buffer2, (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY));
			else
		  		wsprintf(Buffer, TEXT("%s:%d%s"), Buffer2, (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY), sAltUnit);
		  } else 
			wsprintf(Buffer, TEXT("%s:%d"), Buffer2, (int)WayPointCalc[i].GR);


		  if ( (MapBox == (MapBox_t)mbBoxed) || (MapBox == (MapBox_t)mbBoxedNoUnit)) {
			  TextDisplayMode.AsFlag.Border = 1;
			  TextDisplayMode.AsFlag.WhiteBold = 0;
		  } else
		  	TextDisplayMode.AsFlag.WhiteBold = 1; // outlined 
		  	TextDisplayMode.AsFlag.Color=TEXTWHITE; 
		} else {
			//wsprintf(Buffer, TEXT("%s"),Buffer2);
			_tcscpy(Buffer,Buffer2);
			if (islandable && isairport) {
				TextDisplayMode.AsFlag.WhiteBold = 1; // outlined 
				TextDisplayMode.AsFlag.Color=TEXTWHITE; 
			}
		}
				  
		break;
	      case DISPLAYNUMBER:
		dowrite = (DeclutterLabels<MAPLABELS_ONLYTOPO) || intask || islandable;  // 100620
#ifndef MAP_ZOOM
		if ( (islandable && !isairport) && MapScale >=10 ) dowrite=0; // FIX then no need to go further
#else /* MAP_ZOOM */
		if ( (islandable && !isairport) && MapWindow::zoom.Scale() >=10 ) dowrite=0; // FIX then no need to go further
#endif /* MAP_ZOOM */

		if (draw_alt) {
		  if ( ArrivalValue == (ArrivalValue_t) avAltitude ) {
			if ( (MapBox == (MapBox_t)mbUnboxedNoUnit) || (MapBox == (MapBox_t)mbBoxedNoUnit) )
		  		wsprintf(Buffer, TEXT("%d:%d"), WayPointList[i].Number, (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY));
			else
		  		wsprintf(Buffer, TEXT("%d:%d%s"), WayPointList[i].Number, (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY), sAltUnit);
		  } else
		  	wsprintf(Buffer, TEXT("%d:%d"), WayPointList[i].Number, (int)(WayPointCalc[i].GR));
		  if ( (MapBox == (MapBox_t)mbBoxed) || (MapBox == (MapBox_t)mbBoxedNoUnit)) {
			  TextDisplayMode.AsFlag.Border = 1;
			  TextDisplayMode.AsFlag.WhiteBold = 0;
		  } else
		  	TextDisplayMode.AsFlag.WhiteBold = 1; // outlined 
	      		TextDisplayMode.AsFlag.Color=TEXTWHITE; 
		} else {
		  wsprintf(Buffer, TEXT("%d"),WayPointList[i].Number);
		  if (islandable && isairport) {
		    TextDisplayMode.AsFlag.WhiteBold = 1; // outlined 
	      		TextDisplayMode.AsFlag.Color=TEXTWHITE; 
		  }
		}
		break;
				  
				  

	      case DISPLAYNAMEIFINTASK: 
		dowrite = intask;
		if (intask) {
		  if (draw_alt) {
		  if ( ArrivalValue == (ArrivalValue_t) avAltitude ) {
			if ( (MapBox == (MapBox_t)mbUnboxedNoUnit) || (MapBox == (MapBox_t)mbBoxedNoUnit) )
			    wsprintf(Buffer, TEXT("%s:%d"),
				     WayPointList[i].Name, 
				     (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY));
			else
			    wsprintf(Buffer, TEXT("%s:%d%s"),
				     WayPointList[i].Name, 
				     (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY), 
				     sAltUnit);


		  } else
			    wsprintf(Buffer, TEXT("%s:%d"),
				     WayPointList[i].Name, 
				     (int)(WayPointCalc[i].GR)); 

		  if ( (MapBox == (MapBox_t)mbBoxed) || (MapBox == (MapBox_t)mbBoxedNoUnit)) {
			  TextDisplayMode.AsFlag.Border = 1;
			  TextDisplayMode.AsFlag.WhiteBold = 0;
		  } else
		  	TextDisplayMode.AsFlag.WhiteBold = 1; // outlined 
	      		TextDisplayMode.AsFlag.Color=TEXTWHITE; 

		  }
		  else {
		    wsprintf(Buffer, TEXT("%s"),WayPointList[i].Name);
		    // TODO CHECK THIS, UNTESTED..
                    if (islandable && isairport) {
		       TextDisplayMode.AsFlag.WhiteBold = 1; // outlined 
	      		TextDisplayMode.AsFlag.Color=TEXTWHITE; 
		    }
		  }
		}
			else {
		  		wsprintf(Buffer, TEXT(" "));
				dowrite=true;
			}
		break;
	      case DISPLAYNONE:
		dowrite = (DeclutterLabels<MAPLABELS_ONLYTOPO) || intask || islandable;  // 100711
		if (draw_alt) {
		  if ( ArrivalValue == (ArrivalValue_t) avAltitude ) {
			if ( (MapBox == (MapBox_t)mbUnboxedNoUnit) || (MapBox == (MapBox_t)mbBoxedNoUnit) )
			  wsprintf(Buffer, TEXT("%d"), 
				   (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY));
			else
			  wsprintf(Buffer, TEXT("%d%s"), 
				   (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY), 
				   sAltUnit);
		  } else
			  wsprintf(Buffer, TEXT("%d"), 
				   (int)(WayPointCalc[i].GR) );

		  if ( (MapBox == (MapBox_t)mbBoxed) || (MapBox == (MapBox_t)mbBoxedNoUnit)) {
			  TextDisplayMode.AsFlag.Border = 1;
			  TextDisplayMode.AsFlag.WhiteBold = 0;
		  } else
		  	TextDisplayMode.AsFlag.WhiteBold = 1; // outlined 
	      	    TextDisplayMode.AsFlag.Color=TEXTWHITE; 
		}
		else {
		  	wsprintf(Buffer, TEXT(" "));
		}
		break;

	      default:
#if (WINDOWSPC<1)
		ASSERT(0);
#endif
		break;

	      } // end intask/irange/dowrite

		#if MOREDECLUTTER
#ifndef MAP_ZOOM
		if (MapScale<20 && islandable && dowrite) {
#else /* MAP_ZOOM */
		if (MapWindow::zoom.Scale()<20 && islandable && dowrite) {
#endif /* MAP_ZOOM */
			TextInBox(hdc, Buffer, WayPointList[i].Screen.x+5, WayPointList[i].Screen.y, 0, TextDisplayMode, true); 
			dowrite=false; // do not pass it along
		}
		#else
	      if (MapScale>=3 && MapScale<10 && islandable && dowrite) { // can't find a better solution for this 
		if (isairport)
		  TextInBox(hdc, Buffer, WayPointList[i].Screen.x+5, WayPointList[i].Screen.y, 0, TextDisplayMode, false);
		else
		  TextInBox(hdc, Buffer, WayPointList[i].Screen.x+5, WayPointList[i].Screen.y, 0, TextDisplayMode, true);  
		dowrite=false; // do not pass it along
	      }
	      if (MapScale<3 && islandable && dowrite) { // damned irange problems
		TextInBox(hdc, Buffer, WayPointList[i].Screen.x+5, WayPointList[i].Screen.y, 0, TextDisplayMode, false);  
		dowrite=false; // do not pass it along
	      }
	      if (MapScale>=10 && MapScale<20 && islandable && dowrite) { // damned irange problems
		TextInBox(hdc, Buffer, WayPointList[i].Screen.x+5, WayPointList[i].Screen.y, 0, TextDisplayMode, true); 
		dowrite=false; // do not pass it along
	      }
		#endif

		// Do not show takeoff for gliders, check TakeOffWayPoint 
		if (i==RESWP_TAKEOFF) {
			if (TakeOffWayPoint) {
				intask=false; // 091031 let TAKEOFF be decluttered
				WayPointList[i].Visible=TRUE;
			} else {
				WayPointList[i].Visible=FALSE;
				dowrite=false;
			}
		}
	      if (dowrite) { 
		MapWaypointLabelAdd(
				    Buffer,
				    WayPointList[i].Screen.x+5,
				    WayPointList[i].Screen.y,
				    TextDisplayMode,
				    (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY),
				    intask,islandable,isairport,excluded,i);
	      }
	    } // end if intask
      
#ifdef HAVEEXCEPTIONS
	  }__finally
#endif
	     { ; }
	} // if visible
    } // for all waypoints
 
  qsort(&MapWaypointLabelList, MapWaypointLabelListCount, sizeof(MapWaypointLabel_t), MapWaypointLabelListCompare);

  int j;

  // now draw task/landable waypoints in order of range (closest last)
  // writing unconditionally

  for (j=MapWaypointLabelListCount-1; j>=0; j--){

    MapWaypointLabel_t *E = &MapWaypointLabelList[j];

    // draws if they are in task unconditionally,
    // otherwise, does comparison
    if ( E->inTask || (E->isLandable && !E->isExcluded) ) { 
      TextInBox(hdc, E->Name, E->Pos.x,
		E->Pos.y, 0, E->Mode, 
		false); 
    }
  }

  // now draw normal waypoints in order of range (furthest away last)
  // without writing over each other (or the task ones)
  for (j=0; j<MapWaypointLabelListCount; j++) {

    MapWaypointLabel_t *E = &MapWaypointLabelList[j];

    if (!E->inTask && !E->isLandable ) {

      if ( TextInBox(hdc, E->Name, E->Pos.x, E->Pos.y, 0, E->Mode, true) == true) {
#ifndef MAP_ZOOM
	if(MapScale > 4) {
#else /* MAP_ZOOM */
	if(MapWindow::zoom.Scale() > 4) {
#endif /* MAP_ZOOM */
		if (BlackScreen) // 091109
	 		 SelectObject(hDCTemp,hInvSmall);
		else
	 		 SelectObject(hDCTemp,hSmall);
	} else {
		if (BlackScreen) // 091109
	  		SelectObject(hDCTemp,hInvTurnPoint);
		else
	  		SelectObject(hDCTemp,hTurnPoint);
	}

	DrawBitmapX(hdc,
		    E->Pos.x-NIBLSCALE(10), 
		    E->Pos.y-NIBLSCALE(10),
		    20,20,
		    hDCTemp,0,0,SRCPAINT);
        
	DrawBitmapX(hdc,
		    E->Pos.x-NIBLSCALE(10), 
		    E->Pos.y-NIBLSCALE(10),
		    20,20,
		    hDCTemp,20,0,SRCAND);
      }


    }
  }

} // end DrawWaypoint


