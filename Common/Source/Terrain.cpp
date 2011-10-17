/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Terrain.cpp,v 8.6 2010/12/17 02:02:27 root Exp root $
*/

#include "externs.h"
#include "Terrain.h"
#include "RasterTerrain.h"
#include "STScreenBuffer.h"
#include "Process.h"
#include "Waypointparser.h"
#include "RGB.h"

using std::min;
using std::max;


unsigned short minalt=9999;

Topology* TopoStore[MAXTOPOLOGY];

#if USETOPOMARKS
TopologyWriter *topo_marks = NULL;
#endif

#define MINRANGE 0.2

#if USETOPOMARKS
bool reset_marks = false;
#endif

BYTE tshadow_r, tshadow_g, tshadow_b, tshadow_h;
BYTE thighlight_r, thighlight_g, thighlight_b, thighlight_h;

bool RectangleIsInside(rectObj r_exterior, rectObj r_interior) {
  if ((r_interior.minx >= r_exterior.minx)&&
      (r_interior.maxx <= r_exterior.maxx)&&
      (r_interior.miny >= r_exterior.miny)&&
      (r_interior.maxy <= r_exterior.maxy))    
    return true;
  else 
    return false;
}

void SetTopologyBounds(const RECT rcin, const bool force) {
  static rectObj bounds_active;
  static double range_active = 1.0;
  rectObj bounds_screen;
  (void)rcin;
  bounds_screen = MapWindow::CalculateScreenBounds(1.0);

  bool recompute = false;

  // only recalculate which shapes when bounds change significantly
  // need to have some trigger for this..

  // trigger if the border goes outside the stored area
  if (!RectangleIsInside(bounds_active, bounds_screen)) {
    recompute = true;
  }
  
  // also trigger if the scale has changed heaps
  double range_real = max((bounds_screen.maxx-bounds_screen.minx), 
			  (bounds_screen.maxy-bounds_screen.miny));
  double range = max(MINRANGE,range_real);

  double scale = range/range_active;
  if (max(scale, 1.0/scale)>4) {
    recompute = true;
  }

  if (recompute || force || LKSW_ForceNearestTopologyCalculation) {

    // make bounds bigger than screen
    if (range_real<MINRANGE) {
      scale = BORDERFACTOR*MINRANGE/range_real;
    } else {
      scale = BORDERFACTOR;
    }
    bounds_active = MapWindow::CalculateScreenBounds(scale);

    range_active = max((bounds_active.maxx-bounds_active.minx), 
		       (bounds_active.maxy-bounds_active.miny));
    
    for (int z=0; z<MAXTOPOLOGY; z++) {
      if (TopoStore[z]) {
	TopoStore[z]->triggerUpdateCache=true;          
      }
    }
    #if USETOPOMARKS
    if (topo_marks) {
      topo_marks->triggerUpdateCache = true;
    }
    #endif

    // now update visibility of objects in the map window
    MapWindow::ScanVisibility(&bounds_active);

  }

  // check if things have come into or out of scale limit
  for (int z=0; z<MAXTOPOLOGY; z++) {
    if (TopoStore[z]) {
      TopoStore[z]->TriggerIfScaleNowVisible();          
    }
  }

  // ok, now update the caches
  #if USETOPOMARKS 
  if (topo_marks) {
    topo_marks->updateCache(bounds_active);
  }
  #endif
  
  if (EnableTopology) {
    // check if any needs to have cache updates because wasnt 
    // visible previously when bounds moved
    bool sneaked= false;
    bool rta;

    // we will make sure we update at least one cache per call
    // to make sure eventually everything gets refreshed

    int total_shapes_visible = 0;
    for (int z=0; z<MAXTOPOLOGY; z++) {
      if (TopoStore[z]) {
	rta = MapWindow::RenderTimeAvailable() || force || !sneaked;
	if (TopoStore[z]->triggerUpdateCache) {
	  sneaked = true;
	}
	TopoStore[z]->updateCache(bounds_active, !rta);
	total_shapes_visible += TopoStore[z]->shapes_visible_count;
      }
    }
#ifdef DEBUG_GRAPHICS
    DebugStore("%d # shapes\n", total_shapes_visible);
#endif

  }
}

#if USETOPOMARKS
// inititalise shapes for markers, not the text file surviving restarts
void TopologyInitialiseMarks() {

  StartupStore(TEXT(". Initialise marks%s"),NEWLINE);

  LockTerrainDataGraphics();
	  
  // TODO code: - This convert to non-unicode will not support all languages
  //		(some may use more complicated PATH names, containing Unicode)
  //  char buffer[MAX_PATH];
  //  ConvertTToC(buffer, LocalPath(TEXT("xcsoar-marks")));
  // DISABLED LocalPath
  // JMW localpath does NOT work for the shapefile renderer!
  if (topo_marks) {
    topo_marks->DeleteFiles();
    delete topo_marks;
  }

  TCHAR buf[MAX_PATH];
  LocalPath(buf, _T(LKD_CONF));
  _tcscat(buf, _T("\\")); _tcscat(buf, _T(LKF_SMARKS)); 
  topo_marks = new TopologyWriter(buf, RGB(0xD0,0xD0,0xD0)); 
  if (topo_marks) {
    topo_marks->scaleThreshold = 30.0;
    //topo_marks->scaleDefaultThreshold = 30.0;	// 101212
    topo_marks->scaleCategory = 0;		// 101212 marked locations
    topo_marks->loadBitmap(IDB_MARK);
  }
  UnlockTerrainDataGraphics();
}
#endif

void CloseTopology() {
  StartupStore(TEXT(". CloseTopology%s"),NEWLINE);

  LockTerrainDataGraphics();
  for (int z=0; z<MAXTOPOLOGY; z++) {
    if (TopoStore[z]) {
      delete TopoStore[z];
    }
  }
  UnlockTerrainDataGraphics();
}

#if USETOPOMARKS
void TopologyCloseMarks() {
  StartupStore(TEXT(". CloseMarks%s"),NEWLINE);
  LockTerrainDataGraphics();
  if (topo_marks) {
    topo_marks->DeleteFiles();
    delete topo_marks;
    topo_marks = NULL;
  }
  UnlockTerrainDataGraphics();
}
#endif


extern void LatitudeToCUPString(double Latitude, TCHAR *Buffer);
extern void LongitudeToCUPString(double Latitude, TCHAR *Buffer);

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
		marktime,GPS_INFO.Second,slat,slon, iround((int)CALCULATED_INFO.NavAltitude),
		GPS_INFO.Day,GPS_INFO.Month,GPS_INFO.Year, marktime, snear );

	fwrite(message,strlen(message),1,stream);
	fclose(stream);

extern int GetVirtualWaypointMarkerSlot(void);

	j=GetVirtualWaypointMarkerSlot();

	WayPointList[j].Latitude=lat;
	WayPointList[j].Longitude=lon;
	WayPointList[j].Altitude=CALCULATED_INFO.NavAltitude;
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

#if USETOPOMARKS
void DrawMarks (const HDC hdc, const RECT rc)
{

  LockTerrainDataGraphics();
  if (topo_marks) {
	if (reset_marks) {
		topo_marks->Reset();
		reset_marks = false;
	}
	topo_marks->Paint(hdc, rc);
  }
  UnlockTerrainDataGraphics();

}
#endif


void DrawTopology(const HDC hdc, const RECT rc)
{
  static double lastForceNearest=0;

  LockTerrainDataGraphics();

  // Ok I explain what is the logic here. We want the nearest, which is triggered by dlgOracle.
  // The dlgOracle function is zoomin in to 2km scale and this is causing reload of topology
  // up to very small cities details without including far away points.
  // At the same time, it is setting ForceNearestTopologyCalculation to true.
  // The oracle page is over the map now, and we dont paint topology: we calculate nearest instead.
  // Then we clear the flag, so we do the calculation only once.
  // The Oracle checks that we have cleared ForceNearestTopologyCalculation here, and that means
  // that data is available from SearchNearest.
  if (LKSW_ForceNearestTopologyCalculation) {
	if (lastForceNearest==0) {
		lastForceNearest=GPS_INFO.Time;
		goto _exit;
	}
	// We must wait for a few seconds to be sure cache was updated
	if ( (GPS_INFO.Time - lastForceNearest)<2) goto _exit;
	for (int z=0; z<MAXTOPOLOGY; z++) {
		if (TopoStore[z]) {
			// See also CheckScale for category checks! We should use a function in fact.
			if ( TopoStore[z]->scaleCategory == 10 ||
			     (TopoStore[z]->scaleCategory >= 70 && TopoStore[z]->scaleCategory <=100))
				TopoStore[z]->SearchNearest(rc); 
		}
	}
	LKSW_ForceNearestTopologyCalculation=false; // Done, the Oracle can compute now.
  } else {
	lastForceNearest=0;
	for (int z=0; z<MAXTOPOLOGY; z++) {
		if (TopoStore[z]) {
			TopoStore[z]->Paint(hdc,rc);
		}
	}
  }
_exit:
  UnlockTerrainDataGraphics();

}


double ReadZoomTopology(int iCategory) {

  for (int z=0; z<MAXTOPOLOGY; z++) {
	if (TopoStore[z]) { 
		if ( TopoStore[z]->scaleCategory == iCategory ) {
			return (TopoStore[z]->scaleThreshold);
		}
	}
  }
  return 0.0;
}

bool HaveZoomTopology(int iCategory) {

  for (int z=0; z<MAXTOPOLOGY; z++) {
	if (TopoStore[z]) { 
		if ( TopoStore[z]->scaleCategory == iCategory ) {
			return true;
		}
	}
  }
  return false;
}

// mode: 0 normal Change the topology scale of Category with newScale.
// mode: 1 zoom more or less for the category with newScale (newScale is the zoom increment).
// mode: 2 zoom more or less for all categories (tCategory is ignored, newScale is the zoom increment for all items)
// mode: 3 reset default zoom for Category (newScale is ignored)
// mode: 4 reset default zoom for all Categories (Category is ignored, newScale is ignored)
void ChangeZoomTopology(int iCategory, double newScale, short cztmode)
{
  if (LKTopo<1) return;

  LockTerrainDataGraphics();

  if (cztmode==0) {
	for (int z=0; z<MAXTOPOLOGY; z++) {
		if (TopoStore[z]) { 
			if ( TopoStore[z]->scaleCategory == iCategory ) {
				#if DEBUG_LKTOPO
				StartupStore(_T("... ChangeZoomTopology zindex=%d, categ=%d oldscale=%f newscale=%f%s"),z,iCategory,
				TopoStore[z]->scaleThreshold, newScale,NEWLINE);
				#endif
				TopoStore[z]->scaleThreshold = newScale;
			}
		}
	}
	UnlockTerrainDataGraphics();
	return;
  }

  if (cztmode==1) {
	for (int z=0; z<MAXTOPOLOGY; z++) {
		if (TopoStore[z]) { 
			if ( TopoStore[z]->scaleCategory == iCategory ) {
				#if DEBUG_LKTOPO
				StartupStore(_T("... ChangeZoomTopology: zindex=%d, categ=%d oldscale=%f increment=%f%s"),z,iCategory,
				TopoStore[z]->scaleThreshold, newScale,NEWLINE);
				#endif
				TopoStore[z]->scaleThreshold += newScale;
			}
		}
	}
	UnlockTerrainDataGraphics();
	return;
  }

  if (cztmode==2) {
	for (int z=0; z<MAXTOPOLOGY; z++) {
		if (TopoStore[z]) { 
			#if DEBUG_LKTOPO
			StartupStore(_T("... ChangeZoomTopology for all: zindex=%d, categ=%d oldscale=%f increment=%f%s"),z,iCategory,
			TopoStore[z]->scaleThreshold, newScale,NEWLINE);
			#endif
			TopoStore[z]->scaleThreshold += newScale;
		}
	}
	UnlockTerrainDataGraphics();
	return;
  }

  if (cztmode==3) {
	for (int z=0; z<MAXTOPOLOGY; z++) {
		if (TopoStore[z]) { 
			if ( TopoStore[z]->scaleCategory == iCategory ) {
				#if DEBUG_LKTOPO
				StartupStore(_T("... ChangeZoomTopology default zindex=%d, categ=%d oldscale=%f default=%f%s"),
				z,iCategory, TopoStore[z]->scaleThreshold, TopoStore[z]->scaleDefaultThreshold,NEWLINE);
				#endif
				TopoStore[z]->scaleThreshold = TopoStore[z]->scaleDefaultThreshold;
			}
		}
	}
	UnlockTerrainDataGraphics();
	return;
  }

  if (cztmode==4) {
	for (int z=0; z<MAXTOPOLOGY; z++) {
		if (TopoStore[z]) { 
			#if DEBUG_LKTOPO
			StartupStore(_T("... ChangeZoomTopology all default zindex=%d, categ=%d oldscale=%f default=%f%s"),
			z,iCategory, TopoStore[z]->scaleThreshold, TopoStore[z]->scaleDefaultThreshold,NEWLINE);
			#endif
			TopoStore[z]->scaleThreshold = TopoStore[z]->scaleDefaultThreshold;
		}
	}
	UnlockTerrainDataGraphics();
	return;
  }

  UnlockTerrainDataGraphics();
}

#define NUM_COLOR_RAMP_LEVELS 13


// If you change NUMRAMPS, also change the InputEvent SERVICE TERRCOL because I am lazy
#define NUMRAMPS	14

// terrain shadowing and highlight relative to type
// shadow to blue   is 0 0 64
// highl  to yellow is 255 255 16
const COLORRAMP terrain_shadow[NUMRAMPS] = {
  // { 63, 0, 0, 64}, // 101016 low lands, no blue shading
  { 63, 60, 10, 10},
  // { 63, 0, 0, 64}, // 101016 mountain , no blue shading
  { 63, 60, 10, 10},
  { 63, 0, 0, 64},
  { 63, 0, 0, 64},
  { 63, 0, 0, 64},
  { 63, 0, 0, 64},
  { 63, 0, 0, 64},
  { 63, 16, 32, 32},
  { 63, 16, 32, 32},
  { 63, 16, 32, 32},
  { 63, 16, 32, 32},
  { 63, 16, 32, 32},
  { 63, 60,60, 60},
  { 63, 60,60, 60}
  //{ 63, 16, 32, 32}
};
const COLORRAMP terrain_highlight[NUMRAMPS] = {
  // { 32, 255, 255, 16}, // 101016 low lands, no highlight
  { 255, 0,0,0},
  // { 32, 255, 255, 16}, // 101016 CHANGED for mountain, no highlight too yellowing
  { 255, 0,0,0},
  { 255, 0,0,0},
  { 255, 0,0,0},
  { 255, 0,0,0},
  { 255, 0,0,0},
  { 255, 0,0,0},
  // ^^ 32 255 255 16 originally
  // { 32, 220, 255, 220}, // 101016 no highlight for lkoogle etc.
  { 255, 0,0,0},
  { 255, 0,0,0},
  { 255, 0,0,0},
  { 255, 0,0,0},
  { 255, 0,0,0},
  // { 32, 220, 255, 220}  // 101016 high alps no highlight
  { 63, 250, 250, 250},
  { 255, 0,0,0}
};

// Use shading for terrain modes
const bool terrain_doshading[NUMRAMPS] = {
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,	// YouSee Shaded
	1, 	// YouSee HiContrast
	0	// Obstacles
};
// Use minimal altitude normalizer for terrain modes
const bool terrain_minalt[NUMRAMPS] = {
	1,
	1,
	1,	// Imhof 7
	1,	// IMhof 4
	1,	// Imhof 12
	1,	// Atlas
	1,	// ICAO
	1,
	1,
	1,
	1,
	0,	// YouSee Default
	1, 	// YouSee HiContrast
	1 	// Obstacles
};

const COLORRAMP terrain_colors[NUMRAMPS][NUM_COLOR_RAMP_LEVELS] = { 
  {
    {0,           0x70, 0xc0, 0xa7},
    {250,         0xca, 0xe7, 0xb9},
    {500,         0xf4, 0xea, 0xaf},
    {750,         0xdc, 0xb2, 0x82},
    {1000,        0xca, 0x8e, 0x72},
    {1250,        0xde, 0xc8, 0xbd},
    {1500,        0xe3, 0xe4, 0xe9},
    {1750,        0xdb, 0xd9, 0xef},
    {2000,        0xce, 0xcd, 0xf5},
    {2250,        0xc2, 0xc1, 0xfa},
    {2500,        0xb7, 0xb9, 0xff},
    {5000,        0xb7, 0xb9, 0xff},
    {6000,        0xb7, 0xb9, 0xff}
  },
  // old mountain
  /*
  {
    {0,           0x70, 0xc0, 0xa7},
    {500,         0xca, 0xe7, 0xb9},
    {1000,        0xf4, 0xea, 0xaf},
    {1500,        0xdc, 0xb2, 0x82},
    {2000,        0xca, 0x8e, 0x72},
    {2500,        0xde, 0xc8, 0xbd},
    {3000,        0xe3, 0xe4, 0xe9},
    {3500,        0xdb, 0xd9, 0xef},
    {4000,        0xce, 0xcd, 0xf5},
    {4500,        0xc2, 0xc1, 0xfa},
    {5000,        0xb7, 0xb9, 0xff},
    {6000,        0xb7, 0xb9, 0xff},
    {7000,        0xb7, 0xb9, 0xff}
  },
  */
  // LK Mountain 101016
  {
    {0,           0x70, 0xc0, 0xa7},
    {350,         0xca, 0xe7, 0xb9},
    {700,        0xf4, 0xea, 0xaf},
    {1050,        0xdc, 0xb2, 0x82},
    {1400,        0xd1, 0x9a, 0x5c},
    {1750,        0xca, 0x8e, 0x72},
    {2100,        0x9b, 0x59, 0x3b},
    {2450,        0xde, 0xc8, 0xbd},
    {2800,        0xe3, 0xe4, 0xe9},
    {3150,        0xdb, 0xd9, 0xef},
    {3500,        0xce, 0xcd, 0xf5},
    {3850,        0xb7, 0xb9, 0xff},
    {7000,        0xff, 0xff, 0xff}
  },
  { // Imhof Type 7, geomteric 1.35 9
    {0,    153, 178, 169},
    {368,  180, 205, 181},
    {496,  225, 233, 192},
    {670,  255, 249, 196},
    {905,  255, 249, 196},
    {1222, 255, 219, 173},
    {1650, 254, 170, 136},
    {2227, 253, 107, 100},
    {3007, 255, 255, 255},
    {5000, 255, 255, 255},
    {6000, 255, 255, 255},
    {7000, 255, 255, 255},
    {8000, 255, 255, 255}
  },
  { // Imhof Type 4, geomteric 1.5 8
    {0,    175, 224, 203},
    {264,  211, 237, 211},
    {396,  254, 254, 234},
    {594,  252, 243, 210},
    {891,  237, 221, 195},
    {1336, 221, 199, 175},
    {2004, 215, 170, 148},
    {3007, 255, 255, 255},
    {4000, 255, 255, 255},
    {5000, 255, 255, 255},
    {6000, 255, 255, 255},
    {7000, 255, 255, 255},
    {8000, 255, 255, 255}
  },
  { // Imhof Type 12, geomteric  1.5 8
    {0,    165, 220, 201},
    {399,  219, 239, 212},
    {558,  254, 253, 230},
    {782,  254, 247, 211},
    {1094,  254, 237, 202},
    {1532, 254, 226, 207},
    {2145, 254, 209, 204},
    {3004, 255, 255, 255},
    {4000, 255, 255, 255},
    {5000, 255, 255, 255},
    {6000, 255, 255, 255},
    {7000, 255, 255, 255},
    {8000, 255, 255, 255}
  },
  { // Imhof Atlas der Schweiz
    {0,     47, 101, 147},
    {368,   58, 129, 152},
    {496,  117, 148, 153},
    {670,  155, 178, 140},
    {905,  192, 190, 139},
    {1222, 215, 199, 137},
    {1650, 229, 203, 171},
    {2227, 246, 206, 171},
    {3007, 252, 246, 244},
    {5001, 252, 246, 244},
    {7000, 252, 246, 244},
    {8000, 252, 246, 244},
    {9000, 252, 246, 244}
  },
  { // ICAO
    {0,           180, 205, 181},
    {199,         180, 205, 181},
    {200,         225, 233, 192},
    {499,         225, 233, 192},
    {500,         255, 249, 196},
    {999,         255, 249, 196},
    {1000,        255, 219, 173},
    {1499,        255, 219, 173},
    {1500,        254, 170, 136},
    {1999,        254, 170, 136},
    {2000,        253, 107, 100},
    {2499,        253, 107, 100},
    {2500,        255, 255, 255} 
  },
  { // LKoogle lowlands
    {0,           222, 226, 203},
    {250,         222, 226, 203},
    {500,         180, 180, 180},
    {750,         170, 170, 170},
    {1000,        160, 160, 160},
    {1250,        140, 140, 140},
    {1500,        130, 130, 130},
    {1750,        120, 120, 120},
    {2000,        190, 190, 190},
    {2250,        215, 215, 215},
    {2500,        240, 240, 240},
    {3000,        240, 240, 240},
    {4000,        240, 240, 240},
  },
  { // LKoogle mountains
    {0,           222, 226, 203},
    {500,         222, 226, 203},
    {1000,        180, 180, 180},
    {1500,        160, 160, 160},
    {2000,        140, 140, 140},
    {2500,        120, 120, 120},
    {3000,        190, 190, 190},
    {3500,        215, 215, 215},
    {4000,        240, 240, 240},
    {4500,        240, 240, 240},
    {5000,        240, 240, 240},
    {6000,        240, 240, 240},
    {7000,        240, 240, 240}
  },
  { // Low Alps
    {0,           0x70, 0xc0, 0xa7},
    {250,         0xca, 0xe7, 0xb9},
    {500,         0xf4, 0xea, 0xaf},
    {750,         0xdc, 0xb2, 0x82},
    {1000,        0xca, 0x8e, 0x72},
    {1250,        180, 180, 180},
    {1500,        160, 160, 160},
    {1750,        150, 150, 150},
    {2000,        140, 140, 140},
    {2250,        130, 130, 130},
    {2500,        200, 200, 200},
    {3000,        220, 220, 220},
    {4000,        240, 240, 240},
  },
  { // Alps
    {150,         0x70, 0xc0, 0xa7},
    {350,         0xca, 0xe7, 0xb9},
    {500,         0xca, 0xe7, 0xb9},
    {650,         0xf4, 0xea, 0xaf},
    {800,         0xf4, 0xea, 0xaf},
    {950,         0xdc, 0xb2, 0x82},
    {1100,        0xca, 0x8e, 0x72},
    {1500,        175, 175, 175},
    {1750,        165, 165, 165},
    {2000,        155, 155, 155},
    {2500,        235, 235, 235},
    {3000,        245, 245, 245},
    {4000,        255, 255, 255},
  },
  { // YouSee
    {0,         112,191,170},
    {800,         254,255,188},
    {1900,         194,135,93},
    {2900,         230,230,228},
    {4900,         186,185,251},
    {6000,         255,255,255},
    {6000,         255,255,255},
    {6000,         255,255,255},
    {6000,         255,255,255},
    {6000,         255,255,255},
    {6000,         255,255,255},
    {6000,         255,255,255},
    {6000,         255,255,255},
  },
/*
  { // YouSee High Contrast
    {0,       204,223,191 },
    {300,     102,175,96 },
    {500,     2,127,0  },
    {1000,    255,254,0},
    {1200,    225,192,0},
    {1800,    124,3,0},
    {3100,    255,255,253},
    {4900,    160,191,237},
    {6000,         255,255,255},
    {6000,         255,255,255},
    {6000,         255,255,255},
    {6000,         255,255,255},
    {6000,         255,255,255},
  } 
*/
  {
    {0,       235,255,235 },
    {100,     197,216,246 },
    {200,        0,170,0  },
    {300,        0,128,0  },
    {400,       0,85,0},
    {500,    51,119,0},
    {700,    153,187,0},
    {900,    255,255,0},
    {1000,    241,227,0},
    {1300,    199,142,0},
    {1800,    128,0,0},
    {3100,    255,255,255},
    {4900,    160,191,237},
  },

  {	// Obstacles
    {0,       227,255,224 },
    {50,      227,255,224 },
    {51,      255,255,0 },
    {120,     255,255,0 },
    {149,     255,100,50 },
    {150,     255,0,0 }, // 0m
    {300,     255,0,0},
    {500,    220,0,0},
    {700,    200,0,0},
    {900,    180,0,0},
    {1100,    150,0,0},
    {1300,    120,0,0},
    {3500,    100,0,0}
  }
};

void ColorRampLookup(const short h, 
                     BYTE &r, BYTE &g, BYTE &b,
		     const COLORRAMP* ramp_colors, 
                     const int numramp,
                     const unsigned char interp_levels) {

  unsigned short f, of;
  unsigned short is = 1<<interp_levels;

  /* Monochrome

#ifdef DEBUG
  r = 0xDA;
  g = 0xDA;
  b = 0xDA;
  return;
#endif

  */

  // gone past end, so use last color
  if (h>=ramp_colors[numramp-1].h) {
    r = ramp_colors[numramp-1].r;
    g = ramp_colors[numramp-1].g;
    b = ramp_colors[numramp-1].b;
    return;
  }
  for (unsigned int i=numramp-2; i--; ) {
    if (h>=ramp_colors[i].h) {
      f = (unsigned short)(h-ramp_colors[i].h)*is/
        (unsigned short)(ramp_colors[i+1].h-ramp_colors[i].h);
      of = is-f;
      
      r = (f*ramp_colors[i+1].r+of*ramp_colors[i].r) >> interp_levels;
      g = (f*ramp_colors[i+1].g+of*ramp_colors[i].g) >> interp_levels;
      b = (f*ramp_colors[i+1].b+of*ramp_colors[i].b) >> interp_levels;
      return;
    }
  }

  // check if h lower than lowest
  if (h<=ramp_colors[0].h) {
    r = ramp_colors[0].r;
    g = ramp_colors[0].g;
    b = ramp_colors[0].b;
    return;
  }
}


#define MIX(x,y,i) (BYTE)((x*i+y*((1<<7)-i))>>7)

inline void TerrainShading(const short illum, BYTE &r, BYTE &g, BYTE &b)
{
  char x;
  if (illum<0) {           // shadow to blue
    x = min((int)tshadow_h,-illum);
    r = MIX(tshadow_r,r,x);
    g = MIX(tshadow_g,g,x);
    b = MIX(tshadow_b,b,x);
  } else if (illum>0) {    // highlight to yellow
    if (thighlight_h == 255) return; // 101016
    x = min((int)thighlight_h,illum/2);
    r = MIX(thighlight_r,r,x);
    g = MIX(thighlight_g,g,x);
    b = MIX(thighlight_b,b,x);
  }
}


// map scale is approximately 2 points on the grid
// therefore, want one to one mapping if mapscale is 0.5
// there are approx 30 pixels in mapscale
// 240/DTQUANT resolution = 6 pixels per terrain
// (mapscale/30)  km/pixels
//        0.250   km/terrain
// (0.25*30/mapscale) pixels/terrain
//  mapscale/(0.25*30)
//  mapscale/7.5 terrain units/pixel
// 
// this is for TerrainInfo.StepSize = 0.0025;

extern DWORD misc_tick_count;

class TerrainRenderer {
public:
  TerrainRenderer(RECT rc) {

    if (!RasterTerrain::IsDirectAccess()) {
      dtquant = 6;
    } else {
	dtquant = 2; 
      // on my PDA (600MhZ, 320x240 screen):
      // dtquant=2, latency=170 ms
      // dtquant=3, latency=136 ms
      // dtquant=4, latency= 93 ms
    }
    blursize = max((unsigned int)0, (dtquant-1)/2);
    oversampling = max(1,(blursize+1)/2+1);
    if (blursize==0) {
      oversampling = 1; // no point in oversampling, just let stretchblt do the scaling
    }

    /*
      dtq  ovs  blur  res_x  res_y   sx  sy  terrain_loads  pixels
       1    1    0    320    240    320 240    76800        76800
       2    1    0    160    120    160 120    19200        19200
       3    2    1    213    160    107  80     8560        34080
       4    2    1    160    120     80  60     4800        19200
       5    3    2    192    144     64  48     3072        27648
    */


    #if (WINDOWSPC>0)
    // need at least 2Ghz singlecore CPU here for dtquant 1
    dtquant=2;
    #else
    // scale dtquant so resolution is not too high on large displays
    dtquant *= ScreenScale;  // lower resolution a bit.. (no need for CPU >800mHz)

    if (ScreenSize!=ss640x480)
	    if (dtquant>3) dtquant=3; // .. but not too much
    #endif

    int res_x = iround((rc.right-rc.left)*oversampling/dtquant);
    int res_y = iround((rc.bottom-rc.top)*oversampling/dtquant);

    sbuf = new CSTScreenBuffer();
    sbuf->Create(res_x, res_y, RGB_WHITE);
    ixs = sbuf->GetCorrectedWidth()/oversampling;
    iys = sbuf->GetHeight()/oversampling;

    hBuf = (unsigned short*)malloc(sizeof(unsigned short)*ixs*iys);

    #if 100303
    if (hBuf==NULL)  {
	StartupStore(_T("------ TerrainRenderer: malloc(%d) failed!%s"),sizeof(unsigned short)*ixs*iys, NEWLINE);
    } else {
	StartupStore(_T(". TerrainRenderer: malloc(%d) ok%s"),sizeof(unsigned short)*ixs*iys, NEWLINE);
    }
    #endif

    colorBuf = (BGRColor*)malloc(256*128*sizeof(BGRColor));

  }

  ~TerrainRenderer() {
    if (hBuf) free(hBuf);
    if (colorBuf) free(colorBuf);
    if (sbuf) delete sbuf;
  }

public:
  POINT spot_max_pt;
  POINT spot_min_pt;
  short spot_max_val;
  short spot_min_val;

private:

  unsigned int ixs, iys; // screen dimensions in coarse pixels
  unsigned int dtquant;
  unsigned int epx; // step size used for slope calculations

  RECT rect_visible;

  CSTScreenBuffer *sbuf;

  double pixelsize_d;

  int oversampling;
  int blursize;

  unsigned short *hBuf;
  BGRColor *colorBuf;
  bool do_shading;
  RasterMap *DisplayMap;
  bool is_terrain;
  int interp_levels;
  COLORRAMP* color_ramp;
  unsigned int height_scale;

public:
  bool SetMap() {
      interp_levels = 2;
      is_terrain = true;
      height_scale = 4;
      DisplayMap = RasterTerrain::TerrainMap;
      color_ramp = (COLORRAMP*)&terrain_colors[TerrainRamp][0];

    if (is_terrain) {
	do_shading = true;
    } else {
	do_shading = false;
    }

    if (DisplayMap) 
      return true;
    else 
      return false;

  }

  void SetShading() { 
	if (is_terrain && Shading && terrain_doshading[TerrainRamp])
		do_shading=true;
	else
		do_shading=false;
  }

  void Height() {

    double X, Y;
    int x, y; 
    int X0 = (unsigned int)(dtquant/2); 
    int Y0 = (unsigned int)(dtquant/2);
    int X1 = (unsigned int)(X0+dtquant*ixs);
    int Y1 = (unsigned int)(Y0+dtquant*iys);

    unsigned int rfact=1;

    if (MapWindow::zoom.BigZoom()) {
      MapWindow::zoom.BigZoom(false);
      if (!RasterTerrain::IsDirectAccess()) {
        // first time displaying this data, so do it at half resolution
        // to avoid too many cache misses
        rfact = 2;
      }
    }

    double pixelDX, pixelDY;

    x = (X0+X1)/2;
    y = (Y0+Y1)/2;
    MapWindow::Screen2LatLon(x, y, X, Y);
    double xmiddle = X;
    double ymiddle = Y;
    int dd = (int)lround(dtquant*rfact);

    x = (X0+X1)/2+dd;
    y = (Y0+Y1)/2;
    MapWindow::Screen2LatLon(x, y, X, Y);
    float Xrounding = (float)fabs(X-xmiddle);
    DistanceBearing(ymiddle, xmiddle, Y, X, &pixelDX, NULL);

    x = (X0+X1)/2;
    y = (Y0+Y1)/2+dd;
    MapWindow::Screen2LatLon(x, y, X, Y);
    float Yrounding = (float)fabs(Y-ymiddle);
    DistanceBearing(ymiddle, xmiddle, Y, X, &pixelDY, NULL);

    pixelsize_d = sqrt((pixelDX*pixelDX+pixelDY*pixelDY)/2.0);

    // OK, ready to start loading height

    DisplayMap->Lock();

    misc_tick_count = GetTickCount();

    // TODO code: not needed   RasterTerrain::SetCacheTime();

    // set resolution

    if (DisplayMap->IsDirectAccess()) {
      DisplayMap->SetFieldRounding(0,0);
    } else {
      DisplayMap->SetFieldRounding(Xrounding,Yrounding);
    }

    epx = DisplayMap->GetEffectivePixelSize(&pixelsize_d,
                                            ymiddle, xmiddle);

    // do not shade terrain when using high or low zoom
    if (epx> min(ixs,iys)/4) { 
      do_shading = false;
    } else {
      #if (WINDOWSPC>0)
      if (MapWindow::zoom.Scale()>18) do_shading=false;
      #else
      if (MapWindow::zoom.Scale()>11) do_shading=false;
      #endif
    }


    POINT orig = MapWindow::GetOrigScreen();
    rect_visible.left = max((long)MapWindow::MapRect.left, (long)(MapWindow::MapRect.left-(long)epx*dtquant))-orig.x;
    rect_visible.right = min((long)MapWindow::MapRect.right, (long)(MapWindow::MapRect.right+(long)epx*dtquant))-orig.x;
    rect_visible.top = max((long)MapWindow::MapRect.top, (long)(MapWindow::MapRect.top-(long)epx*dtquant))-orig.y;
    rect_visible.bottom = min((long)MapWindow::MapRect.bottom, (long)(MapWindow::MapRect.bottom+(long)epx*dtquant))-orig.y;

    FillHeightBuffer(X0-orig.x, Y0-orig.y, X1-orig.x, Y1-orig.y);

    DisplayMap->Unlock();

  }

void FillHeightBuffer(const int X0, const int Y0, const int X1, const int Y1) {
    // fill the buffer
  unsigned short* myhbuf = hBuf;
  #ifdef DEBUG
  unsigned short* hBufTop = hBuf+ixs*iys;
  #endif

  const double PanLatitude =  MapWindow::GetPanLatitude();
  const double PanLongitude = MapWindow::GetPanLongitude();
  const double InvDrawScale = MapWindow::GetInvDrawScale()/1024.0;
  const double DisplayAngle = MapWindow::GetDisplayAngle();

  const int cost = ifastcosine(DisplayAngle);
  const int sint = ifastsine(DisplayAngle);
  minalt=9999;
  for (int y = Y0; y<Y1; y+= dtquant) {
	int ycost = y*cost;
	int ysint = y*sint;
	for (int x = X0; x<X1; x+= dtquant, myhbuf++) {
		if ((x>= rect_visible.left) &&
			(x<= rect_visible.right) &&
			(y>= rect_visible.top) &&
			(y<= rect_visible.bottom)) {
			#ifdef DEBUG
			ASSERT(myhbuf<hBufTop);
			#endif

			double Y = PanLatitude - (ycost+x*sint)*InvDrawScale;
			double X = PanLongitude + (x*cost-ysint)*invfastcosine(Y)*InvDrawScale;

			// this is setting to 0 any negative terrain value and can be a problem for dutch people
			// myhbuf cannot load negative values!
			*myhbuf = max(0, (int)DisplayMap->GetField(Y,X));
			if (*myhbuf!=TERRAIN_INVALID) {
				// if (*myhbuf>maxalt) maxalt=*myhbuf;
				if (*myhbuf<minalt) minalt=*myhbuf;
			}
		} else {
			// invisible terrain
			*myhbuf = TERRAIN_INVALID;
		}

	}
  }
  if (!terrain_minalt[TerrainRamp]) minalt=0;	//@ 101110
  if (TerrainRamp==13) {
	if (!GPS_INFO.NAVWarning) {
		if (CALCULATED_INFO.Flying) {
			minalt=(unsigned short)GPS_INFO.Altitude-150; // 500ft
		} else {
			minalt=(unsigned short)GPS_INFO.Altitude+100; // 330ft
		}
	} else {
		minalt+=150;
	}
  }

  // StartupStore(_T("... MinAlt=%d MaxAlt=%d Multiplier=%.3f\n"),minalt,maxalt, (double)((double)maxalt/(double)(maxalt-minalt))); 
 
}

// JMW: if zoomed right in (e.g. one unit is larger than terrain
// grid), then increase the step size to be equal to the terrain
// grid for purposes of calculating slope, to avoid shading problems
// (gridding of display) This is why epx is used instead of 1
// previously.  for large zoom levels, epx=1

void Slope(const int sx, const int sy, const int sz) {

  const int iepx = (int)epx;
  const unsigned int cixs=ixs;

  const unsigned int ciys = iys;
  
  const unsigned int ixsepx = cixs*epx;
  const unsigned int ixsright = cixs-1-iepx;
  const unsigned int iysbottom = ciys-iepx;
  const int hscale = max(1,(int)(pixelsize_d)); 
  const int tc = TerrainContrast;
  unsigned short *thBuf = hBuf;

  const BGRColor* oColorBuf = colorBuf+64*256;
  BGRColor* imageBuf = sbuf->GetBuffer();
  if (!imageBuf) return;

  unsigned short h;

  #ifdef DEBUG
  unsigned short* hBufTop = hBuf+cixs*ciys;
  #endif

  for (unsigned int y = 0; y< iys; y++) {
	const int itss_y = ciys-1-y;
	const int itss_y_ixs = itss_y*cixs;
	const int yixs = y*cixs;
	bool ybottom=false;
	bool ytop=false;
	int p31, p32, p31s;

	if (y<iysbottom) {
		p31= iepx;
		ybottom = true;
	} else {
		p31= itss_y;
	}

	if (y >= (unsigned int) iepx) {
		p31+= iepx;
	} else {
		p31+= y;
		ytop = true;
	}
	p31s = p31*hscale;

	for (unsigned int x = 0 ; x<cixs; x++, thBuf++, imageBuf++) {

		#ifdef DEBUG
		ASSERT(thBuf< hBufTop);
		#endif

		// FIX here Netherland dutch terrain problem
		// if >=0 then the sea disappears...
		if ((h = *thBuf) != TERRAIN_INVALID ) { 
			// if (h==0 && LKWaterThreshold==0) { // no LKM coasts, and water altitude
			if (h==LKWaterThreshold) { // see above.. h cannot be -1000.. so only when LKW is 0 h can be equal
				*imageBuf = BGRColor(85,160,255); // set water color 
				continue;
			}
			h=h-minalt+1;

			int p20, p22;

			h = min(255, h>>height_scale);
			// no need to calculate slope if undefined height or sea level

			if (do_shading) {
				if (x<ixsright) {
					p20= iepx;
					p22= *(thBuf+iepx);
					#ifdef DEBUG
					ASSERT(thBuf+iepx< hBufTop);
					#endif
				} else {
					int itss_x = cixs-x-2;
					p20= itss_x;
					p22= *(thBuf+itss_x);
					#ifdef DEBUG
					ASSERT(thBuf+itss_x< hBufTop);
					ASSERT(thBuf+itss_x>= hBuf);
					#endif
				} 
            
				if (x >= (unsigned int)iepx) {
					p20+= iepx;
					p22-= *(thBuf-iepx);
					#ifdef DEBUG
					ASSERT(thBuf-iepx>= hBuf); 
					#endif
				} else {
					p20+= x;
					p22-= *(thBuf-x);
					#ifdef DEBUG
					ASSERT(thBuf-x>= hBuf);
					#endif
				}
            
				if (ybottom) {
					p32 = *(thBuf+ixsepx);
					#ifdef DEBUG
					ASSERT(thBuf+ixsepx<hBufTop);
					#endif
				} else {
					p32 = *(thBuf+itss_y_ixs);
					#ifdef DEBUG
					ASSERT(thBuf+itss_y_ixs<hBufTop);
					#endif
				}

				if (ytop) {
					p32 -= *(thBuf-yixs);
					#ifdef DEBUG
					ASSERT(thBuf-yixs>=hBuf);
					#endif
				} else {
					p32 -= *(thBuf-ixsepx);
					#ifdef DEBUG
					ASSERT(thBuf-ixsepx>=hBuf);
					#endif
				}
            
				if ((p22==0) && (p32==0)) {

					// slope is zero, so just look up the color
					*imageBuf = oColorBuf[h]; 

				} else {

					// p20 and p31 are never 0... so only p22 or p32 can be zero
					// if both are zero, the vector is 0,0,1 so there is no need
					// to normalise the vector
					int dd0 = p22*p31;
					int dd1 = p20*p32;
					int dd2 = p20*p31s;
              
					while (dd2>512) {
						// prevent overflow of magnitude calculation
						dd0 /= 2;
						dd1 /= 2;
						dd2 /= 2;
					}
					int mag = (dd0*dd0+dd1*dd1+dd2*dd2);
					if (mag>0) {
						mag = (dd2*sz+dd0*sx+dd1*sy)/isqrt4(mag);
						mag = max(-64,min(63,(mag-sz)*tc/128));
						*imageBuf = oColorBuf[h+mag*256];
					} else {
						*imageBuf = oColorBuf[h];
					}
				}
			} else {
				// not using shading, so just look up the color
				*imageBuf = oColorBuf[h];
			}
		} else {
			// old: we're in the water, so look up the color for water
			// new: h is TERRAIN_INVALID here
			*imageBuf = oColorBuf[255];
		}
	} // for
  } // for
};



void ColorTable() {
  static COLORRAMP* lastColorRamp = NULL;
  if (color_ramp == lastColorRamp) {
	// no need to update the color table
	return;
  }
  lastColorRamp = color_ramp;

  for (int i=0; i<256; i++) {
	for (int mag= -64; mag<64; mag++) {
		BYTE r, g, b; 
		// i=255 means TERRAIN_INVALID. Water is colored in Slope
		if (i == 255) {
			colorBuf[i+(mag+64)*256] = BGRColor(194,223,197); // LCD green terrain invalid
		} else {
			// height_scale, color_ramp interp_levels  used only for weather
			// ColorRampLookup is preparing terrain color to pass to TerrainShading for mixing

			ColorRampLookup(i<<height_scale, r, g, b, color_ramp, NUM_COLOR_RAMP_LEVELS, interp_levels);
			if (do_shading) TerrainShading(mag, r, g, b);
			colorBuf[i+(mag+64)*256] = BGRColor(r,g,b);
		}
	}
  }
}

  void Draw(HDC hdc, RECT rc) {

    sbuf->Zoom(oversampling);

    if (blursize>0) {

      sbuf->HorizontalBlur(blursize); 
      sbuf->VerticalBlur(blursize);

    }
    sbuf->DrawStretch(&hdc, rc);

  }

};


TerrainRenderer *trenderer = NULL;

int Performance = 0;

void CloseTerrainRenderer() {
  if (trenderer) {
    delete trenderer;
  }
}



void DrawTerrain( const HDC hdc, const RECT rc, 
                  const double sunazimuth, const double sunelevation)
{
  (void)sunelevation; // TODO feature: sun-based rendering option
  (void)rc;

  if (!RasterTerrain::isTerrainLoaded()) {
    return;
  }

  if (!trenderer) {
    trenderer = new TerrainRenderer(MapWindow::MapRect);
  }

  if (!trenderer->SetMap()) {
    return;
  }

  // load terrain shading parameters
  // Make them instead dynamically calculated based on previous average terrain illumination
  tshadow_r= terrain_shadow[TerrainRamp].r;
  tshadow_g= terrain_shadow[TerrainRamp].g;
  tshadow_b= terrain_shadow[TerrainRamp].b;
  tshadow_h= terrain_shadow[TerrainRamp].h;

  thighlight_r= terrain_highlight[TerrainRamp].r;
  thighlight_g= terrain_highlight[TerrainRamp].g;
  thighlight_b= terrain_highlight[TerrainRamp].b;
  thighlight_h= terrain_highlight[TerrainRamp].h;

  // step 1: calculate sunlight vector
  int sx, sy, sz;
  double fudgeelevation = (10.0+80.0*TerrainBrightness/255.0);

  sx = (int)(255*(fastcosine(fudgeelevation)*fastsine(sunazimuth)));
  sy = (int)(255*(fastcosine(fudgeelevation)*fastcosine(sunazimuth)));
  sz = (int)(255*fastsine(fudgeelevation));

  trenderer->SetShading();
  trenderer->ColorTable();
  // step 2: fill height buffer

  trenderer->Height(); 

  // step 3: calculate derivatives of height buffer
  // step 4: calculate illumination and colors
  trenderer->Slope(sx, sy, sz); 
  
  // step 5: draw
  trenderer->Draw(hdc, MapWindow::MapRect);

  misc_tick_count = GetTickCount()-misc_tick_count;
}


#include "wcecompat/ts_string.h"
// TODO code: check ts_string does the right thing

void OpenTopology() {
  StartupStore(TEXT(". OpenTopology%s"),NEWLINE);
  CreateProgressDialog(gettext(TEXT("_@M902_"))); // Loading Topology File...

  // Start off by getting the names and paths
  static TCHAR szOrigFile[MAX_PATH] = TEXT("\0");
  static TCHAR szFile[MAX_PATH] = TEXT("\0");
  static TCHAR Directory[MAX_PATH] = TEXT("\0");

  LKTopo=0;

  LockTerrainDataGraphics();

  for (int z=0; z<MAXTOPOLOGY; z++) {
    TopoStore[z] = 0;
  }
 
  GetRegistryString(szRegistryTopologyFile, szFile, MAX_PATH);
  ExpandLocalPath(szFile);
  _tcscpy(szOrigFile,szFile); // make copy of original
  ContractLocalPath(szOrigFile);

  // remove it in case it causes a crash (will restore later)
  SetRegistryString(szRegistryTopologyFile, TEXT("\0"));

  if (1) {

    // file is blank, so look for it in a map file
    static TCHAR  szMapFile[MAX_PATH] = TEXT("\0");
    GetRegistryString(szRegistryMapFile, szMapFile, MAX_PATH);
    if (_tcslen(szMapFile)==0) {
      UnlockTerrainDataGraphics();
      return;
    }
    ExpandLocalPath(szMapFile);

    // Look for the file within the map zip file...
    _tcscpy(Directory,szMapFile);
    _tcscat(Directory,TEXT("/"));
    szFile[0]=0;
    _tcscat(szFile,Directory);
    _tcscat(szFile,TEXT("topology.tpl"));

  } else {
    ExtractDirectory(Directory,szFile);
  }

  // Ready to open the file now..
  ZZIP_FILE* zFile = zzip_fopen(szFile, "rt");
  
  if (!zFile) {
    UnlockTerrainDataGraphics();
    StartupStore(TEXT(". No topology file <%s>%s"), szFile,NEWLINE);
    return;
  }

  TCHAR ctemp[80];
  TCHAR TempString[READLINE_LENGTH+1];
  TCHAR ShapeName[50];
  double ShapeRange;
  long ShapeIcon;
  long ShapeField;
  TCHAR wShapeFilename[MAX_PATH];
  TCHAR *Stop;
  int numtopo = 0;
  int shapeIndex=0;
  LKWaterThreshold=0;

  while(ReadString(zFile,READLINE_LENGTH,TempString)) {
      
    if(_tcslen(TempString) > 0 && _tcsstr(TempString,TEXT("*")) != TempString) // Look For Comment
      {
        
        BYTE red, green, blue;
        // filename,range,icon,field
        
        // File name
        PExtractParameter(TempString, ctemp, 0);
        _tcscpy(ShapeName, ctemp);
        
        _tcscpy(wShapeFilename, Directory);

        _tcscat(wShapeFilename,ShapeName);
        _tcscat(wShapeFilename,TEXT(".shp"));
        
        // Shape range
        PExtractParameter(TempString, ctemp, 1);
        ShapeRange = StrToDouble(ctemp,NULL);

	// Normally ShapeRange is indicating km threshold for items to be drawn.
	// If over 5000, we identify an LKmap topology and subtract 5000 to get the type.
	// 
	// SCALE CATEGORIES scaleCategory
	// 0 is reserved
	// 1 is marked locations
	// 05 for coast areas
	// 10 for water area
	// 20 for water line
	// 30 for big road
	// 40 for medium road
	// 50 for small road
	// 60 for railroad
	// 70 for big city
	// 80 for medium city
	// 90 for small city
	// 100 for very small city
	// 110 for city area polyline with no name
	if ( ShapeRange>5000 && ShapeRange<6000 ) {
		shapeIndex=(int)ShapeRange-5000;
		// Load default values
		switch(shapeIndex) {
			case 5:
				// Coast Area
				ShapeRange=100;
				// Below (<=) this values, it is water painted blue
				LKWaterThreshold=-1000;
				break;
			case 10:
				// Water Area
				ShapeRange=100;
				break;
			case 20:
				// Water line
				ShapeRange=7;
				break;
			case 30:
				// Big road
				ShapeRange=15;
				break;
			case 40:
				// Medium road
				ShapeRange=4;
				break;
			case 50:
				// Small road
				ShapeRange=2;
				break;
			case 60:
				// Railroad
				ShapeRange=10;
				break;
			case 70:
				// Big city
				ShapeRange=15;
				break;
			case 80:
				// Med city
				ShapeRange=10;
				break;
			case 90:
				// Small city
				ShapeRange=6;
				break;
			case 100:
				// Very small city
				ShapeRange=2;
				break;
			case 110:
				// City polyline area
				ShapeRange=15;
				break;
			default:
				// UNKNOWN
				ShapeRange=100;
				break;
		}
		// ShapeRange is belonging to a LKMAPS topology..
		if (LKTopo == -1) {
			// Problem. Mixed topology file. No good..
			StartupStore(_T("------ INVALID MIXED OLD/NEW TOPOLOGY FILES- TOPOLOGY IGNORED%s"),NEWLINE);
			UnlockTerrainDataGraphics();
			LKTopo=0;
			return;
		}
		#if DEBUG_LKTOPO
		StartupStore(_T("... LKMAPS new topo file%s"),NEWLINE);
		#endif
		LKTopo++;
			
	} else {
		#if DEBUG_LKTOPO
		StartupStore(_T("... OLD XCS topo file%s"),NEWLINE);
		#endif
		LKTopo=-1;
	}

        
        // Shape icon
        PExtractParameter(TempString, ctemp, 2);
        ShapeIcon = _tcstol(ctemp, &Stop, 10);
        
        // Shape field for text display
        
        // sjt 02NOV05 - field parameter enabled
        PExtractParameter(TempString, ctemp, 3);
        if (iswalnum(ctemp[0])) {
          ShapeField = _tcstol(ctemp, &Stop, 10);
          ShapeField--;
        } else {
          ShapeField = -1;
	}
        
        // Red component of line / shading colour
        PExtractParameter(TempString, ctemp, 4);
        red = (BYTE)_tcstol(ctemp, &Stop, 10);
        
        // Green component of line / shading colour
        PExtractParameter(TempString, ctemp, 5);
        green = (BYTE)_tcstol(ctemp, &Stop, 10);
        
        // Blue component of line / shading colour
        PExtractParameter(TempString, ctemp, 6);
        blue = (BYTE)_tcstol(ctemp, &Stop, 10);
        
        if ((red==64) 
            && (green==96) 
            && (blue==240)) {
          // JMW update colours to ICAO standard
          red =    85; // water colours
          green = 160;
          blue =  255;
        }
        
        if (ShapeField<0) {
          Topology* newtopo;
          newtopo = new Topology(wShapeFilename);
          TopoStore[numtopo] = newtopo;
        } else {
          TopologyLabel *newtopol;
          newtopol = new TopologyLabel(wShapeFilename, ShapeField);
          TopoStore[numtopo] = newtopol;
        }


        TopoStore[numtopo]->scaleCategory = shapeIndex;
        TopoStore[numtopo]->scaleDefaultThreshold = ShapeRange;
	TopoStore[numtopo]->scaleThreshold = ShapeRange;

        if (ShapeIcon!=0) 
          TopoStore[numtopo]->loadBitmap(ShapeIcon);
	else {
	  // Careful not to use hPen and hBrush then! Always check that it is not null
          TopoStore[numtopo]->loadPenBrush(RGB(red,green,blue));
	}

	if (shapeIndex ==  5) if ( LKTopoZoomCat05 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat05;
	if (shapeIndex == 10) if ( LKTopoZoomCat10 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat10;
	if (shapeIndex == 20) if ( LKTopoZoomCat20 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat20;
	if (shapeIndex == 30) if ( LKTopoZoomCat30 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat30;
	if (shapeIndex == 40) if ( LKTopoZoomCat40 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat40;
	if (shapeIndex == 50) if ( LKTopoZoomCat50 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat50;
	if (shapeIndex == 60) if ( LKTopoZoomCat60 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat60;
	if (shapeIndex == 70) if ( LKTopoZoomCat70 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat70;
	if (shapeIndex == 80) if ( LKTopoZoomCat80 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat80;
	if (shapeIndex == 90) if ( LKTopoZoomCat90 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat90;
	if (shapeIndex == 100) if ( LKTopoZoomCat100 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat100;
	if (shapeIndex == 110) if ( LKTopoZoomCat110 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat110;

	#if DEBUG_LKTOPO
	StartupStore(_T("... TopoStore[%d] scaleCategory=%d Threshold=%f defaultthreshold=%f%s"),numtopo,shapeIndex,
		TopoStore[numtopo]->scaleThreshold,TopoStore[numtopo]->scaleDefaultThreshold,NEWLINE);
	#endif

        numtopo++;
      }
  }
  
  //  CloseHandle (hFile);
  zzip_fclose(zFile);

  // file was OK, so save it
  SetRegistryString(szRegistryTopologyFile, szOrigFile);

  if (LKTopo>0) {
	StartupStore(_T(". LKMAPS Advanced Topology file found%s"),NEWLINE);
  } else {
	LKTopo=0;
  }

  UnlockTerrainDataGraphics();

}


