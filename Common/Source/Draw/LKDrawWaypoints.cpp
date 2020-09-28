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
#include "utils/array_adaptor.h"
#include "Screen/Point.hpp"

namespace {

	MapWaypointLabel_t MapWaypointLabelList[200];
	MapWaypointLabel_t* SortedWaypointLabelList[200];
	size_t MapWaypointLabelListCount=0;

	const LKIcon& get_turnpoint_icon(short style) {
		switch (style) {
		case STYLE_MTPASS:
			return hMountpass;
		case STYLE_MTTOP:
			return hMountop;
		case STYLE_SENDER:
			return hSender;
		case STYLE_VOR:
			return hVor;
		case STYLE_NDB:
			return hNdb;
		case STYLE_COOLTOWER:
			return hCoolTower;
		case STYLE_DAM:
			return hDam;
		case STYLE_TUNNEL:
			return hTunnel;
		case STYLE_BRIDGE:
			return hBridge;
		case STYLE_POWERPLANT:
			return hPowerPlant;
		case STYLE_CASTLE:
			return hCastle;
		case STYLE_INTERSECTION:
			return hIntersect;
		case STYLE_THERMAL:
			return hLKThermal;
		case STYLE_MARKER:
			return hBmpMarker;
		default:
			return (BlackScreen) ? hInvTurnPoint : hTurnPoint;
		}
	}

	const TCHAR* get_turnpoint_utf8_symbol(short style) {
		switch (style)
		{
		case STYLE_NORMAL:
			return  MsgToken(2361); // _@M2361_ "◎"
		case STYLE_AIRFIELDGRASS:
		case STYLE_OUTLANDING:
		case STYLE_GLIDERSITE:
		case STYLE_AIRFIELDSOLID:
			return MsgToken(2362); // _@M2362_ "✈"
		case STYLE_MTPASS:
			return MsgToken(2363); // _@M2363_ "◠"
		case STYLE_MTTOP:
			return MsgToken(2364); //  _@M2364_ "▴"
		case STYLE_SENDER:
			return MsgToken(2365); // _@M2365_ "☨"
		case STYLE_VOR:
			return MsgToken(2366); // _@M2366_ "⬡"
		case STYLE_NDB:
			return MsgToken(2367); // _@M2367_ "✺"
		case STYLE_COOLTOWER:
			return MsgToken(2368); //  _@M2368_ "⛃"
		case STYLE_DAM:
			return MsgToken(2369); // _@M2369_ "≊"
		case STYLE_TUNNEL:
			return MsgToken(2370); // _@M2370_ "⋒"
		case STYLE_BRIDGE:
			return MsgToken(2371); // _@M2371_ "≍"
		case STYLE_POWERPLANT:
			return MsgToken(2372); // _@M2372_ "⚡"
		case STYLE_CASTLE:
			return MsgToken(2373); // _@M2373_ "♜"
		case STYLE_INTERSECTION:
			return MsgToken(2374); // _@M2374_ "⨯"
		case STYLE_TRAFFIC:
			return MsgToken(2375); // _@M2375_ "✈"
		case STYLE_THERMAL:
			return MsgToken(2376); // _@M2376_ "♨"
		case STYLE_MARKER:
			return MsgToken(2377); // _@M2377_ "⚑"
		default:
			return MsgToken(2378); // _@M2378_ "✈"
		}
	}

	LKColor get_turnpoint_utf8_color(short style) {
		if (IsDithered()) {
			return LKColor(30, 30, 30);
		}
		switch(style) {
		case STYLE_NORMAL:
			return RGB_DARKGREEN;
		case STYLE_AIRFIELDGRASS:
		case STYLE_OUTLANDING:
		case STYLE_GLIDERSITE:
		case STYLE_AIRFIELDSOLID:
			return LKColor(131,111,255);
		case STYLE_MTPASS:
			return RGB_ORANGE;
		case STYLE_SENDER:
		case STYLE_VOR:
		case STYLE_NDB:
		case STYLE_BRIDGE:
			return RGB_DARKGREY;
		case STYLE_COOLTOWER:
			return LKColor(82,82,82);
		case STYLE_DAM:
			return RGB_DARKBLUE;
		case STYLE_POWERPLANT:
			return LKColor(82,82,82);
		case STYLE_CASTLE:
			return LKColor(139,54,38);
		case STYLE_MTTOP:
		case STYLE_TUNNEL:
		case STYLE_INTERSECTION:
		case STYLE_TRAFFIC:
			return RGB_BLACK;
		case STYLE_THERMAL:
			return LKColor(210,105,30);
		case STYLE_MARKER:
		default:
			return LKColor(199,21,133);
		}
	}

	void UTF8DrawWaypointPictorial(LKSurface& Surface, const RECT& rc,const TCHAR *Pict ,const LKColor& Color) {
		if (!Pict) return;

		const auto OldColor = Surface.SetTextColor(Color);
		int xtext = Surface.GetTextWidth(Pict);
		int ytext = Surface.GetTextHeight(Pict);
		Surface.DrawText(rc.left -xtext/2 , rc.top+ytext/2, Pict);
		Surface.SetTextColor(OldColor);
	}

	void UTF8WaypointPictorial(LKSurface& Surface, const RECT& rc, const WAYPOINT* wp ) {
		if (!wp) return;

		const TCHAR* Pict = get_turnpoint_utf8_symbol(wp->Style);
		if(Pict) {
			LKColor NewCol = get_turnpoint_utf8_color(wp->Style);

			const auto OldFont =  Surface.SelectObject(LK8PanelBigFont);
			const auto OldCol = Surface.SetTextColor(NewCol);
			int xtext = Surface.GetTextWidth(Pict);
			int ytext = Surface.GetTextHeight(Pict);
			Surface.DrawText(rc.left +(rc.right-rc.left-xtext)/2 , rc.top+(rc.bottom-rc.top-ytext)/2 , Pict);
			Surface.SelectObject(OldFont);
			Surface.SetTextColor(OldCol);
		}
	}

	void DrawMAPWaypointPictoUTF8(LKSurface& Surface, const RECT& rc, short Style) {
		const TCHAR* Symbol = get_turnpoint_utf8_symbol(Style);
		if(Symbol) {
			LKColor Col = get_turnpoint_utf8_color(Style);
			UTF8DrawWaypointPictorial( Surface, rc, Symbol ,Col);
		}
	}

	struct MapWaypointLabelListCompare {
		bool operator()(const MapWaypointLabel_t* elem1, const MapWaypointLabel_t* elem2 ) {
			// in task first
			if(elem1->inTask && !elem2->inTask) {
				return true;
			}
			if(!elem1->inTask && elem2->inTask) {
				return false;
			}

			if(!elem1->inTask && !elem2->inTask) {
				// if both are not in task, landable become first
				if(elem1->isLandable && !elem2->isLandable) {
					return true;
				}
				if(!elem1->isLandable && elem2->isLandable) {
					return false;
				}

				if(!elem1->isLandable && !elem2->isLandable) {
					// if both are not landable, thermal will be be last
					if(elem1->isThermal && !elem2->isThermal) {
						return false;
					}
					if(!elem1->isThermal && elem2->isThermal) {
						return true;
					}
				}
			}
			// otherwise sort by arrival height (higher first)
			return (elem1->AltArivalAGL > elem2->AltArivalAGL);
		}
	};

	template<size_t size>
	void MapWaypointLabelAdd(const TCHAR (&Name)[size], const RasterPoint& pos,
				 const TextInBoxMode_t *Mode,
				 int AltArivalAGL, bool inTask, bool isLandable, 
				 bool isThermal,  int index, short style){

		static_assert(array_size(MapWaypointLabelList->Name) >= size, "possible buffer overflow" );

		if (MapWaypointLabelListCount >= array_size(MapWaypointLabelList)-1) return;

		MapWaypointLabel_t* E = &MapWaypointLabelList[MapWaypointLabelListCount];

		_tcscpy(E->Name, Name);
		E->Pos = pos;
		memcpy((void*)&(E->Mode), Mode, sizeof(TextInBoxMode_t));     // E->Mode = Mode;
		E->AltArivalAGL = AltArivalAGL;
		E->inTask = inTask;
		E->isLandable = isLandable;
		E->isThermal  = isThermal;
		E->index = index;
		E->style = style;

		SortedWaypointLabelList[MapWaypointLabelListCount] = E;
		MapWaypointLabelListCount++;
	}

} // namespace


void MapWindow::DrawWaypointsNew(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj) {
	int bestwp=-1;
	TCHAR Buffer[LKSIZEBUFFER];
	TCHAR Buffer2[NAME_SIZE+1]; // size must be > size of waypoint name or waypoint code or max of resizer values
	TextInBoxMode_t TextDisplayMode = {0};

	constexpr size_t label_resizer[] = {
		NAME_SIZE + 1,   // DISPLAYNAME 0
		20,              // DISPLAYNUMBER 1
		NAME_SIZE + 1,   // DISPLAYNAMEIFINTASK 2
		3,               // DISPLAYFIRSTTHREE 3
		5,               // DISPLAYFIRSTFIVE 4
		8,               // DISPLAYFIRST8 5
		10,              // DISPLAYFIRST10 6
		12,              // DISPLAYFIRST12 7
		0,               // DISPLAYNONE 8
		CUPSIZE_CODE + 1 // DISPLAYICAO 9
	};

	if (WayPointList.empty()) return;

	const TCHAR* sAltUnit = _T("");
	if( (MapBox == (MapBox_t)mbUnboxedNoUnit) || (MapBox == (MapBox_t)mbBoxedNoUnit) ) {
		sAltUnit = Units::GetAltitudeName();	
	}


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
		//inrange=(MapWindow::zoom.RealScale() <=18); // 17.85, 25km scale
		inrange=(MapWindow::zoom.RealScale() <=15); // 14.28, 20km scale
		decluttericons=false;
		break;
	case dmLow:
		inrange=(MapWindow::zoom.RealScale() <=11); // 10.71, 15km scale
		decluttericons=(MapWindow::zoom.RealScale() >=14);
		minrunway=200;
		break;
	case dmMedium:
		inrange=(MapWindow::zoom.RealScale() <=10);
		decluttericons=(MapWindow::zoom.RealScale() >=10);
		minrunway=400;
		break;
	case dmHigh:
		inrange=(MapWindow::zoom.RealScale() <=10);
		decluttericons=(MapWindow::zoom.RealScale() >=10);
		minrunway=800;
		break;
	case dmVeryHigh:
		inrange=(MapWindow::zoom.RealScale() <=10);
		decluttericons=(MapWindow::zoom.RealScale() >=10);
		minrunway=1600;
		break;
	default:
		LKASSERT(0);
		break;
	}

	// Draw Runaway
	if (MapWindow::zoom.RealScale() <= 20) {
		for(size_t idx = 0;idx < WayPointList.size(); ++idx) {
			const WAYPOINT& tp = WayPointList[idx];
			const WPCALC& tpc = WayPointCalc[idx];

			if (!tp.Visible) {
				continue;
			}
		    if(Appearance.IndLandable == wpLandableDefault) {
				double fScaleFact = zoom.RealScale();
				if (decluttericons) {
					if (tpc.IsAirport && (tp.RunwayLen>minrunway || tp.RunwayLen==0)) {
						DrawRunway(Surface,&tp,rc, &_Proj, fScaleFact);
					}
				} else {
					DrawRunway(Surface,&tp,rc, &_Proj, fScaleFact);
				}
			} else {
				const LKIcon* pWptBmp = nullptr;
				if (tpc.IsAirport) {
					if (tp.Reachable) {
						pWptBmp = &hBmpAirportReachable;
						if (arrivalcutoff < (int) (tp.AltArivalAGL)) {
							arrivalcutoff = (int) (tp.AltArivalAGL);
							bestwp = idx;
							foundairport++;
						}
					} else {
						pWptBmp = &hBmpAirportUnReachable;
					}
				} else if ( tpc.IsOutlanding ) {
					// outlanding
					if (!tp.Reachable) {
						pWptBmp = &hBmpFieldReachable;
						// get the outlanding as bestwp only if no other choice
						if (foundairport == 0) {
							// do not set arrivalcutoff: any next reachable airport is better than an outlanding
							if (arrivalcutoff < (int) (tp.AltArivalAGL)) {
								bestwp = idx;
							}
						}
					} else {
						pWptBmp = &hBmpFieldUnReachable;
					}
				} else {
					continue; // do not draw icons for normal turnpoints here
				}

				if(pWptBmp) {
					// TODO : use real IconSize instead of hardcoded value
					const unsigned IconSize = (UseHiresBitmap ? iround(IBLSCALE(10.0)) : 20); 
					const RasterPoint ScreenPt =  _Proj.ToRasterPoint(tp.Latitude, tp.Longitude);
					pWptBmp->Draw(Surface, ScreenPt.x-IconSize/2, ScreenPt.y-IconSize/2, IconSize,IconSize);
				}
    		}
  		} // for all waypoints
	}

  	if (foundairport==0 && bestwp>=0) {
		arrivalcutoff = (int)WayPointList[bestwp].AltArivalAGL;
	}

	const RECT ClipRect = {
		rc.left-WPCIRCLESIZE,
		rc.top-WPCIRCLESIZE,
		rc.right+(WPCIRCLESIZE*3),
		rc.bottom+WPCIRCLESIZE
	};

	for(size_t idx = 0; idx < WayPointList.size(); ++idx) {
		const WAYPOINT& tp = WayPointList[idx];
		const WPCALC& tpc = WayPointCalc[idx];

		if(!tp.Visible) {
			continue;
		}

		memset((void*)&TextDisplayMode, 0, sizeof(TextDisplayMode));

		bool excluded=false;

		bool intask = tp.InTask;
		bool dowrite = intask; // initially set only for intask

		// airports are also landpoints. should be better handled
		bool isairport = ((tp.Flags & AIRPORT) == AIRPORT);
		bool islandpoint = ((tp.Flags & LANDPOINT) == LANDPOINT);
		bool islandable = tpc.IsLandable;
		bool isthermal = (tp.Style == STYLE_THERMAL);

		// always in range if MapScale <=10
		bool irange = inrange;

		if(MapWindow::zoom.RealScale() > 20) {
			irange=false;
			goto NiklausWirth; // with compliments
		}
		if (decluttericons) {
			if (! (tpc.IsAirport && (tp.RunwayLen>minrunway || tp.RunwayLen==0))) {
				irange=false;
				goto NiklausWirth;
			}
		}

		if(islandable && tp.Reachable) {

			TextDisplayMode.Reachable = 1;
			if ((GetMultimap_Labels()<MAPLABELS_ALLOFF)||intask) {
				dowrite = true;
				// exclude outlandings worst than visible airports, only when there are visible reachable airports!
				if ( isairport==false && islandpoint==true ) {
					if ( (int)tp.AltArivalAGL >=2000 ) { // more filter
						excluded=true;
					} else {
						if ( (bestwp>=0) && (idx==(unsigned)bestwp) && (foundairport==0) ) { // this outlanding is the best option
							isairport=true;
							islandpoint=false; // make it an airport TODO paint it as best
						} else {
							if ( foundairport >0 ) {
								if ( (int)tp.AltArivalAGL <= arrivalcutoff ) {
									excluded=true;
								}
							}
						}
					}
				} else {
					// do not display airport arrival if close to the best so far.
					// ex: best arrival is 1200m, include onlye below 1200/4  (prevent division by zero)
					// This way we only display far points, and skip closer points
					// WE NEED MORE INFO ABOUT LANDING POINTS: THE .CUP FORMAT WILL LET US KNOW WHAT IS
					// BEST TO SHOW AND WHAT IS NOT. Winpilot format is useless here.
					dowrite=true;// TEST FIX not necessary probably
					// it's an airport
					if ( (bestwp>=0) && (idx != (unsigned)bestwp) && (arrivalcutoff>600) ) {
						if ( (arrivalcutoff / ((int)tp.AltArivalAGL+1))<4 ) {
							excluded=true;
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
				if ( isairport ) {
					excluded=false; //  show all airports, declutter outlandings
				}
				break;
			default:
				break; // else work normally
		}

		if(idx==RESWP_OPTIMIZED) {
			dowrite = DoOptimizeRoute();
		}

		// here come both turnpoints and landables..
		if( intask || irange || dowrite) {  // irange always set when MapScale <=10

			bool draw_alt = TextDisplayMode.Reachable && ((GetMultimap_Labels()<MAPLABELS_ONLYTOPO) || intask); // 100711 reachable landing point!
			if (excluded==true) {
				draw_alt=false; // exclude close outlandings
			}

			dowrite = (GetMultimap_Labels()<MAPLABELS_ONLYTOPO) || intask || islandable;  // 100711
			if ( (islandable && !isairport) && MapWindow::zoom.RealScale() >=10 ) {
				dowrite = false; // FIX then no need to go further
			}

			if(DisplayTextType == DISPLAYNAMEIFINTASK) {
				dowrite = intask;
			}

			if(DisplayTextType == DISPLAYNONE) {
				Buffer2[0] = _T('\0'); // 
			} else if (DisplayTextType == DISPLAYICAO) {
				_tcscpy(Buffer2,tp.Code);
			} else if (DisplayTextType == DISPLAYNUMBER) {
				_stprintf(Buffer2, _T("%d"),tp.Number);
			} else if(intask || (DisplayTextType != DISPLAYNAMEIFINTASK)) {
				CopyTruncateString(Buffer2, array_size(Buffer2), tp.Name, label_resizer[DisplayTextType]);
			}

			if (draw_alt) {
				switch ( ArrivalValue ) {
					default:
					case avNone :
						_tcscpy(Buffer,Buffer2);
					break;

					case avGR :
						_stprintf(Buffer, TEXT("%s:%d"), Buffer2, (int)tpc.GR);
					break;

					case avAltitude :
						_stprintf(Buffer, TEXT("%s:%d%s"), Buffer2, (int)(tp.AltArivalAGL*ALTITUDEMODIFY), sAltUnit);
					break;

					case avGR_Altitude :
						_stprintf(Buffer, TEXT("%s:%d/%d%s"), Buffer2, (int)tpc.GR, (int)(tp.AltArivalAGL*ALTITUDEMODIFY), sAltUnit);
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
				_tcscpy(Buffer,Buffer2);
				if (islandable && isairport) {
					TextDisplayMode.WhiteBold = 1; // outlined
					TextDisplayMode.Color=RGB_WHITE;
				}
			}
		} // end intask/irange/dowrite

		// Do not show takeoff for gliders, check TakeOffWayPoint
		if (idx==RESWP_TAKEOFF) {
			if (TakeOffWayPoint) {
				intask=false; // 091031 let TAKEOFF be decluttered
			} else {
				dowrite=false;
			}
		}

		if (dowrite) {
			RasterPoint LabelPos =  _Proj.ToRasterPoint(tp.Latitude, tp.Longitude);
			LabelPos.x += 5;

			if (PtInRect(&ClipRect, LabelPos)) {

				MapWaypointLabelAdd(
						Buffer,
						LabelPos,
						&TextDisplayMode,
						(int) (tp.AltArivalAGL * ALTITUDEMODIFY),
						intask, islandable, isthermal, idx, tp.Style);
			}
		}
	} // for all waypoints

	auto sorted_array = make_array(SortedWaypointLabelList, MapWaypointLabelListCount);
	std::sort(std::begin(sorted_array), std::end(sorted_array), MapWaypointLabelListCompare());

	for (MapWaypointLabel_t *E : sorted_array) {

		if (!TextInBox(Surface, &rc, E->Name, E->Pos.x, E->Pos.y+NIBLSCALE(1), &(E->Mode), true)) {
			continue;
		}
		if(E->isLandable) {
			continue; // don't draw icon, already done in previous loop...
		}
		// If we are at low zoom, use a dot for icons, so we dont clutter the screen
		if(!Appearance.UTF8Pictorials) {
			const LKIcon* pWptBmp = nullptr;
			if(MapWindow::zoom.RealScale() > 4) {
				pWptBmp = (BlackScreen) ? &hInvSmall : &hSmall;
			} else if(E->isThermal) {
				pWptBmp = (E->AltArivalAGL > 0) ? &hLKThermal : &hLKThermalRed;
			} else {
				pWptBmp = &get_turnpoint_icon(E->style);
			}

			if(pWptBmp) {
				const unsigned IconSize = (UseHiresBitmap ? IBLSCALE(10) : 20);
				pWptBmp->Draw(Surface, E->Pos.x - IconSize/2, E->Pos.y - IconSize/2, IconSize, IconSize);
			}
		}
		else {
			int ytext = Surface.GetTextHeight(_T("X"));
			RECT rctmp {
				E->Pos.x, 
				E->Pos.y - ytext, 
				E->Pos.x, 
				E->Pos.y
			};
			DrawMAPWaypointPictoUTF8( Surface, rctmp, E->style);
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

/**
 * Draw trunpoint pictogram centered in @rc
 */
void MapWindow::DrawWaypointPicto(LKSurface& Surface, const RECT& rc, const WAYPOINT* wp) {
    if(Appearance.UTF8Pictorials) {
		UTF8WaypointPictorial( Surface,  rc, wp);
    } else {

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

		const LKIcon& tp_icon = get_turnpoint_icon(wp->Style);
		tp_icon.Draw(Surface, x, y, cx ,cy);
	}
}
