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

#include "shapelib/mapshape.h"
#include "XShape.h"

namespace {

// Release the vector's backing allocation; clear() and resize(0) may keep
// capacity.
template <typename T>
void DeallocateVector(std::vector<T>& v) {
  std::vector<T> empty;
  empty.swap(v);
}

}  // namespace

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
    shpBounds.resize(shpfile.numshapes);
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
      DeallocateVector(shpBounds);
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

  DeallocateVector(shpBounds);
  try {
    allShapes.resize(shpfile.numshapes);
  }
  catch (std::exception&) {
    // Fallback to mode 0
    StartupStore(
        _T("------ WARN Topology,  malloc failed allShapes, fallback to mode 0"));
    return initCache_0();
  }
  // Load all shapes to allShapes 
  for (int i = 0; i < shpfile.numshapes; i++) {
    if (!(allShapes[i] = loadShape(i))) {
      StartupStore(_T("------ WARN Topology,  loadShape failed for allShapes[%d], fallback to mode 1"), i);
      DeallocateVector(allShapes); // Cleanup
      return initCache_1(); // fallback to mode 1 
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


  DeallocateVector(shpBounds);
  DeallocateVector(allShapes);
  in_scale_last = false;

  std::fill(visibleShapes.begin(), visibleShapes.end(), nullptr);

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
    visibleShapes.resize(shpfile.numshapes);
    initCache();
    shapefileopen = true;
  }
  catch (std::exception&) {
    msShapefileClose(&shpfile);
    StartupStore(_T("------ ERR Topology,  malloc failed visibleShapes, close shapefile <%s>"), from_utf8(filename).c_str());
  }
}

void Topology::Close() {
  if (shapefileopen) {
    DeallocateVector(visibleShapes);
    DeallocateVector(shpBounds);
    DeallocateVector(allShapes);
    msShapefileClose(&shpfile);
    shapefileopen = false;
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

void Topology::flushCache() {
  std::fill(visibleShapes.begin(), visibleShapes.end(), nullptr);
}

void Topology::flushOutOfBounds(const rectObj& thebounds) {
  std::replace_if(
      visibleShapes.begin(), visibleShapes.end(),
      [&](const auto& p) {
        return p && (msRectOverlap(&(p->shape.bounds), &thebounds) != MS_TRUE);
      },
      nullptr);
}

void Topology::updateCache(rectObj thebounds, bool purgeonly) {
  if (!triggerUpdateCache) return;

  if (!shapefileopen || visibleShapes.empty()) {
    return;
  }

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
        flushOutOfBounds(thebounds);
      }
      else { //In this case we have to run the original algoritm
        msShapefileWhichShapes(&shpfile, thebounds, 0);
        for (int i=0; i<shpfile.numshapes; i++) {
          if (msGetBit(shpfile.status, i)) {
            if (!visibleShapes[i]) {
              // shape is now in range, and wasn't before
              visibleShapes[i] = loadShape(i);
            }
          }
          else {
            visibleShapes[i] = nullptr;
          }
        }  // for
      }
      break;

    case 1:  // Bounds array in memory
      bigger = (msRectContained(&lastBounds, &thebounds) == MS_TRUE);
      smaller = (msRectContained(&thebounds, &lastBounds) == MS_TRUE);
      if (bigger || in_scale_again) { //We don't need to remove shapes, just load, so skip loaded ones
        for (int i=0; i<shpfile.numshapes; i++) {
          if (visibleShapes[i]) continue;
          if(msRectOverlap(&shpBounds[i], &thebounds) == MS_TRUE) {
            // shape is now in range, and wasn't before
            visibleShapes[i] = loadShape(i);
          }
        }  // for
      }
      else if (smaller) { //Search inside, we don't need to load additional shapes, just remove
        flushOutOfBounds(thebounds);
      }
      else {
        // Otherwise we have to search the all array
        for (int i = 0; i < shpfile.numshapes; i++) {
          if (msRectOverlap(&shpBounds[i], &thebounds) == MS_TRUE) {
            if (!visibleShapes[i]) {
              // shape is now in range, and wasn't before
              visibleShapes[i] = loadShape(i);
            }
          }
          else {
            visibleShapes[i] = nullptr;
          }
        }  // for
      }
      break;

    case 2: // All shapes in memory
      for (int i=0; i<shpfile.numshapes; i++) {
        const auto& pshp = allShapes[i];
        if(msRectOverlap(&(pshp->shape.bounds), &thebounds) == MS_TRUE) {
          visibleShapes[i] = pshp;
        }
        else {
          visibleShapes[i] = nullptr;
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

  const auto hfOld = Surface.SelectObject(MapTopologyFont);

  // use the already existing screenbounds_latlon, calculated by CalculateScreenPositions in MapWindow2
  const rectObj screenBounds = MapWindow::screenbounds_latlon;
  const PixelRect screenRect(rc);

  for (const auto& cshape : visibleShapes) {
    if (!cshape || cshape->hide) {
      continue;
    }
    if (!cshape->Overlap(screenBounds)) {
      continue;
    }

    cshape->Draw(renderer, Surface, screenRect, _Proj, hbBrush, hPen, hBitmap, !nolabels);
  }

  Surface.SelectObject(hfOld);
}

void Topology::SearchNearest(const rectObj& bounds) {

  if (!shapefileopen) return;

  if(msRectOverlap(&shpfile.bounds, &bounds) != MS_TRUE) {
    return;
  }

  for (int ixshp = 0; ixshp < shpfile.numshapes; ixshp++) {
    XShapePtr cshape = visibleShapes[ixshp];
    if (!cshape) {
      if ((cache_mode == 1) &&
          (msRectOverlap(&shpBounds[ixshp], &bounds) != MS_TRUE)) {
        // if bounds is in cache and does not overlap no need to load shape;
        continue;
      }
      cshape = loadShape(ixshp);
      visibleShapes[ixshp] = cshape;
    }

    if (!cshape || cshape->hide || !cshape->HasLabel()) {
      continue;
    }
    if (!cshape->Overlap(bounds)) {
      continue;
    }

    const shapeObj& shape = cshape->shape;

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
