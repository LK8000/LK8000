/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Topology.cpp,v 8.5 2010/12/12 23:24:48 root Exp root $
*/

#include "externs.h"
#include <ctype.h> // needed for Wine
#include "Topology.h"
#include "Multimap.h"
#include "OS/Memory.h"
#include "resource_data.h"

#include <Poco/Latin1Encoding.h>
#include <Poco/UTF16Encoding.h>
#include <Poco/UTF8Encoding.h>
#include <Poco/TextConverter.h>

#include "../Draw/ScreenProjection.h"
#include "ShapeSpecialRenderer.h"
#include "NavFunctions.h"
#include "ScreenGeometry.h"
#include "utils/stringext.h"
#include <functional>

#ifdef ENABLE_OPENGL
#include "OpenGL/GLShapeRenderer.h"
#include "shapelib/mapshape.h"
#endif

//#define DEBUG_TFC

XShape::XShape() : hide(false) {
  msInitShape(&shape);
}


XShape::~XShape() {
  clear();
}


void XShape::clear() {
  msFreeShape(&shape);
}


void XShape::load(shapefileObj* shpfile, int i) {
  msSHPReadShape(shpfile->hSHP, i, &shape);
}


void Topology::loadBitmap(const int xx) {
  hBitmap.LoadFromResource(MAKEINTRESOURCE(xx));
}

// thecolor is relative to shapes, not to labels
// TODO (maybe): get rid of LKM internal colors defined per shape level,
// and assign them here dynamically. Easy and fast to do.
void Topology::loadPenBrush(const LKColor thecolor) {
  int psize;
  switch(scaleCategory) {
	case 20: // water lines
		switch(ScreenSize) {
			case ss800x480:
			case ss640x480:
				psize=3;
				break;
			case ss480x272:
			case ss320x240:
			case ss400x240:
			case ss480x640:
			case ss480x800:
				psize=2;
				break;
			default:
#if !defined(PNA) || !defined(UNDER_CE)

                                psize=IBLSCALE(2);
#else
				if (ScreenLandscape)
				    psize=3;
				else
				    psize=NIBLSCALE(1);
#endif
				break;
		}
		break;
	case 30: // big roads
		switch(ScreenSize) {
			case ss800x480:
			case ss640x480:
				psize=3;
				break;
			case ss480x272:
			case ss320x240:
			case ss400x240:
			case ss480x640:
			case ss480x800:
				psize=2;
				break;
			default:
#if !defined(PNA) || !defined(UNDER_CE)

				psize=IBLSCALE(2);
#else
				if (ScreenLandscape)
				    psize=3;
				else
				    psize=NIBLSCALE(1);
#endif
				break;
		}
		break;
	case 40: // medium roads
		switch(ScreenSize) {
			case ss800x480:
				psize=2;
				break;
			case ss480x272:
			case ss640x480:
			case ss480x640:
			case ss480x800:
				psize=NIBLSCALE(1);
				break;
			default:
                                psize=IBLSCALE(1);
				break;
		}
		break;
	case 60: // railroads
		switch(ScreenSize) {
			default:
				psize=IBLSCALE(1);
		}
		break;

	default:
		psize=NIBLSCALE(1);
		break;
  }
  hPen.Create(PEN_SOLID, psize, thecolor);
  hbBrush.Create(thecolor);

}

Topology::Topology(const TCHAR* shpname) {
  memset((void*)&shpfile, 0 ,sizeof(shpfile));
  shapefileopen = false;
  triggerUpdateCache = false;
  scaleThreshold = 0;
  shpCache= NULL;

  shpBounds = NULL;
  shps = NULL;
  cache_mode = 0;
  lastBounds.minx = lastBounds.miny = lastBounds.maxx = lastBounds.maxy = 0;

  in_scale = false;

  // filename aleady points to _MAPS subdirectory!
  to_utf8(shpname, filename);

  Open();
}


void Topology::initCache()
{
  //Selecting caching scenarios based on available memory and topo size
  // Unfortunatelly I don't find a suitable algorithm to estimate the loaded
  // shapefile's memory footprint so we never choose mode2. KR
  // v5 note by Paolo: mode2 had a critical bug in delete, so good we did not use it
  size_t free_size = CheckFreeRam();
  size_t bounds_size = sizeof(rectObj)*shpfile.numshapes;

  //Cache mode selection based on available memory
  cache_mode = 0;
  free_size -= 10000*1024;		// Safe: if we don't have enough memory we use mode0
  if (free_size>bounds_size) cache_mode = 1;

  // TESTING ONLY, mode override
  //cache_mode = 2;

  shpBounds = NULL;
  shps = NULL;
  in_scale_last = false;

  for (int i=0; i<shpfile.numshapes; i++) shpCache[i] = NULL;

  //StartupStore(_T("... Topology InitCache mode: %d  (free_size=%ld  bounds_size=%ld)\n"),cache_mode,free_size,bounds_size);

  switch (cache_mode) {
	default:
	case 0:
		// Original
		#ifdef DEBUG_TFC
		StartupStore(_T("Topology cache using mode 0%s"), NEWLINE);
		#endif
		break;

	case 1:
		// Using bounds array in memory
		#ifdef DEBUG_TFC
		StartupStore(_T(". Topology cache using mode 1%s"), NEWLINE);
		#endif
		shpBounds = (rectObj*)malloc(sizeof(rectObj)*shpfile.numshapes);
		if (shpBounds == NULL) {
			//Fallback to mode 0
			StartupStore(_T("------ WARN Topology,  malloc failed shpBounds, fallback to mode0%s"), NEWLINE);
			cache_mode = 0;
			break;
		}
		// Get bounds for each shape from shapefile
		rectObj *prect;
		int retval;
		for (int i=0; i<shpfile.numshapes; i++) {
			prect = &shpBounds[i];
			retval = msSHPReadBounds(shpfile.hSHP, i, prect);
			if (retval) {
				StartupStore(_T("------ WARN Topology, shape bounds reading failed, fallback to mode0%s"), NEWLINE);
				// Cleanup
				free(shpBounds);
				shpBounds=NULL;
				cache_mode = 0;
				break;
			}
		}//for
		break;

#if 0 // buggy
	case 2:
		// Using shape array in memory
		#ifdef DEBUG_TFC
		StartupStore(_T(". Topology cache using mode 2%s"), NEWLINE);
		#endif
		shpBounds = NULL;
		shps = (XShape**)malloc(sizeof(XShape*)*shpfile.numshapes);
		if (shps == NULL) {
			//Fallback to mode 0
			StartupStore(_T("------ WARN Topology,  malloc failed shps, fallback to mode0%s"), NEWLINE);
			cache_mode = 0;
			break;
		}
		// Load all shapes to shps
		for (int i=0; i<shpfile.numshapes; i++) {
			if ( (shps[i] = addShape(i)) == NULL ) {
				StartupStore(_T("------ WARN Topology,  addShape failed for shps[%d], fallback to mode0%s"), i, NEWLINE);
				// Cleanup
				for (int j=0; j<i; j++) delete(shps[i]); // BUG, should be using j not i
				free(shps);
				shps=NULL;
				cache_mode = 0;
				break;
			}
		}
#endif

  } //sw
}


void Topology::Open() {
  shapefileopen = false;

  if (msShapefileOpen(&shpfile, "rb", filename, true) == -1) {
    StartupStore(_T("------ Topology: Open FAILED for <%s>%s"),filename,NEWLINE);
    return;
  }

  scaleThreshold = 1000.0;
  shpCache = (XShape**)malloc(sizeof(XShape*)*shpfile.numshapes);
  if (shpCache) {
    initCache();
    shapefileopen = true;
  } else {
	StartupStore(_T("------ ERR Topology,  malloc failed shpCache%s"), NEWLINE);
  }
}


void Topology::Close() {
  if (shapefileopen) {
    if (shpCache) {
      flushCache();
      free(shpCache); shpCache = NULL;
    }
    if (shpBounds) {
      free(shpBounds); shpBounds = NULL;
    }
    if (shps) {
	  for (int i=0; i<shpfile.numshapes; i++) {
		if (shps[i]) delete shps[i];
	  }
      free(shps); shps = NULL;
    }
    msShapefileClose(&shpfile);
    shapefileopen = false;  // added sgi
  }
}


Topology::~Topology() {
  Close();
}


bool Topology::CheckScale(void) {
  if (scaleCategory==10||scaleCategory==5)
	return (MapWindow::zoom.RealScale() <= scaleDefaultThreshold);
  else
	return (MapWindow::zoom.RealScale() <= scaleThreshold);
}

void Topology::TriggerIfScaleNowVisible(void) {
  triggerUpdateCache |= (CheckScale() != in_scale);
}

//
// Always check shpCache is not NULL before calling flushCache!
//
void Topology::flushCache() {
#ifdef DEBUG_TFC
  StartupStore(TEXT("---flushCache() starts%s"),NEWLINE);
  PeriodClock starttick;
  starttick.Update();
#endif
  switch (cache_mode) {
	case 0:  // Original
	case 1:  // Bounds array in memory
		for (int i=0; i<shpfile.numshapes; i++) {
			removeShape(i);
		}
		break;
	case 2:  // Shapes in memory
		for (int i=0; i<shpfile.numshapes; i++) {
			shpCache[i] = NULL;
		}
		break;
  }//sw
  shapes_visible_count = 0;
#ifdef DEBUG_TFC
  StartupStore(TEXT("   flushCache() ends (%dms)%s"),starttick.Elapsed(),NEWLINE);
#endif
}

void Topology::updateCache(rectObj thebounds, bool purgeonly) {
  if (!triggerUpdateCache) return;

  if (!shapefileopen || !shpCache) return;

  in_scale = CheckScale();

  if (!in_scale) {
    // not visible, so flush the cache
    // otherwise we waste time on looking up which shapes are in bounds
    flushCache();
    triggerUpdateCache = false;
    in_scale_last = false;
    return;
  }

  if (purgeonly) {
    in_scale_last = in_scale;
    return;
  }

  triggerUpdateCache = false;

#ifdef DEBUG_TFC
  StartupStore(TEXT("---UpdateCache() starts, mode%d%s"),cache_mode,NEWLINE);
  PeriodClock starttick;
  starttick.Update();
#endif

  if(msRectOverlap(&shpfile.bounds, &thebounds) != MS_TRUE) {
    // this happens if entire shape is out of range
    // so clear buffer.
    flushCache();
    in_scale_last = in_scale;
    return;
  }

  bool smaller = false;
  bool bigger = false;
  bool in_scale_again = in_scale && !in_scale_last;
  int shapes_loaded = 0;
  shapes_visible_count = 0;
  in_scale_last = in_scale;

  switch (cache_mode) {
    case 0: // Original code plus one special case
      smaller = (msRectContained(&thebounds, &lastBounds) == MS_TRUE);
      if (smaller) { //Special case, search inside, we don't need to load additional shapes, just remove
        shapes_visible_count = 0;
        for (int i=0; i<shpfile.numshapes; i++) {
          if (shpCache[i]) {
            if(msRectOverlap(&(shpCache[i]->shape.bounds), &thebounds) != MS_TRUE) {
              removeShape(i);
            } else shapes_visible_count++;
          }
        }//for
      } else {
        //In this case we have to run the original algoritm
        msShapefileWhichShapes(&shpfile, thebounds, 0);
        shapes_visible_count = 0;
        for (int i=0; i<shpfile.numshapes; i++) {
          if (msGetBit(shpfile.status, i)) {
            if (shpCache[i]==NULL) {
              // shape is now in range, and wasn't before
              shpCache[i] = addShape(i);
              shapes_loaded++;
            }
            shapes_visible_count++;
          } else {
            removeShape(i);
          }
        }//for
      }
      break;

    case 1:  // Bounds array in memory
      bigger = (msRectContained(&lastBounds, &thebounds) == MS_TRUE);
      smaller = (msRectContained(&thebounds, &lastBounds) == MS_TRUE);
      if (bigger || in_scale_again) { //We don't need to remove shapes, just load, so skip loaded ones
        for (int i=0; i<shpfile.numshapes; i++) {
          if (shpCache[i]) continue;
          if(msRectOverlap(&shpBounds[i], &thebounds) == MS_TRUE) {
            // shape is now in range, and wasn't before
            shpCache[i] = addShape(i);
            shapes_loaded++;
          }
        }//for
        shapes_visible_count+=shapes_loaded;
      } else
      if (smaller) { //Search inside, we don't need to load additional shapes, just remove
        for (int i=0; i<shpfile.numshapes; i++) {
          if (shpCache[i]==NULL) continue;
          if(msRectOverlap(&shpBounds[i], &thebounds) != MS_TRUE) {
            removeShape(i);
          } else shapes_visible_count++;
        }//for
      } else {
        //Otherwise we have to search the all array
        for (int i=0; i<shpfile.numshapes; i++) {
          if(msRectOverlap(&shpBounds[i], &thebounds) == MS_TRUE) {
            if (shpCache[i]==NULL) {
              // shape is now in range, and wasn't before
              shpCache[i] = addShape(i);
              shapes_loaded++;
            }
            shapes_visible_count++;
          } else {
            removeShape(i);
          }
        }//for
      }
      break;

    case 2: // All shapes in memory
      XShape *pshp;
      shapes_visible_count = 0;
      for (int i=0; i<shpfile.numshapes; i++) {
        pshp = shps[i];
        if(msRectOverlap(&(pshp->shape.bounds), &thebounds) == MS_TRUE) {
          shpCache[i] = pshp;
          shapes_visible_count++;
        } else {
          shpCache[i] = NULL;
        }
      }//for
      break;
    }//sw

    lastBounds = thebounds;

#ifdef DEBUG_TFC
  long free_size = CheckFreeRam();
  StartupStore(TEXT("   UpdateCache() ends, shps_visible=%d ram=%luM (%dms)%s"),shapes_visible_count, free_size/(1024*1024), starttick.Elapsed(),NEWLINE);
#endif
}


XShape* Topology::addShape(const int i) {
  XShape* theshape = new XShape();
  LKASSERT(theshape);
  theshape->load(&shpfile,i);
  return theshape;
}


// Be sure shpCache is not NULL before calling removeShape
void Topology::removeShape(const int i) {
  if (shpCache[i]) {
    delete shpCache[i];
    shpCache[i]= NULL;
  }
}


// This is checking boundaries based on lat/lon values.
// It is not enough for screen overlapping verification.
bool Topology::checkVisible(const shapeObj& shape, const rectObj &screenRect) {
  return (msRectOverlap(&shape.bounds, &screenRect) == MS_TRUE);
}

template <typename T>
static T shape2Screen(const lineObj& line, int iskip, const ScreenProjection& _Proj, std::vector<T>& points) {

  typedef typename T::scalar_type scalar_type;
  using std::placeholders::_1;

  T leftPoint = {
    std::numeric_limits<scalar_type>::max(),
    std::numeric_limits<scalar_type>::max()
  };
  const int last = line.numpoints-1;
  points.clear();
  points.reserve((line.numpoints/iskip)+1);


  const GeoToScreen<T> ToScreen(_Proj);

  // first point is inserted before loop for avoid to check "if(first)" inside loop
  points.push_back(ToScreen(line.point[0]));

  for(int i = 1; i < last; i+=iskip) {
    const T pt = ToScreen(line.point[i]);
    if (pt.x<=leftPoint.x) {
      leftPoint = pt;
    }
    const T& prev_pt = points.back();
    if(lround(std::abs((prev_pt.x - pt.x) + std::abs(prev_pt.y - pt.y))) > 2) {
        points.push_back(std::move(pt));
    }
  }
  const T pt = ToScreen(line.point[last]);
  if (pt.x<=leftPoint.x) {
    leftPoint = pt;
  }
  points.push_back(std::move(pt));

  return leftPoint;
}
// Paint a single topology element

void Topology::Paint(ShapeSpecialRenderer& renderer, LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj) const {

  if (!shapefileopen) return;
  bool nolabels=false;
  // 130217 scaleCat 5 and 10 are the same! So careful..
  if (scaleCategory==10||scaleCategory==5) {
	// for water areas, use scaleDefault
	if ( MapWindow::zoom.RealScale()>scaleDefaultThreshold) {
		return;
	}
	// since we just checked category 10, if we are over scale we set nolabels
	if ( MapWindow::zoom.RealScale()>scaleThreshold) nolabels=true;
  } else
  if (MapWindow::zoom.RealScale() > scaleThreshold) return;

#ifdef ENABLE_OPENGL
  static GLShapeRenderer shape_renderer;
  shape_renderer.setClipRect(rc);
  shape_renderer.setNoLabel(nolabels);
#endif


  // TODO code: only draw inside screen!
  // this will save time with rendering pixmaps especially
  // checkVisible does only check lat lon , not screen pixels..
  // We need to check also screen.


  const auto hpOld = Surface.SelectObject(hPen);

  LKSurface::OldBrush hbOld {};
  if (hbBrush) {
    hbOld = Surface.SelectObject(hbBrush);
  }

  const auto hfOld = Surface.SelectObject(MapTopologyFont);

#ifndef ENABLE_OPENGL
  // get drawing info
  int iskip = 1;

  // attempt to bugfix 100615 polyline glitch with zoom over 33Km
  // do not skip points, if drawing coast lines which have a scaleThreshold of 100km!
  // != 5 and != 10
  if (scaleCategory>10) {
    if (MapWindow::zoom.RealScale()>0.25*scaleThreshold) {
      iskip = 2;
    }
    if (MapWindow::zoom.RealScale()>0.5*scaleThreshold) {
      iskip = 3;
    }
    if (MapWindow::zoom.RealScale()>0.75*scaleThreshold) {
      iskip = 4;
    }
  }

  typedef RasterPoint ScreenPoint;
#else
  typedef FloatPoint ScreenPoint;
#endif

  // use the already existing screenbounds_latlon, calculated by CalculateScreenPositions in MapWindow2
  const rectObj screenRect = MapWindow::screenbounds_latlon;



  static std::vector<ScreenPoint> points;

  for (int ixshp = 0; ixshp < shpfile.numshapes; ixshp++) {

    XShape *cshape = shpCache[ixshp];

    if (!cshape || cshape->hide) continue;

    shapeObj *shape = &(cshape->shape);

    switch(shape->type) {


      case(MS_SHAPE_POINT):{

        bool dobitmap=false;
        if (scaleCategory<90 || (MapWindow::zoom.RealScale()<2)) dobitmap=true;
        // first a latlon overlap check, only approximated because of fastcosine in latlon2screen
        if (checkVisible(*shape, screenRect))
          for (int tt = 0; tt < shape->numlines; tt++) {
            const lineObj &line = shape->line[tt];
			for (int jj=0; jj< line.numpoints; jj++) {
                const pointObj &point = line.point[jj];
                if(msPointInRect(&point, &screenRect)) {
                    const POINT sc = _Proj.ToRasterPoint(point.y, point.x);
                    if (dobitmap) {
                        // bugfix 101212 missing case for scaleCategory 0 (markers)
                        if (scaleCategory == 0 ||
                            cshape->renderSpecial(renderer, Surface, sc.x, sc.y, rc)) {
                            MapWindow::DrawBitmapIn(Surface, sc, hBitmap);
                        }
                    } else {
                        cshape->renderSpecial(renderer, Surface, sc.x, sc.y, rc);
                    }
                }
			}
		}
	  }
	  break;

    case(MS_SHAPE_LINE):

      if (checkVisible(*shape, screenRect))
        for (int tt = 0; tt < shape->numlines; ++tt) {
          const ScreenPoint ptLabel = shape2Screen(shape->line[tt], 1, _Proj, points);
          Surface.Polyline(points.data(), points.size(), rc);
          cshape->renderSpecial(renderer, Surface,ptLabel.x,ptLabel.y,rc);
        }
      break;

    case(MS_SHAPE_POLYGON):
	  // if it's a water area (nolabels), print shape up to defaultShape, but print
	  // labels only up to custom label levels
      if (checkVisible(*shape, screenRect)) {
#ifdef ENABLE_OPENGL
        shape_renderer.renderPolygon(renderer, Surface, *cshape, hbBrush, _Proj);
#else
        for (int tt = 0; tt < shape->numlines; ++tt) {
          const RasterPoint ptLabel = shape2Screen(shape->line[tt], iskip, _Proj, points);
          Surface.Polygon(points.data(), points.size(), rc);
          if (!nolabels ) {
            cshape->renderSpecial(renderer, Surface,ptLabel.x,ptLabel.y,rc);
          }
        }
#endif
      }

	break;

    default:
      break;
    }
  }
  if (hbOld) {
    Surface.SelectObject(hbOld);
  }
  Surface.SelectObject(hpOld);
  Surface.SelectObject(hfOld);
}

TopologyLabel::TopologyLabel(const TCHAR* shpname, int field1):Topology(shpname),bUTF8()
{
  setField(max(0,field1));
};

TopologyLabel::~TopologyLabel()
{
}


void TopologyLabel::setField(int i) {
  field = i;
}

XShape* TopologyLabel::addShape(const int i) {

  XShapeLabel* theshape = new XShapeLabel();
  LKASSERT(theshape);
  theshape->load(&shpfile,i);
  theshape->setlabel(msDBFReadStringAttribute( shpfile.hDBF, i, field),bUTF8);
  return theshape;
}

bool XShapeLabel::nearestItem(int category, double lon, double lat) {

  NearestTopoItem *item;
  if(!label || _tcslen(label) == 0) {
      return false;
  }

  switch(category) {
	case 10:
		item=&NearestWaterArea;
		break;
	case 70:
	case 80:
	case 110:
		item=&NearestBigCity;
		break;
	case 90:
		item=&NearestCity;
		break;
	case 100:
		item=&NearestSmallCity;
		break;
	default:
		#if TESTBENCH
		StartupStore(_T("...... Cannot use nearestItem cat=%d <%s>%s"),category,label,NEWLINE);
		#endif
		return false;
		break;
  }

  double distance, bearing;
  DistanceBearing(lat,lon,GPS_INFO.Latitude, GPS_INFO.Longitude,
	&distance, &bearing);

  #if DEBUG_NEARESTTOPO
  StartupStore(_T("... cat=%d, <%s> lat=%f lon=%f mylat=%f mylon=%f distance=%.2f\n"),category, Temp,lat,lon,
	GPS_INFO.Latitude, GPS_INFO.Longitude, distance/1000);
  #endif

  // If first time, use it
  if (!item->Valid || (item->Distance > distance)) {
	item->Latitude=lat;
	item->Longitude=lon;
	_tcscpy(item->Name,label);
	item->Distance=distance;
	item->Bearing=bearing;
	item->Valid=true;

	#if DEBUG_NEARESTTOPO
	StartupStore(_T(".... cat=%d, <%s> dist=%f nearer\n"),category, Temp,distance);
	#endif
	return true;
  }

  return false;

}

// Print topology labels
bool XShapeLabel::renderSpecial(ShapeSpecialRenderer& renderer, LKSurface& Surface, int x, int y, const RECT& ClipRect) const {
  if (label && ((GetMultimap_Labels()==MAPLABELS_ALLON)||(GetMultimap_Labels()==MAPLABELS_ONLYTOPO))) {

    //Do not waste time with null labels
    int size = _tcslen(label);
    if(size <= 0) {
        return false;
    }

	SIZE tsize;
	RECT brect;
	Surface.GetTextSize(label, &tsize);

	// shift label from center point of shape
	x+= NIBLSCALE(2);
	y+= NIBLSCALE(2);
	brect.left = x-NIBLSCALE(3);
	brect.right = brect.left+tsize.cx+NIBLSCALE(3);
	brect.top = y-NIBLSCALE(3);
	brect.bottom = brect.top+tsize.cy+NIBLSCALE(3);

	if ( DeclutterMode != (DeclutterMode_t)dmDisabled ) {
		if (!MapWindow::checkLabelBlock(&brect)) return false;
	}

	// Do not print outside boundaries of Draw area
	if (brect.bottom>=ClipRect.bottom ||
		brect.top<=ClipRect.top ||
		brect.left<=ClipRect.left ||
		brect.right>=ClipRect.right) return false;

    renderer.Add({x, y}, label);
	return true; // 101016
  }
  return false; // 101016
}


void XShapeLabel::setlabel(const char* src, bool bUTF8) {
  // Case1 : NULL or not informative label, we show the shape without label
  if (
      (src == NULL) ||
      (strcmp(src,"NULL") == 0) ||
      (strcmp(src,"UNK") == 0)
     ) {
	if (label) {
		free(label);
		label= NULL;
	}
	hide=false;
	return;
  }

  // Case2 : shapes that do not contain any useful information, we HIDE the shape and the label
  if (
      (strcmp(src,"RAILWAY STATION") == 0) ||
      (strcmp(src,"RAILROAD STATION") == 0)
     ) {
	if (label) {
		free(label);
		label= NULL;
	}
	hide=true;
	return;
  }
  // Any other case : we display shape and its label as well

  if (label) {
      free(label);
      label= NULL;
  }

#ifdef UNICODE

    if ( bUTF8 ) {
      // from utf-8 to UNICODE
      size_t size = strlen(src) + 1;
      if(size) {
        label = (TCHAR*) malloc(size * sizeof (TCHAR));
        from_utf8(src, label, size);
      }
    }
    else {
      // from Latin1 (ANSI) To UNICODE
      int nChars = MultiByteToWideChar(CP_ACP, 0, src, -1, NULL, 0);
      if (nChars) {
          label = (TCHAR*) malloc(nChars * sizeof (TCHAR));
          if (label) {
              MultiByteToWideChar(CP_ACP, 0, src, -1, label, nChars);
          }
      }
    }


#else


    if ( bUTF8 ) {
      // do simple copy 
      label = strdup(src);
    }
    else {
      std::string utf8String;

      Poco::Latin1Encoding Latin1Encoding;
      Poco::UTF8Encoding utf8Encoding;
       
      // from Latin1 (ISO-8859-1) To Utf8
      Poco::TextConverter converter(Latin1Encoding, utf8Encoding);
      converter.convert(src, strlen(src), utf8String);
      label = strdup(utf8String.c_str());
    }


#endif

  hide=false;
}


XShapeLabel::~XShapeLabel() {
  if (label) {
    free(label);
    label= NULL;
  }
}



void XShapeLabel::clear() {
  XShape::clear();
  if (label) {
    free(label);
    label= NULL;
  }
}

// //////////////////////////////////////////////////////////////

void Topology::SearchNearest(const rectObj& bounds) {

  if (!shapefileopen) return;

  if(msRectOverlap(&shpfile.bounds, &bounds) != MS_TRUE) {
      return;
  }

  for (int ixshp = 0; ixshp < shpfile.numshapes; ixshp++) {

    std::unique_ptr<XShape> shape_tmp;
	XShape *cshape = shpCache[ixshp];
    if(!cshape) {
      if((cache_mode == 1) && (msRectOverlap(&shpBounds[ixshp], &bounds) != MS_TRUE)) {
          // if bounds is in cache and does not overlap no need to load shape;
          continue;
      }
      shape_tmp.reset(addShape(ixshp));
      cshape = shape_tmp.get();
    }

	if (!cshape || cshape->hide || !cshape->HasLabel()) continue;
	const shapeObj& shape = cshape->shape;

    if(msRectOverlap(&(cshape->shape.bounds), &bounds) != MS_TRUE) {
        continue;
    }

	switch(shape.type) {

	   case(MS_SHAPE_POINT):

			for (int tt = 0; tt < shape.numlines; tt++) {
				for (int jj=0; jj< shape.line[tt].numpoints; jj++) {
					cshape->nearestItem(scaleCategory, shape.line[tt].point[jj].x, shape.line[tt].point[jj].y);
				}
			}
		break;

	   case(MS_SHAPE_LINE):
/*
			for (int tt = 0; tt < shape->numlines; tt ++) {

				int minx = rc.right;
				int miny = rc.bottom;
				int msize = min(shape->line[tt].numpoints, MAXCLIPPOLYGON);

				MapWindow::LatLon2Screen(shape->line[tt].point, pt, msize, 1);
				for (int jj=0; jj< msize; jj++) {
					if (pt[jj].x<=minx) {
						minx = pt[jj].x;
						miny = pt[jj].y;
					}
				}

				cshape->nearestItem(scaleCategory, shape->line[tt].point[0].x, shape->line[tt].point[0].y);
			}
*/
		break;

	   case(MS_SHAPE_POLYGON):

			for (int tt = 0; tt < shape.numlines; tt ++) {
                // TODO : check if that is good
                //  it's surprising distance to Polygon are not distance to first point but
                //  distance to nearest vertex.
                //  right implementation is in #msDistancePointToShape, but this don't use great circle ditance.
				cshape->nearestItem(scaleCategory, shape.line[tt].point[0].x, shape.line[tt].point[0].y);
			}
        break;

	   default:
		break;

    } // switch type of shape
  } // for all shapes in this category
} // Topology SearchNearest
