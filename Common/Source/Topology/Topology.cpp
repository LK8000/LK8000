/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Topology.cpp,v 8.5 2010/12/12 23:24:48 root Exp root $
*/

#include "externs.h"
#include <ctype.h> // needed for Wine
#include "Topology.h"
#include "Multimap.h"
#include "OS/Memory.h"
#include "resource_data.h"

#include "../Draw/ScreenProjection.h"
#include "ShapeSpecialRenderer.h"
#include "NavFunctions.h"
#include "ScreenGeometry.h"
#include "utils/charset_helper.h"
#include "utils/array_adaptor.h"
#include <functional>
#include "Utils.h"

#include "ShapePolygonRenderer.h"
#include "shapelib/mapshape.h"
#include "XShape.h"

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
        psize=IBLSCALE(2);
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
				psize=IBLSCALE(2);
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

Topology::Topology(const TCHAR* shpname, int field1) : field(field1) {
  // filename already points to _MAPS subdirectory!
  to_utf8(shpname, filename);
  Open();
}

bool Topology::initCache_0() {
  cache_mode = 0;

  StartupStore(_T(". <%s> Topology cache using mode 0"), from_utf8(filename).c_str());

  return true;
}

// Using bounds array in memory

bool Topology::initCache_1() {
  cache_mode = 1;

  StartupStore(_T(". <%s> Topology cache using mode 1"), from_utf8(filename).c_str());
  try {
    shpBounds = std::make_unique<rectObj[]>(shpfile.numshapes);
  }
  catch (std::exception&) {
    //Fallback to mode 0
    StartupStore(_T("------ WARN Topology,  malloc failed shpBounds, fallback to mode0%s"), NEWLINE);
    return initCache_0();
  }
  // Get bounds for each shape from shapefile
  for (int i = 0; i < shpfile.numshapes; i++) {
    rectObj *prect = &shpBounds[i];
    int retval = msSHPReadBounds(shpfile.hSHP, i, prect);
    if (retval) {
      StartupStore(_T("------ WARN Topology, shape bounds reading failed, fallback to mode0%s"), NEWLINE);
      // Cleanup
      shpBounds = nullptr;
      return initCache_0();
    }
  }//for  
  return true;
}

#ifdef USE_TOPOLOGY_CACHE_LEVEL2

// Using shape array in memory
bool Topology::initCache_2() {
  cache_mode = 2;

  StartupStore(_T(". <%s> Topology cache using mode 2"), from_utf8(filename).c_str());

  shpBounds = nullptr;
  try {
    shps = std::make_unique<XShapePtr[]>(shpfile.numshapes);
  }
  catch (std::exception&) {
    // Fallback to mode 0
    StartupStore(
        _T("------ WARN Topology,  malloc failed shps, fallback to mode 0"));
    return initCache_0();
  }
  // Load all shapes to shps
  for (int i = 0; i < shpfile.numshapes; i++) {
    if (!(shps[i] = loadShape(i))) {
      StartupStore(_T("------ WARN Topology,  loadShape failed for shps[%d], fallback to mode 1"), i);
      // Cleanup
      shps = nullptr;
      return initCache_1();
    }
  }
  return true;
}

#endif

void Topology::initCache() {
  //Selecting caching scenarios based on available memory and topo size
  // Unfortunatelly I don't find a suitable algorithm to estimate the loaded
  // shapefile's memory footprint so we never choose mode2. KR

  size_t free_size = CheckFreeRam();
  size_t bounds_size = sizeof(rectObj) * shpfile.numshapes;

  //Cache mode selection based on available memory
  cache_mode = 0;

  free_size -= 10 * 1024 * 1024;  // Safe: if we more than have 10MB of free memory can try mode 1
  if (free_size > bounds_size) {
    cache_mode = 1;
  }

#ifdef USE_TOPOLOGY_CACHE_LEVEL2
  bounds_size = (sizeof(XShapePtr) + sizeof(XShape)) * shpfile.numshapes;
  free_size -= 40 * 1024 * 1024; // Safe: if we more than have 50MB of free memory we can try mode 2
  if (free_size > bounds_size) {
    cache_mode = 2;
  }
#endif

  shpBounds = nullptr;
  shps = nullptr;
  in_scale_last = false;

  for (int i = 0; i < shpfile.numshapes; i++) {
    shpCache[i] = nullptr;
  }

  switch (cache_mode) {
    default:
    case 0:
      initCache_0();
      break;
    case 1:
      initCache_1();
      break;
#ifdef USE_TOPOLOGY_CACHE_LEVEL2
    case 2:
      initCache_2();
      break;
#endif
  } //sw
}

void Topology::Open() {
  shapefileopen = false;

  if (msShapefileOpen(&shpfile, "rb", filename, true) == -1) {
    StartupStore(_T("------ Topology: Open FAILED for <%s>"), from_utf8(filename).c_str());
    return;
  }

  scaleThreshold = 1000.0;
  try {
    shpCache = std::make_unique<XShapePtr[]>(shpfile.numshapes);
    initCache();
    shapefileopen = true;
  }
  catch (std::exception&) {
    msShapefileClose(&shpfile);
    StartupStore(_T("------ ERR Topology,  malloc failed shpCache"));
  }
}

void Topology::Close() {
  if (shapefileopen) {
    shpCache = nullptr;
    shpBounds = nullptr;
    shps = nullptr;
    msShapefileClose(&shpfile);
    shapefileopen = false;  // added sgi
  }
}

Topology::~Topology() {
  Close();
}

bool Topology::CheckScale() {
  if (scaleCategory == 10 || scaleCategory == 5) {
    return (MapWindow::zoom.RealScale() <= scaleDefaultThreshold);
  }
  else {
    return (MapWindow::zoom.RealScale() <= scaleThreshold);
  }
}

void Topology::TriggerIfScaleNowVisible() {
  triggerUpdateCache |= (CheckScale() != in_scale);
}

//
// Always check shpCache is not NULL before calling flushCache!
//
void Topology::flushCache() {
  if (shpCache) {
    std::fill_n(shpCache.get(), shpfile.numshapes, nullptr);
  }
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

  if (msRectOverlap(&shpfile.bounds, &thebounds) != MS_TRUE) {
    // this happens if entire shape is out of range
    // so clear buffer.
    flushCache();
    in_scale_last = in_scale;
    return;
  }

  bool smaller = false;
  bool bigger = false;
  bool in_scale_again = in_scale && !in_scale_last;
  in_scale_last = in_scale;

  switch (cache_mode) {
    case 0: // Original code plus one special case
      smaller = (msRectContained(&thebounds, &lastBounds) == MS_TRUE);
      if (smaller) { //Special case, search inside, we don't need to load additional shapes, just remove
        for (int i=0; i<shpfile.numshapes; i++) {
          if (shpCache[i]) {
            if(msRectOverlap(&(shpCache[i]->shape.bounds), &thebounds) != MS_TRUE) {
              shpCache[i] = nullptr;
            }
          }
        }//for
      }
      else { //In this case we have to run the original algoritm
        msShapefileWhichShapes(&shpfile, thebounds, 0);
        for (int i=0; i<shpfile.numshapes; i++) {
          if (msGetBit(shpfile.status, i)) {
            if (!shpCache[i]) {
              // shape is now in range, and wasn't before
              shpCache[i] = loadShape(i);
            }
          }
          else {
            shpCache[i] = nullptr;
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
            shpCache[i] = loadShape(i);
          }
        }//for
      }
      else if (smaller) { //Search inside, we don't need to load additional shapes, just remove
        for (int i=0; i<shpfile.numshapes; i++) {
          if (!shpCache[i]) continue;
          if(msRectOverlap(&shpBounds[i], &thebounds) != MS_TRUE) {
            shpCache[i] = nullptr;
          }
        }//for
      }
      else {
        //Otherwise we have to search the all array
        for (int i=0; i<shpfile.numshapes; i++) {
          if(msRectOverlap(&shpBounds[i], &thebounds) == MS_TRUE) {
            if (!shpCache[i]) {
              // shape is now in range, and wasn't before
              shpCache[i] = loadShape(i);
            }
          }
          else {
            shpCache[i] = nullptr;
          }
        }//for
      }
      break;

    case 2: // All shapes in memory
      for (int i=0; i<shpfile.numshapes; i++) {
        XShapePtr pshp = shps[i];
        if(msRectOverlap(&(pshp->shape.bounds), &thebounds) == MS_TRUE) {
          shpCache[i] = pshp;
        }
        else {
          shpCache[i] = nullptr;
        }
      }//for
      break;
    }//sw

    lastBounds = thebounds;
}

std::unique_ptr<XShape> Topology::loadShape(const int i) {
  try {
    return std::make_unique<XShape>(&shpfile, i, field);
  }
  catch (std::exception&) {
  }
  return nullptr;
}

// This is checking boundaries based on lat/lon values.
// It is not enough for screen overlapping verification.
bool Topology::checkVisible(const shapeObj& shape, const rectObj &screenRect) {
  return (msRectOverlap(&shape.bounds, &screenRect) == MS_TRUE);
}

template <typename T>
static T shape2Screen(const lineObj& line, const ScreenProjection& _Proj, std::vector<T>& points) {

  using scalar_type = typename T::scalar_type ;
  using std::placeholders::_1;

  if (line.numpoints < 1) {
    return {
      std::numeric_limits<scalar_type>::max(),
      std::numeric_limits<scalar_type>::max()
    };
  }

  const int last = line.numpoints - 1;
  points.clear();
  points.reserve(line.numpoints + 1);

  const GeoToScreen<T> ToScreen(_Proj);

  // Process the first point outside the loop
  T leftPoint = ToScreen(line.point[0]);
  points.push_back(leftPoint);

  for (int i = 1; i <= last; ++i) {
    T pt = ToScreen(line.point[i]);
    if (pt.x <= leftPoint.x) {
      leftPoint = pt;
    }
    const T& prev_pt = points.back(); // Use reference to avoid copying
    if(lround(std::abs(prev_pt.x - pt.x) + std::abs(prev_pt.y - pt.y)) > 2) {
      points.push_back(std::move(pt));
    }
  }

  return leftPoint;
}
// Paint a single topology element

void Topology::Paint(ShapeSpecialRenderer& renderer, LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj) const {
  if (!shapefileopen) {
    return;
  }
  bool nolabels = false;
  // 130217 scaleCat 5 and 10 are the same! So careful..
  if (scaleCategory == 10 || scaleCategory == 5) {
    // for water areas, use scaleDefault
    if (MapWindow::zoom.RealScale() > scaleDefaultThreshold) {
      return;
    }
    // since we just checked category 10, if we are over scale we set nolabels
    if (MapWindow::zoom.RealScale() > scaleThreshold) {
      nolabels = true;
    }
  }
  else if (MapWindow::zoom.RealScale() > scaleThreshold) {
    return;
  }

#ifdef HAVE_GLES  
  using ScreenPoint = FloatPoint;
#else
  using ScreenPoint = RasterPoint;
#endif
  ShapePolygonRenderer shape_renderer(PolygonDrawCallback{Surface});
  shape_renderer.setClipRect(PixelRect(rc));
  shape_renderer.setNoLabel(nolabels);

  // TODO code: only draw inside screen!
  // this will save time with rendering pixmaps especially
  // checkVisible does only check lat lon , not screen pixels..
  // We need to check also screen.

  const auto hpOld = Surface.SelectObject(hPen);

  LKSurface::OldBrush hbOld{};
  if (hbBrush) {
    hbOld = Surface.SelectObject(hbBrush);
  }

  const auto hfOld = Surface.SelectObject(MapTopologyFont);

  // use the already existing screenbounds_latlon, calculated by CalculateScreenPositions in MapWindow2
  const rectObj screenRect = MapWindow::screenbounds_latlon;

  static std::vector<ScreenPoint> points;

  for (const auto& cshape : make_array(shpCache.get(), shpfile.numshapes)) {
    if (!cshape || cshape->hide) {
      continue;
    }

    const shapeObj& shape = cshape->shape;

    switch (shape.type) {
      case MS_SHAPE_POINT:
        if (checkVisible(shape, screenRect)) {
          for (const lineObj& line : make_array(shape.line, shape.numlines)) {
            for (const pointObj& point : make_array(line.point, line.numpoints)) {
              if (msPointInRect(&point, &screenRect)) {
                const POINT sc = _Proj.ToRasterPoint(point.y, point.x);
                if (cshape->renderSpecial(renderer, Surface, sc.x, sc.y, rc)) {
                  MapWindow::DrawBitmapIn(Surface, sc, hBitmap);
                }
              }
            }
          }
        }
        break;

      case MS_SHAPE_LINE:
        if (checkVisible(shape, screenRect)) {
          for (const lineObj& line : make_array(shape.line, shape.numlines)) {
            const ScreenPoint ptLabel = shape2Screen<ScreenPoint>(line, _Proj, points);
            Surface.Polyline(points.data(), points.size(), rc);
            cshape->renderSpecial(renderer, Surface, ptLabel.x, ptLabel.y, rc);
          }
        }
        break;

      case MS_SHAPE_POLYGON:
        // if it's a water area (nolabels), print shape up to defaultShape, but print
        // labels only up to custom label levels
        if (checkVisible(shape, screenRect)) {
          shape_renderer.renderPolygon(renderer, Surface, *cshape, hbBrush, _Proj);
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

void Topology::SearchNearest(const rectObj& bounds) {

  if (!shapefileopen) return;

  if(msRectOverlap(&shpfile.bounds, &bounds) != MS_TRUE) {
    return;
  }

  for (int ixshp = 0; ixshp < shpfile.numshapes; ixshp++) {
    XShapePtr cshape = shpCache[ixshp];
    if (!cshape) {
      if ((cache_mode == 1) &&
          (msRectOverlap(&shpBounds[ixshp], &bounds) != MS_TRUE)) {
        // if bounds is in cache and does not overlap no need to load shape;
        continue;
      }
      cshape = loadShape(ixshp);
    }

    if (!cshape || cshape->hide || !cshape->HasLabel()) {
      continue;
    }
    const shapeObj& shape = cshape->shape;

    if (msRectOverlap(&(cshape->shape.bounds), &bounds) != MS_TRUE) {
      continue;
    }

    switch (shape.type) {
      case (MS_SHAPE_POINT):
        for (int tt = 0; tt < shape.numlines; tt++) {
          for (int jj = 0; jj < shape.line[tt].numpoints; jj++) {
            cshape->nearestItem(scaleCategory, shape.line[tt].point[jj].x,
                                shape.line[tt].point[jj].y);
          }
        }
        break;

      case (MS_SHAPE_POLYGON):
        for (int tt = 0; tt < shape.numlines; tt++) {
          // TODO : check if that is good
          //  it's surprising distance to Polygon are not distance to first
          //  point but distance to nearest vertex. right implementation is in
          //  #msDistancePointToShape, but this don't use great circle ditance.
          cshape->nearestItem(scaleCategory, shape.line[tt].point[0].x,
                              shape.line[tt].point[0].y);
        }
        break;

      default:
        break;

    }  // switch type of shape
  }  // for all shapes in this category
}  // Topology SearchNearest
