/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKDrawWaypoints.cpp,v 1.2 2010/12/11 19:03:52 root Exp root $
*/

#include "externs.h"
#include "LKMapWindow.h"
#include "LKStyle.h"
#include "Bitmaps.h"
#include "DoInits.h"
#include "LKObjects.h"
#include "RGB.h"
#include "Multimap.h"
#include "ScreenGeometry.h"
#include <string.h>
#include "Draw/ScreenProjection.h"
#include "Util/TruncateString.hpp"
#include "Asset.hpp"


MapWaypointLabel_t MapWaypointLabelList[200];
MapWaypointLabel_t* SortedWaypointLabelList[200];

size_t MapWaypointLabelListCount=0;
void DrawMAPWaypointPictoUTF8(LKSurface& Surface, const RECT& rc, int Style);

struct MapWaypointLabelListCompare {
	bool operator()(const MapWaypointLabel_t* elem1, const MapWaypointLabel_t* elem2 ) {
		// landable first
		if(elem1->isLandable && !elem2->isLandable) {
			return true;
		}
		if(!elem1->isLandable && elem2->isLandable) {
			return false;
		}

		// thermal last
		if(elem1->isThermal && !elem2->isThermal) {
			return false;
		}
		if(!elem1->isThermal && elem2->isThermal) {
			return true;
		}

		// otherwise sort by arrival height
		return (elem1->AltArivalAGL < elem2->AltArivalAGL);
	}
};

static
void MapWaypointLabelAdd(const TCHAR *Name, const RasterPoint& pos,
			 const TextInBoxMode_t *Mode,
			 int AltArivalAGL, bool inTask, bool isLandable, bool isAirport, 
			 bool isThermal, bool isExcluded,  int index, short style){

  if (MapWaypointLabelListCount >= array_size(MapWaypointLabelList)-1) return;

  MapWaypointLabel_t* E = &MapWaypointLabelList[MapWaypointLabelListCount];

  _tcscpy(E->Name, Name);
  E->Pos = pos;
  memcpy((void*)&(E->Mode), Mode, sizeof(TextInBoxMode_t));     // E->Mode = Mode;
  E->AltArivalAGL = AltArivalAGL;
  E->inTask = inTask;
  E->isLandable = isLandable;
  E->isAirport  = isAirport;
  E->isThermal  = isThermal;
  E->isExcluded = isExcluded;
  E->index = index;
  E->style = style;

  SortedWaypointLabelList[MapWaypointLabelListCount] = E;
  MapWaypointLabelListCount++;

}






void MapWindow::DrawWaypointsNew(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj)
{
  unsigned int i;
  int bestwp=-1;
  TCHAR Buffer[LKSIZEBUFFER];
  TCHAR Buffer2[LKSIZEBUFFER];
  TCHAR sAltUnit[LKSIZEBUFFERUNIT];
  TextInBoxMode_t TextDisplayMode = {0};

  static int resizer[10]={ 20,20,20,3,5,8,10,12,0 };


  // if pan mode, show full names
  int pDisplayTextType = DisplayTextType;

  if (WayPointList.empty()) return;

  _tcscpy(sAltUnit, Units::GetAltitudeName());

  MapWaypointLabelListCount = 0;

  int arrivalcutoff=0, foundairport=0;

  // setting decluttericons will not paint outlanding, and use minrunway to declutter even more
  bool decluttericons=false;
  // inrange : max scale allowed to print non-landable waypoints
  bool inrange=true;
  // minimal size of a runway to paint it while decluttering
  int minrunway=0;

  //
  // RealScale
  //
  //   2km 	 1.42
  // 3.5km	 2.5
  //   5km	 3.57
  // 7.5km	 5.35
  //  10km	 7.14
  //  15km	10.71
  //  20km	14.28
  //  25km	17.85
  //  40km	28.57
  //  50km	35.71
  //  75km	53.57

  switch(DeclutterMode) {
	case dmDisabled:
		//inrange=(MapWindow::zoom.RealScale() <=18 ? true:false); // 17.85, 25km scale
		inrange=(MapWindow::zoom.RealScale() <=15 ? true:false); // 14.28, 20km scale
		decluttericons=false;
		break;
	case dmLow:
		inrange=(MapWindow::zoom.RealScale() <=11 ? true:false); // 10.71, 15km scale
		decluttericons=(MapWindow::zoom.RealScale() >=14 ? true : false);
		minrunway=200;
		break;
	case dmMedium:
		inrange=(MapWindow::zoom.RealScale() <=10 ? true:false);
		decluttericons=(MapWindow::zoom.RealScale() >=10 ? true : false);
		minrunway=400;
		break;
	case dmHigh:
		inrange=(MapWindow::zoom.RealScale() <=10 ? true:false);
		decluttericons=(MapWindow::zoom.RealScale() >=10 ? true : false);
		minrunway=800;
		break;
	case dmVeryHigh:
		inrange=(MapWindow::zoom.RealScale() <=10 ? true:false);
		decluttericons=(MapWindow::zoom.RealScale() >=10 ? true : false);
		minrunway=1600;
		break;
	default:
		LKASSERT(0);
		break;
  }

  if (MapWindow::zoom.RealScale() <=20) for(i=0;i<WayPointList.size();i++) {
	if (WayPointList[i].Visible != TRUE )	continue; // false may not be FALSE?

    if(Appearance.IndLandable == wpLandableDefault)
    {

      double fScaleFact = zoom.RealScale();

      if (decluttericons) {
        if (WayPointCalc[i].IsAirport && (WayPointList[i].RunwayLen>minrunway || WayPointList[i].RunwayLen==0)) {
            DrawRunway(Surface,&WayPointList[i],rc, &_Proj, fScaleFact);
        }
      } else {
        DrawRunway(Surface,&WayPointList[i],rc, &_Proj, fScaleFact);
      }
    }
    else
    {
        const LKIcon* pWptBmp = NULL;
        if (WayPointCalc[i].IsAirport) {
            if (WayPointList[i].Reachable == FALSE)	{
                pWptBmp = &hBmpAirportUnReachable;
            } else {
                pWptBmp = &hBmpAirportReachable;
                if ( arrivalcutoff < (int)(WayPointList[i].AltArivalAGL)) {
                    arrivalcutoff = (int)(WayPointList[i].AltArivalAGL);
                    bestwp=i; foundairport++;
                }
            }
        } else {
            if ( WayPointCalc[i].IsOutlanding ) {
                // outlanding
                if (WayPointList[i].Reachable == FALSE)
                    pWptBmp = &hBmpFieldUnReachable;
                else {
                    pWptBmp = &hBmpFieldReachable;
                    // get the outlanding as bestwp only if no other choice
                    if (foundairport == 0) {
                        // do not set arrivalcutoff: any next reachable airport is better than an outlanding
                        if ( arrivalcutoff < (int)(WayPointList[i].AltArivalAGL)) bestwp=i;
                    }
                }
            } else {
                continue; // do not draw icons for normal turnpoints here
            }
        }


        if(pWptBmp) {
            // TODO 
            const unsigned IconSize = (UseHiresBitmap ? iround(IBLSCALE(10.0)) : 20); 
            const RasterPoint ScreenPt =  _Proj.ToRasterPoint(WayPointList[i].Latitude, WayPointList[i].Longitude);
            pWptBmp->Draw(Surface, ScreenPt.x-IconSize/2, ScreenPt.y-IconSize/2, IconSize,IconSize);
        }
    }
  } // for all waypoints

  if (foundairport==0 && bestwp>=0)  arrivalcutoff = (int)WayPointList[bestwp].AltArivalAGL;

  const RECT ClipRect = {
      rc.left-WPCIRCLESIZE,
      rc.top-WPCIRCLESIZE,
      rc.right+(WPCIRCLESIZE*3),
      rc.bottom+WPCIRCLESIZE
  };

  for(i=0;i<WayPointList.size();i++) {

	if(WayPointList[i].Visible) {

	    memset((void*)&TextDisplayMode, 0, sizeof(TextDisplayMode));

	    bool excluded=false;

	    bool intask = WaypointInTask(i);
	    bool dowrite = intask; // initially set only for intask

	    // airports are also landpoints. should be better handled
	    bool isairport = ((WayPointList[i].Flags & AIRPORT) == AIRPORT);
	    bool islandpoint = ((WayPointList[i].Flags & LANDPOINT) == LANDPOINT);
	    bool islandable = WayPointCalc[i].IsLandable;
		bool isthermal = (WayPointList[i].Style == STYLE_THERMAL);

	    // always in range if MapScale <=10
	    bool irange = inrange;

	    if(MapWindow::zoom.RealScale() > 20) {
	      irange=false;
	      goto NiklausWirth; // with compliments
	    }
	    if (decluttericons) {
		if (! (WayPointCalc[i].IsAirport && (WayPointList[i].RunwayLen>minrunway || WayPointList[i].RunwayLen==0))) {
		      irange=false;
		      goto NiklausWirth;
		}
            }

	    if( islandable ) {

	      if(WayPointList[i].Reachable){

		TextDisplayMode.Reachable = 1;


		if ((GetMultimap_Labels()<MAPLABELS_ALLOFF)||intask) {

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


	      }
	    }

	  NiklausWirth:

	    if (intask || (OutlinedTp==(OutlinedTp_t)otAll) ) {
	      TextDisplayMode.WhiteBold = 1;
	      TextDisplayMode.Color=RGB_WHITE;
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

	      bool draw_alt = TextDisplayMode.Reachable && ((GetMultimap_Labels()<MAPLABELS_ONLYTOPO) || intask); // 100711 reachable landing point!

	      if (excluded==true) draw_alt=false; // exclude close outlandings

	      switch(pDisplayTextType) {

	     case DISPLAYNAME:
	     case DISPLAYFIRSTTHREE:
	     case DISPLAYFIRSTFIVE:
	     case DISPLAYFIRST8:
	     case DISPLAYFIRST10:
	     case DISPLAYFIRST12:
	     case DISPLAYICAO:
		dowrite = (GetMultimap_Labels()<MAPLABELS_ONLYTOPO) || intask || islandable;  // 100711
		if ( (islandable && !isairport) && MapWindow::zoom.RealScale() >=10 ) dowrite=0; // FIX then no need to go further

		// 101215
		if (DisplayTextType == DISPLAYICAO) {
			_tcscpy(Buffer2,WayPointList[i].Code);
		} else {
		  if (DisplayTextType == DISPLAYNAME) {
			 _tcscpy(Buffer2,WayPointList[i].Name);
		  } else {
			CopyTruncateString(Buffer2, array_size(Buffer2), WayPointList[i].Name, resizer[DisplayTextType]);
		  }
		}





		if (draw_alt) {
		  switch ( ArrivalValue )
		  {
		    case avNone :
		      _tcscpy(Buffer,Buffer2);
		    break;

		    case avGR :
			  _stprintf(Buffer, TEXT("%s:%d"), Buffer2, (int)WayPointCalc[i].GR);
			break;

		    case avAltitude :
			  if ( (MapBox == (MapBox_t)mbUnboxedNoUnit) || (MapBox == (MapBox_t)mbBoxedNoUnit) )
			  	_stprintf(Buffer, TEXT("%s:%d"), Buffer2, (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY));
			  else
			  	_stprintf(Buffer, TEXT("%s:%d%s"), Buffer2, (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY), sAltUnit);
	        break;

	        case avGR_Altitude :
			  if ( (MapBox == (MapBox_t)mbUnboxedNoUnit) || (MapBox == (MapBox_t)mbBoxedNoUnit) )
				_stprintf(Buffer, TEXT("%s:%d/%d"), Buffer2, (int)WayPointCalc[i].GR, (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY));
			  else
				_stprintf(Buffer, TEXT("%s:%d/%d%s"), Buffer2, (int)WayPointCalc[i].GR, (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY), sAltUnit);
			  break;

		  }
		  if ( (MapBox == (MapBox_t)mbBoxed) || (MapBox == (MapBox_t)mbBoxedNoUnit)) {
			  TextDisplayMode.Border = 1;
			  TextDisplayMode.WhiteBold = 0;
		  } else {
            TextDisplayMode.WhiteBold = 1; // outlined
          }
          TextDisplayMode.Color=RGB_WHITE;
		} else {
			//_stprintf(Buffer, TEXT("%s"),Buffer2);
			_tcscpy(Buffer,Buffer2);
			if (islandable && isairport) {
				TextDisplayMode.WhiteBold = 1; // outlined
				TextDisplayMode.Color=RGB_WHITE;
			}
		}

		break;
	      case DISPLAYNUMBER:
		dowrite = (GetMultimap_Labels()<MAPLABELS_ONLYTOPO) || intask || islandable;
		if ( (islandable && !isairport) && MapWindow::zoom.RealScale() >=10 ) dowrite=0; // FIX then no need to go further

		if (draw_alt) {
		  if ( ArrivalValue == (ArrivalValue_t) avAltitude ) {
			if ( (MapBox == (MapBox_t)mbUnboxedNoUnit) || (MapBox == (MapBox_t)mbBoxedNoUnit) )
				_stprintf(Buffer, TEXT("%d:%d"), WayPointList[i].Number, (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY));
			else
				_stprintf(Buffer, TEXT("%d:%d%s"), WayPointList[i].Number, (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY), sAltUnit);
		  } else
		  	if ( ArrivalValue == (ArrivalValue_t) avGR )
				_stprintf(Buffer, TEXT("%d:%d"), WayPointList[i].Number, (int)(WayPointCalc[i].GR));
			else  // only choice left is ( ArrivalValue == (ArrivalValue_t) avGR_Altitude )
				if ( (MapBox == (MapBox_t)mbUnboxedNoUnit) || (MapBox == (MapBox_t)mbBoxedNoUnit) )
					_stprintf(Buffer, TEXT("%d:%d/%d"), WayPointList[i].Number, (int)(WayPointCalc[i].GR), (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY));
				else
					_stprintf(Buffer, TEXT("%d:%d/%d%s"), WayPointList[i].Number, (int)(WayPointCalc[i].GR), (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY), sAltUnit);

		  if ( (MapBox == (MapBox_t)mbBoxed) || (MapBox == (MapBox_t)mbBoxedNoUnit)) {
			  TextDisplayMode.Border = 1;
			  TextDisplayMode.WhiteBold = 0;
		  } else {
			TextDisplayMode.WhiteBold = 1; // outlined
          }
          TextDisplayMode.Color=RGB_WHITE;
		} else {
		  _stprintf(Buffer, TEXT("%d"),WayPointList[i].Number);
		  if (islandable && isairport) {
		    TextDisplayMode.WhiteBold = 1; // outlined
			TextDisplayMode.Color=RGB_WHITE;
		  }
		}
		break;



	      case DISPLAYNAMEIFINTASK:
		dowrite = intask;
		if (intask) {
		  if (draw_alt) {
		  if ( ArrivalValue == (ArrivalValue_t) avAltitude ) {
			if ( (MapBox == (MapBox_t)mbUnboxedNoUnit) || (MapBox == (MapBox_t)mbBoxedNoUnit) )
			    _stprintf(Buffer, TEXT("%s:%d"),
				     WayPointList[i].Name,
				     (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY));
			else
			    _stprintf(Buffer, TEXT("%s:%d%s"),
				     WayPointList[i].Name,
				     (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY),
				     sAltUnit);
		  } else
		  if ( ArrivalValue == (ArrivalValue_t) avGR )
			  _stprintf(Buffer, TEXT("%s:%d"),
				     WayPointList[i].Name,
				     (int)(WayPointCalc[i].GR));
		  else  // only choice left is ( ArrivalValue == (ArrivalValue_t) avGR_Altitude )
		  if ( (MapBox == (MapBox_t)mbUnboxedNoUnit) || (MapBox == (MapBox_t)mbBoxedNoUnit) )
			  _stprintf(Buffer, TEXT("%s:%d/%d"),
			  		 WayPointList[i].Name,
			  		 (int)(WayPointCalc[i].GR),
			  		 (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY));
		  else
			  _stprintf(Buffer, TEXT("%s:%d/%d%s"),
			  		 WayPointList[i].Name,
					 (int)(WayPointCalc[i].GR),
					 (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY),
					 sAltUnit);

		  if ( (MapBox == (MapBox_t)mbBoxed) || (MapBox == (MapBox_t)mbBoxedNoUnit)) {
			  TextDisplayMode.Border = 1;
			  TextDisplayMode.WhiteBold = 0;
		  } else {
			TextDisplayMode.WhiteBold = 1; // outlined
          }
          TextDisplayMode.Color=RGB_WHITE;

		  }
		  else {
		    _stprintf(Buffer, TEXT("%s"),WayPointList[i].Name);
		    // TODO CHECK THIS, UNTESTED..
                    if (islandable && isairport) {
		       TextDisplayMode.WhiteBold = 1; // outlined
			TextDisplayMode.Color=RGB_WHITE;
		    }
		  }
		}
			else {
				_stprintf(Buffer, TEXT(" "));
				dowrite=true;
			}
		break;
	      case DISPLAYNONE:
		dowrite = (GetMultimap_Labels()<MAPLABELS_ONLYTOPO) || intask || islandable;
		if (draw_alt) {
		  switch(ArrivalValue)
          {
		    case  avAltitude:		    
			  if ( (MapBox == (MapBox_t)mbUnboxedNoUnit) || (MapBox == (MapBox_t)mbBoxedNoUnit) )
			    _stprintf(Buffer, TEXT("%d"),
				   (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY));
			  else
			    _stprintf(Buffer, TEXT("%d%s"),
				   (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY),
				   sAltUnit);
		    break;

		    case avGR:
			  _stprintf(Buffer, TEXT("%d"),
				   (int)(WayPointCalc[i].GR) );
		    break;

		  	case avGR_Altitude:
			  if ( (MapBox == (MapBox_t)mbUnboxedNoUnit) || (MapBox == (MapBox_t)mbBoxedNoUnit) )
				_stprintf(Buffer, TEXT("%d/%d"),
				   (int)(WayPointCalc[i].GR),
				   (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY));
			  else
				_stprintf(Buffer, TEXT("%d/%d%s"),
				   (int)(WayPointCalc[i].GR),
				   (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY),
				   sAltUnit);
			  break;

		    default: Buffer[0] = '\0';
		    break;
          }
		  if ( (MapBox == (MapBox_t)mbBoxed) || (MapBox == (MapBox_t)mbBoxedNoUnit)) {
			  TextDisplayMode.Border = 1;
			  TextDisplayMode.WhiteBold = 0;
		  } else
			TextDisplayMode.WhiteBold = 1; // outlined
		    TextDisplayMode.Color=RGB_WHITE;
		}
		else {
			_stprintf(Buffer, TEXT(" "));
		}
		break;

          default:
		break;

	      } // end intask/irange/dowrite

    if (MapWindow::zoom.RealScale()<20 && islandable && dowrite) {
	  const RasterPoint ScreenPt =  _Proj.ToRasterPoint(WayPointList[i].Latitude, WayPointList[i].Longitude);
      TextInBox(Surface, &rc, Buffer, ScreenPt.x+IBLSCALE(4), ScreenPt.y, &TextDisplayMode, true);
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
			RasterPoint LabelPos =  _Proj.ToRasterPoint(WayPointList[i].Latitude, WayPointList[i].Longitude);
			LabelPos.x += 5;

            if (PtInRect(&ClipRect, LabelPos)) {

                MapWaypointLabelAdd(
                        Buffer,
                        LabelPos,
                        &TextDisplayMode,
                        (int) (WayPointList[i].AltArivalAGL * ALTITUDEMODIFY),
                        intask, islandable, isairport, isthermal, excluded, i, WayPointList[i].Style);
            }
          }
	    } // end if intask

	} // if visible
    } // for all waypoints

  std::sort( std::begin(SortedWaypointLabelList), 
  			 std::next(SortedWaypointLabelList, MapWaypointLabelListCount), 
			 MapWaypointLabelListCompare());

  // now draw task/landable waypoints in order of range (closest last)
  // writing unconditionally

  for (int j=MapWaypointLabelListCount-1; j>=0; j--){

    MapWaypointLabel_t *E = SortedWaypointLabelList[j];

    // draws if they are in task unconditionally,
    // otherwise, does comparison
    if ( E->inTask || (E->isLandable && !E->isExcluded) ) {
    const LKIcon* pWptBmp = NULL;

	TextInBox(Surface, &rc, E->Name, E->Pos.x, E->Pos.y+NIBLSCALE(1), &(E->Mode), false);

	// At low zoom, dont print the bitmap because drawn task would make it look offsetted
	if(MapWindow::zoom.RealScale() > 2) continue;

    // If we are at low zoom, use a dot for icons, so we dont clutter the screen
    if(MapWindow::zoom.RealScale() > 1) {
	if (BlackScreen)
		pWptBmp = &hInvSmall;
	else
		pWptBmp = &hSmall;
    } else {
	if (BlackScreen)
		pWptBmp = &hInvTurnPoint;
	else
		pWptBmp = &hTurnPoint;
    }
    if(pWptBmp) {
        // TODO
        const unsigned IconSize = (UseHiresBitmap ? iround(IBLSCALE(10.0)) : 20); 
        pWptBmp->Draw(Surface, E->Pos.x-IconSize/2,E->Pos.y-IconSize/2,IconSize,IconSize);
    }
    } // wp in task
  } // for all waypoint, searching for those in task

  // now draw normal waypoints in order of range (furthest away last)
  // without writing over each other (or the task ones)
  for (size_t j=0; j<MapWaypointLabelListCount; j++) {
    MapWaypointLabel_t *E = SortedWaypointLabelList[j];

    if (!E->inTask && !E->isLandable ) {
      const LKIcon* pWptBmp = NULL;

      if ( TextInBox(Surface, &rc, E->Name, E->Pos.x, E->Pos.y+NIBLSCALE(1), &(E->Mode), true) == true) {

	// If we are at low zoom, use a dot for icons, so we dont clutter the screen
	if(MapWindow::zoom.RealScale() > 4) {
		if (BlackScreen)
			pWptBmp = &hInvSmall;
		else
			pWptBmp = &hSmall;
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
				pWptBmp = &hMountpass;
				break;

			case STYLE_MTTOP:
				pWptBmp = &hMountop;
				break;

			case STYLE_SENDER:
				pWptBmp = &hSender;
				break;

			case STYLE_VOR:
				pWptBmp= &hVor;
				break;

			case STYLE_NDB:
				pWptBmp = &hNdb;
				break;

			case STYLE_COOLTOWER:
				pWptBmp = &hCoolTower;
				break;

			case STYLE_DAM:
				pWptBmp = &hDam;
				break;

			case STYLE_TUNNEL:
				pWptBmp = &hTunnel;
				break;

			case STYLE_BRIDGE:
				pWptBmp = &hBridge;
				break;

			case STYLE_POWERPLANT:
				pWptBmp = &hPowerPlant;
				break;

			case STYLE_CASTLE:
				pWptBmp = &hCastle;
				break;

			case STYLE_INTERSECTION:
				pWptBmp = &hIntersect;
				break;

			case STYLE_TRAFFIC:
				goto turnpoint;
				break;
			case STYLE_THERMAL:
                if(E->AltArivalAGL>0) {
                    pWptBmp = &hLKThermal;
                } else {
                    pWptBmp = &hLKThermalRed;
                }
				break;

			case STYLE_MARKER:
				pWptBmp = &hBmpMarker;
				break;

			default:
turnpoint:
				if (BlackScreen)
					pWptBmp = &hInvTurnPoint;
				else
					pWptBmp = &hTurnPoint;
				break;

		} // switch estyle
	} // below zoom threshold
      if(Appearance.UTF8Pictorials == false)
      {
        if(pWptBmp) {
            // TODO
            const unsigned IconSize = (UseHiresBitmap ? IBLSCALE(10) : 20);
            pWptBmp->Draw(Surface, E->Pos.x - IconSize/2, E->Pos.y - IconSize/2, IconSize, IconSize);
        }
      }
      else
      {
        RECT rctmp;
        int ytext = Surface.GetTextHeight(_T("X"));
        rctmp.left   = E->Pos.x;
        rctmp.top    = E->Pos.y-ytext ;;
        rctmp.right  = E->Pos.x;
        rctmp.bottom = E->Pos.y;

        DrawMAPWaypointPictoUTF8( Surface, rctmp, E->style);
      }
      }
    }
  }

} // end DrawWaypoint


//
// WARNING THESE FUNCTIONS ARE CALLED BY DIALOG MAIN THREAD, not from DRAW THREAD
//

void MapWindow::DrawWaypointPictoBg(LKSurface& Surface, const RECT& rc) {

    if (!hLKPictori)
        return;
    const int cx = rc.right - rc.left;
    const int cy = rc.bottom - rc.top;
    const int x = rc.left;
    const int y = rc.top;

#ifdef USE_GDI
// TODO : replace by real size of Bitmap
    const int cxSrc = UseHiresBitmap?100:45;
    const int cySrc = UseHiresBitmap?100:45;
#else
    const int cxSrc = hLKPictori.GetWidth();
    const int cySrc = hLKPictori.GetHeight();
#endif

    Surface.DrawBitmapCopy(x, y, cx, cy, hLKPictori, cxSrc, cySrc);
}





LKColor GetUTF8WaypointSymbol(TCHAR* pPict, const int Style)
{
  LKColor Col = RGB_BLACK;
  if (pPict ==NULL) return Col;
  switch(Style)
  {
    case STYLE_NORMAL:
      _tcscpy(pPict, MsgToken(2361));    // _@M2361_ "◎"
      Col = RGB_DARKGREEN;
    break;

    // These are not used here in fact
    case STYLE_AIRFIELDGRASS:
    case STYLE_OUTLANDING:
    case STYLE_GLIDERSITE:
    case STYLE_AIRFIELDSOLID:
      _tcscpy(pPict, MsgToken(2362));      // _@M2362_ "✈"
      Col = LKColor(131,111,255);
    break;

    case STYLE_MTPASS:
      _tcscpy(pPict, MsgToken(2363));      // _@M2363_ "◠"
      Col = RGB_ORANGE;
    break;

    case STYLE_MTTOP:
      _tcscpy(pPict,MsgToken(2364));    //  _@M2364_ "▴"
      Col = RGB_BLACK;
    break;

    case STYLE_SENDER:
      _tcscpy(pPict, MsgToken(2365));    // _@M2365_ "☨"
      Col = RGB_DARKGREY;
    break;

    case STYLE_VOR:
      _tcscpy(pPict, MsgToken(2366));  // _@M2366_ "⬡"
      Col = RGB_DARKGREY;
    break;

    case STYLE_NDB:
      _tcscpy(pPict, MsgToken(2367));    // _@M2367_ "✺"
      Col = RGB_DARKGREY;
    break;

    case STYLE_COOLTOWER:
      _tcscpy(pPict, MsgToken(2368));    //  _@M2368_ "⛃"
      Col = LKColor(82,82,82);
    break;

    case STYLE_DAM:
      _tcscpy(pPict, MsgToken(2369)); // _@M2369_ "≊"
      Col = RGB_DARKBLUE;
    break;

    case STYLE_TUNNEL:
      _tcscpy(pPict, MsgToken(2370));  // _@M2370_ "⋒"
      Col = RGB_BLACK;
    break;

    case STYLE_BRIDGE:
      _tcscpy(pPict, MsgToken(2371));   // _@M2371_ "≍"
      Col = RGB_DARKGREY;
    break;

    case STYLE_POWERPLANT:
      _tcscpy(pPict, MsgToken(2372));    // _@M2372_ "⚡"
      Col = LKColor(82,82,82);
    break;

    case STYLE_CASTLE:
      _tcscpy(pPict, MsgToken(2373));  // _@M2373_ "♜"
      Col = LKColor(139,54,38);
    break;

    case STYLE_INTERSECTION:
      _tcscpy(pPict, MsgToken(2374));  // _@M2374_ "⨯"
      Col = RGB_BLACK;
    break;

    case STYLE_TRAFFIC:
      _tcscpy(pPict, MsgToken(2375));   // _@M2375_ "✈"
      Col = RGB_BLACK;
    break;
    case STYLE_THERMAL:
      _tcscpy(pPict, MsgToken(2376));   // _@M2376_ "☁️"
      Col = RGB_LIGHTBLUE;
    break;

    case STYLE_MARKER:
      _tcscpy(pPict, MsgToken(2377));  // _@M2377_ "⚑"
      Col = LKColor(199,21,133);
    break;

    default:
      _tcscpy(pPict, MsgToken(2378));  // _@M2378_ "✈"
      Col = LKColor(199,21,133);
    break;
  } // switch estyle
	if (IsDithered()) {
		Col = LKColor(30, 30, 30);
	}
return Col;
}




void UTF8DrawWaypointPictorial(LKSurface& Surface, const RECT& rc,const TCHAR *Pict ,const LKColor& Color)
{
  if (Pict == NULL) return;

  const auto OldColor = Surface.SetTextColor(Color);
  int xtext = Surface.GetTextWidth(Pict);
  int ytext = Surface.GetTextHeight(Pict);
  Surface.DrawText(rc.left -xtext/2 , rc.top+ytext/2, Pict);
  Surface.SetTextColor(OldColor);
}



void DrawMAPWaypointPictoUTF8(LKSurface& Surface, const RECT& rc, int Style)
{
  TCHAR Pictor[10];
  LKColor Col =  GetUTF8WaypointSymbol(Pictor, Style);
  UTF8DrawWaypointPictorial( Surface, rc, Pictor ,Col);
}


void UTF8WaypointPictorial(LKSurface& Surface, const RECT& rc, const WAYPOINT* wp )
{
if (wp == NULL) return;
  TCHAR Pict[10];

  const auto OldFont =  Surface.SelectObject(LK8PanelBigFont);

  LKColor NewCol = GetUTF8WaypointSymbol(Pict, wp->Style);

  const auto OldCol = Surface.SetTextColor(NewCol);
  int xtext = Surface.GetTextWidth(Pict);
  int ytext = Surface.GetTextHeight(Pict);
  Surface.DrawText(rc.left +(rc.right-rc.left-xtext)/2 , rc.top+(rc.bottom-rc.top-ytext)/2 , Pict);
  Surface.SelectObject(OldFont);
  Surface.SetTextColor(OldCol);
}




void MapWindow::DrawWaypointPicto(LKSurface& Surface, const RECT& rc, const WAYPOINT* wp)
{

    if(Appearance.UTF8Pictorials) {
      return UTF8WaypointPictorial( Surface,  rc, wp);
    }

    const LKIcon* pWptBmp = NULL;

switch(wp->Style) {
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
		pWptBmp = &hMountpass;
		break;

	case STYLE_MTTOP:
		pWptBmp = &hMountop;
		break;

	case STYLE_SENDER:
		pWptBmp = &hSender;
		break;

	case STYLE_VOR:
		pWptBmp = &hVor;
		break;

	case STYLE_NDB:
		pWptBmp = &hNdb;
		break;

	case STYLE_COOLTOWER:
		pWptBmp = &hCoolTower;
		break;

	case STYLE_DAM:
		pWptBmp = &hDam;
		break;

	case STYLE_TUNNEL:
		pWptBmp = &hTunnel;
		break;

	case STYLE_BRIDGE:
		pWptBmp = &hBridge;
		break;

	case STYLE_POWERPLANT:
		pWptBmp = &hPowerPlant;
		break;

	case STYLE_CASTLE:
		pWptBmp = &hCastle;
		break;

	case STYLE_INTERSECTION:
		pWptBmp = &hIntersect;
		break;

	case STYLE_TRAFFIC:
		goto turnpoint;
		break;
	case STYLE_THERMAL:
		pWptBmp = &hLKThermal;
		break;

	case STYLE_MARKER:
		pWptBmp = &hBmpMarker;
		break;

	default:
turnpoint:
		if (BlackScreen)
			pWptBmp = &hInvTurnPoint;
		else
			pWptBmp = &hTurnPoint;
		break;

} // switch estyle

    if (pWptBmp) {

        int cx = rc.right - rc.left;
        int cy = rc.bottom - rc.top;
        int x = rc.left + (cx / 2);
        int y= rc.top + (cy / 2);

        if(cx < 40) {
            cy = cx = 20;
        } else {
            cy = cx = IBLSCALE(20);
        }
        x -= cx/2;
        y -= cy/2;

        pWptBmp->Draw(Surface, x, y, cx ,cy);
    }
}
