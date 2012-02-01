/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKDrawWaypoints.cpp,v 1.2 2010/12/11 19:03:52 root Exp root $
*/

#include "externs.h"
#include "MapWindow.h"
#include "LKMapWindow.h"
#include "LKStyle.h"
#include "Bitmaps.h"

#if (WINDOWSPC>0)
#include <wingdi.h>
#endif

#include "utils/heapcheck.h"


MapWaypointLabel_t MapWaypointLabelList[200]; 

int MapWaypointLabelListCount=0;

#define WPCIRCLESIZE        2 // check also duplicate in TextInBox!


bool MapWindow::WaypointInRange(int i) {
  return (zoom.RealScale() <= 10);
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

  if (MapWindow::zoom.RealScale() <=20) for(i=0;i<NumberOfWayPoints;i++) {

	if (WayPointList[i].Visible != TRUE )	continue; // false may not be FALSE?

	if (WayPointCalc[i].IsAirport) {
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
		if ( WayPointCalc[i].IsOutlanding ) {
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

	DrawBitmapX(hdc, WayPointList[i].Screen.x-NIBLSCALE(10), WayPointList[i].Screen.y-NIBLSCALE(10), 20,20, hDCTemp,0,0,SRCPAINT,false);
	DrawBitmapX(hdc, WayPointList[i].Screen.x-NIBLSCALE(10), WayPointList[i].Screen.y-NIBLSCALE(10), 20,20, hDCTemp,20,0,SRCAND,false);

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

	islandable=WayPointCalc[i].IsLandable;

 	    // always in range if MapScale <=10 
	    irange = WaypointInRange(i); 

	    if(MapWindow::zoom.RealScale() > 20) { 
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
	      if(MapWindow::zoom.RealScale() > 4) {
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
	    if( intask || irange || dowrite) {  // irange always set when MapScale <=10 

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
		if ( (islandable && !isairport) && MapWindow::zoom.RealScale() >=10 ) dowrite=0; // FIX then no need to go further

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
		if ( (islandable && !isairport) && MapWindow::zoom.RealScale() >=10 ) dowrite=0; // FIX then no need to go further

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
		//ASSERT(0);
#endif
		break;

	      } // end intask/irange/dowrite

		if (MapWindow::zoom.RealScale()<20 && islandable && dowrite) {
			TextInBox(hdc, Buffer, WayPointList[i].Screen.x+5, WayPointList[i].Screen.y, 0, TextDisplayMode, true); 
			dowrite=false; // do not pass it along
		}

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

		if(i==RESWP_OPTIMIZED) {
			dowrite = DoOptimizeRoute();
		}


	      if (dowrite) { 
		MapWaypointLabelAdd(
				    Buffer,
				    WayPointList[i].Screen.x+5,
				    WayPointList[i].Screen.y,
				    TextDisplayMode,
				    (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY),
				    intask,islandable,isairport,excluded,i,WayPointList[i].Style);
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

	// If we are at low zoom, use a dot for icons, so we dont clutter the screen
	if(MapWindow::zoom.RealScale() > 4) {
		if (BlackScreen) 
	 		 SelectObject(hDCTemp,hInvSmall);
		else
	 		 SelectObject(hDCTemp,hSmall);
	} else {
		// We switch all styles in the correct order, to force a jump table by the compiler
		// It would be much better to use an array of bitmaps, but no time to do it for 3.0
		switch(E->style) {
			case STYLE_NORMAL:
				goto turnpoint;
				break;

			// These are not used here in fact
			case STYLE_AIRFIELDGRASS:
			case STYLE_OUTLANDING:
			case STYLE_GLIDERSITE:
			case STYLE_AIRFIELDSOLID:
				goto turnpoint;
				break;

			case STYLE_MTPASS:
				SelectObject(hDCTemp,hMountpass);
				break;

			case STYLE_MTTOP:
				SelectObject(hDCTemp,hMountop);
				break;

			case STYLE_SENDER:
			case STYLE_VOR:
			case STYLE_NDB:
			case STYLE_COOLTOWER:
			case STYLE_DAM:
			case STYLE_TUNNEL:
				goto turnpoint;
				break;
			case STYLE_BRIDGE:
				SelectObject(hDCTemp,hBridge);
				break;
			case STYLE_POWERPLANT:
			case STYLE_CASTLE:
				goto turnpoint;
				break;
			case STYLE_INTERSECTION:
				SelectObject(hDCTemp,hIntersect);
				break;
			case STYLE_TRAFFIC:
			case STYLE_THERMAL:
				goto turnpoint;
				break;

			case STYLE_MARKER:
				SelectObject(hDCTemp,hBmpMarker);
				break;

			default:
turnpoint:
				if (BlackScreen)
					SelectObject(hDCTemp,hInvTurnPoint);
				else
					SelectObject(hDCTemp,hTurnPoint);
				break;

		} // switch estyle
	} // below zoom threshold

	// We dont do stretching here. We are using different bitmaps for hi res.
	// The 20x20 size is large enough to make much bigger icons than the old ones.
	DrawBitmapX(hdc,
		    E->Pos.x-NIBLSCALE(10), 
		    E->Pos.y-NIBLSCALE(10),
		    20,20,
		    hDCTemp,0,0,SRCPAINT,false);
        
	DrawBitmapX(hdc,
		    E->Pos.x-NIBLSCALE(10), 
		    E->Pos.y-NIBLSCALE(10),
		    20,20,
		    hDCTemp,20,0,SRCAND,false);
      }


    }
  }

} // end DrawWaypoint


