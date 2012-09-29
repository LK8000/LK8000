/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Topology.cpp,v 8.5 2010/12/12 23:24:48 root Exp root $
*/

#include "externs.h"
#include <ctype.h> // needed for Wine
#include "Topology.h"



XShape::XShape() {
  hide=false;
}


XShape::~XShape() {
  clear();
}


void XShape::clear() {
  msFreeShape(&shape);
}


void XShape::load(shapefileObj* shpfile, int i) {
  msInitShape(&shape);
  msSHPReadShape(shpfile->hSHP, i, &shape);
}


void Topology::loadBitmap(const int xx) {
  hBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(xx));
}

// thecolor is relative to shapes, not to labels
// TODO (maybe): get rid of LKM internal colors defined per shape level,
// and assign them here dynamically. Easy and fast to do.
void Topology::loadPenBrush(const COLORREF thecolor) {
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
			case ss896x672:
				psize=3;
				break;
			//case ss480x272:
			//case ss480x234:
			//case ss240x320:
			//case ss272x480:
			default:
				psize=NIBLSCALE(1);
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
			case ss896x672:
				psize=3;
				break;
			//case ss480x234:
			//case ss240x320:
			//case ss272x480:
			default:
				psize=NIBLSCALE(1);
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
			case ss896x672:
				psize=2;
				break;
			//case ss480x234:
			//case ss400x240:
			//case ss320x240:
			//case ss240x320:
			//case ss272x480:
			default:
				psize=NIBLSCALE(1);
				break;
		}
		break;
	case 60: // railroads
		switch(ScreenSize) {
			//case ss800x480:
			//case ss640x480:
			//case ss896x672:
			//case ss480x272:
			//case ss480x234:
			//case ss400x240:
			//case ss320x240:
			//case ss480x640:
			//case ss480x800:
			//case ss240x320:
			//case ss272x480:
			default:
				psize=1;
		}
		break;

	default:
		psize=NIBLSCALE(1);
		break;
  }
  hPen = (HPEN)CreatePen(PS_SOLID, psize, thecolor);
  hbBrush=(HBRUSH)CreateSolidBrush(thecolor);

}

#if USETOPOMARKS
Topology::Topology(const TCHAR* shpname, bool doappend) {
#else
Topology::Topology(const TCHAR* shpname) {
#endif

  #if USETOPOMARKS
  append = doappend;
  #endif
  memset((void*)&shpfile, 0 ,sizeof(shpfile));
  shapefileopen = false;
  triggerUpdateCache = false;
  scaleThreshold = 0;
  shpCache= NULL;
  hBitmap = NULL;

#ifdef TOPOFASTCACHE
  shpBounds = NULL;
  shps = NULL;
  cache_mode = 0;
  lastBounds.minx = lastBounds.miny = lastBounds.maxx = lastBounds.maxy = 0;
#endif

  in_scale = false;

  // filename aleady points to _MAPS subdirectory!
  _tcscpy( filename, shpname ); 
  Open();
}


#ifdef TOPOFASTCACHE
void Topology::initCache()
{
  //Selecting caching scenarios based on available memory and topo size
  // Unfortunatelly I don't find a suitable algorithm to estimate the loaded
  // shapefile's memory footprint so we never choose mode2. KR
  long free_size = CheckFreeRam();
  long bounds_size = sizeof(rectObj)*shpfile.numshapes;

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
				for (int j=0; j<i; j++) delete(shps[i]);
				free(shps);
				shps=NULL;
				cache_mode = 0;
				break;
			}
		}
  } //sw
}
#endif

void Topology::Open() {
  shapefileopen = false;

  #if USETOPOMARKS
  if (append) {
    if (msSHPOpenFile(&shpfile, (char*)"rb+", filename) == -1) {
      // StartupStore(_T(". Topology: Open: append failed for <%s> (this can be normal)%s"),filename,NEWLINE);
      return;
    }
  } else {
  #endif
    if (msSHPOpenFile(&shpfile, (char*)"rb", filename) == -1) {
      StartupStore(_T("------ Topology: Open FAILED for <%s>%s"),filename,NEWLINE);
      return;
    }
  #if USETOPOMARKS
  }
  #endif
  // StartupStore(_T(". Topology: Open <%s>%s"),filename,NEWLINE);

  scaleThreshold = 1000.0;
  shpCache = (XShape**)malloc(sizeof(XShape*)*shpfile.numshapes);
  if (shpCache) {
    shapefileopen = true;
#ifdef TOPOFASTCACHE
	initCache();
#else
    for (int i=0; i<shpfile.numshapes; i++) {
      shpCache[i] = NULL;
    }
#endif
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
#ifdef TOPOFASTCACHE
    if (shpBounds) {
      free(shpBounds); shpBounds = NULL;
    }
    if (shps) {
	  for (int i=0; i<shpfile.numshapes; i++) {
		if (shps[i]) delete shps[i];
	  }
      free(shps); shps = NULL;
    }
#endif
    msSHPCloseFile(&shpfile);
    shapefileopen = false;  // added sgi
  }
}


Topology::~Topology() {
  Close();
  if (hPen) {
    DeleteObject((HPEN)hPen);
    DeleteObject((HBRUSH)hbBrush);    
  }
  if (hBitmap) {
    DeleteObject(hBitmap);
  }
}


bool Topology::CheckScale(void) {
  // Special case: all topology items are loaded ignoring their scaleThresholds
  if (LKSW_ForceNearestTopologyCalculation) {
	if ( scaleCategory==10|| (scaleCategory>=70 && scaleCategory<=100))
		return true;
	else
		return false;
  }

  if (scaleCategory==10)
	return (MapWindow::zoom.RealScale() <= scaleDefaultThreshold);
  else
	return (MapWindow::zoom.RealScale() <= scaleThreshold);
}

void Topology::TriggerIfScaleNowVisible(void) {
  triggerUpdateCache |= (CheckScale() != in_scale);
}

void Topology::flushCache() {
#ifdef DEBUG_TFC
  StartupStore(TEXT("---flushCache() starts%s"),NEWLINE);
  int starttick = GetTickCount();
#endif
#ifdef TOPOFASTCACHE
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
#else
  for (int i=0; i<shpfile.numshapes; i++) {
    removeShape(i);
  }
#endif
  shapes_visible_count = 0;
#ifdef DEBUG_TFC
  StartupStore(TEXT("   flushCache() ends (%dms)%s"),GetTickCount()-starttick,NEWLINE);
#endif
}

void Topology::updateCache(rectObj thebounds, bool purgeonly) {
  if (!triggerUpdateCache) return;

  if (!shapefileopen) return;

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
#ifdef TOPOFASTCACHE
  StartupStore(TEXT("---UpdateCache() starts, mode%d%s"),cache_mode,NEWLINE);
#else
  StartupStore(TEXT("---UpdateCache() starts, original code%s"),NEWLINE);
#endif
  int starttick = GetTickCount();
#endif

#ifdef TOPOFASTCACHE
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
        msSHPWhichShapes(&shpfile, thebounds, 0);
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
#else

  msSHPWhichShapes(&shpfile, thebounds, 0);
  if (!shpfile.status) {
    // this happens if entire shape is out of range
    // so clear buffer.
    flushCache();
    return;
  }

  shapes_visible_count = 0;

  for (int i=0; i<shpfile.numshapes; i++) {

    if (msGetBit(shpfile.status, i)) {
      
      if (shpCache[i]==NULL) {
	// shape is now in range, and wasn't before
	shpCache[i] = addShape(i);
      }
      shapes_visible_count++;
    } else {
      removeShape(i);
    }
  }
#endif

#ifdef DEBUG_TFC
  long free_size = CheckFreeRam();
  StartupStore(TEXT("   UpdateCache() ends, shps_visible=%d ram=%li (%dms)%s"),shapes_visible_count, free_size, GetTickCount()-starttick,NEWLINE);
#endif
}


XShape* Topology::addShape(const int i) {
  XShape* theshape = new XShape();
  LKASSERT(theshape);
  theshape->load(&shpfile,i);
  return theshape;
}


void Topology::removeShape(const int i) {
  if (shpCache[i]) {
    delete shpCache[i];
    shpCache[i]= NULL;
  }
}


// This is checking boundaries based on lat/lon values. 
// It is not enough for screen overlapping verification.
bool Topology::checkVisible(shapeObj& shape, rectObj &screenRect) {
  return (msRectOverlap(&shape.bounds, &screenRect) == MS_TRUE);
}


// Paint a single topology element

void Topology::Paint(HDC hdc, RECT rc) {

  if (!shapefileopen) return;

  bool nolabels=false;
  if (scaleCategory==10) {
	// for water areas, use scaleDefault
	if ( MapWindow::zoom.RealScale()>scaleDefaultThreshold) {
		return;
	}
	// since we just checked category 10, if we are over scale we set nolabels
	if ( MapWindow::zoom.RealScale()>scaleThreshold) nolabels=true;
  } else 
  if (MapWindow::zoom.RealScale() > scaleThreshold) return;

  // TODO code: only draw inside screen!
  // this will save time with rendering pixmaps especially
  // checkVisible does only check lat lon , not screen pixels..
  // We need to check also screen.

  HPEN  hpOld;
  HBRUSH hbOld;
  HFONT hfOld;

  if (hPen) {
    hpOld = (HPEN)SelectObject(hdc,  hPen);
    hbOld = (HBRUSH)SelectObject(hdc, hbBrush);
  } else {
    hpOld = NULL;
    hbOld = NULL;
  }
  hfOld = (HFONT)SelectObject(hdc, MapLabelFont);

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

  #if TOPOFASTLABEL
  // use the already existing screenbounds_latlon, calculated by CalculateScreenPositions in MapWindow2
  rectObj screenRect = MapWindow::screenbounds_latlon;
  #else
  rectObj screenRect = MapWindow::CalculateScreenBounds(0.0);
  #endif

  static POINT pt[MAXCLIPPOLYGON];
  bool labelprinted=false;

  for (int ixshp = 0; ixshp < shpfile.numshapes; ixshp++) {
    
    XShape *cshape = shpCache[ixshp];

    if (!cshape || cshape->hide) continue;    

    shapeObj *shape = &(cshape->shape);

    switch(shape->type) {


      case(MS_SHAPE_POINT):{

	#if 101016
	// -------------------------- NOT PRINTING ICONS ---------------------------------------------
	bool dobitmap=false;
	if (scaleCategory<90 || (MapWindow::zoom.RealScale()<2)) dobitmap=true;
	// first a latlon overlap check, only approximated because of fastcosine in latlon2screen
	if (checkVisible(*shape, screenRect))
		for (int tt = 0; tt < shape->numlines; tt++) {
			for (int jj=0; jj< shape->line[tt].numpoints; jj++) {
				POINT sc;
				MapWindow::LatLon2Screen(shape->line[tt].point[jj].x, shape->line[tt].point[jj].y, sc);
				if (dobitmap) {
					// bugfix 101212 missing case for scaleCategory 0 (markers)
					if (scaleCategory==0||cshape->renderSpecial(hdc, sc.x, sc.y,labelprinted)) 
						MapWindow::DrawBitmapIn(hdc, sc, hBitmap,true);
				} else {
					cshape->renderSpecial(hdc, sc.x, sc.y,labelprinted);
				}
			}
		}
	}

	#else
	// -------------------------- PRINTING ICONS ---------------------------------------------
	#if (TOPOFAST)
	// no bitmaps for small town over a certain zoom level and no bitmap if no label at all levels
	bool nobitmap=false, noiconwithnolabel=false;
	if (scaleCategory==90 || scaleCategory==100) {
		noiconwithnolabel=true;
		if (MapWindow::MapScale>4) nobitmap=true;
	}
	#endif

	//#if TOPOFASTLABEL
	if (checkVisible(*shape, screenRect))
		for (int tt = 0; tt < shape->numlines; tt++) {
			for (int jj=0; jj< shape->line[tt].numpoints; jj++) {
				POINT sc;
				MapWindow::LatLon2Screen(shape->line[tt].point[jj].x, shape->line[tt].point[jj].y, sc);
	
				#if (TOPOFAST)
				if (!nobitmap)
				#endif
				#if 101016
				// only paint icon if label is printed too
				if (noiconwithnolabel) {
					if (cshape->renderSpecial(hdc, sc.x, sc.y,labelprinted))
						MapWindow::DrawBitmapIn(hdc, sc, hBitmap,true);
				} else {
					MapWindow::DrawBitmapIn(hdc, sc, hBitmap,true);
					cshape->renderSpecial(hdc, sc.x, sc.y,labelprinted);
				}
				#else
				MapWindow::DrawBitmapIn(hdc, sc, hBitmap,true);
				cshape->renderSpecial(hdc, sc.x, sc.y);
				#endif
			}
		}

	}
	#endif // Use optimized point icons 1.23e
	break;

    case(MS_SHAPE_LINE):

      if (checkVisible(*shape, screenRect))
        for (int tt = 0; tt < shape->numlines; tt ++) {
          
          int minx = rc.right;
          int miny = rc.bottom;
          int msize = min(shape->line[tt].numpoints, MAXCLIPPOLYGON);

	  MapWindow::LatLon2Screen(shape->line[tt].point,
				   pt, msize, 1);
          for (int jj=0; jj< msize; jj++) {
            if (pt[jj].x<=minx) {
              minx = pt[jj].x;
              miny = pt[jj].y;
            }
	  }

          ClipPolygon(hdc, pt, msize, rc, false);
          cshape->renderSpecial(hdc,minx,miny,labelprinted);
        }
      break;
      
    case(MS_SHAPE_POLYGON):

	// if it's a water area (nolabels), print shape up to defaultShape, but print
	// labels only up to custom label levels
	if ( nolabels ) {
		if (checkVisible(*shape, screenRect)) {
			for (int tt = 0; tt < shape->numlines; tt ++) {
				int minx = rc.right;
				int msize = min(shape->line[tt].numpoints/iskip, MAXCLIPPOLYGON);
				MapWindow::LatLon2Screen(shape->line[tt].point, pt, msize*iskip, iskip);
				for (int jj=0; jj< msize; jj++) {
					if (pt[jj].x<=minx) {
						minx = pt[jj].x;
					}
				}
				ClipPolygon(hdc,pt, msize, rc, true);
			}
		}
	} else 
	if (checkVisible(*shape, screenRect)) {
		for (int tt = 0; tt < shape->numlines; tt ++) {
			int minx = rc.right;
			int miny = rc.bottom;
			int msize = min(shape->line[tt].numpoints/iskip, MAXCLIPPOLYGON);
			MapWindow::LatLon2Screen(shape->line[tt].point, pt, msize*iskip, iskip);
			for (int jj=0; jj< msize; jj++) {
				if (pt[jj].x<=minx) {
					minx = pt[jj].x;
					miny = pt[jj].y;
				}
			}
			ClipPolygon(hdc,pt, msize, rc, true);
			cshape->renderSpecial(hdc,minx,miny,labelprinted);          
		}
	}
	break;
      
    default:
      break;
    }
  }
  if (hpOld) {
    SelectObject(hdc, hbOld);
    SelectObject(hdc, hpOld);
  }
  SelectObject(hdc, (HFONT)hfOld);

}



TopologyLabel::TopologyLabel(const TCHAR* shpname, int field1):Topology(shpname) 
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
  theshape->setlabel(msDBFReadStringAttribute( shpfile.hDBF, i, field));
  return theshape;
}

bool XShapeLabel::nearestItem(int category, double lon, double lat) {

  NearestTopoItem *item;
  TCHAR Temp[100];
  int size = MultiByteToWideChar(CP_ACP, 0, label, -1, Temp, 100) - 1;
  if (size <= 0) return false;	

  switch(category) {
	case 10:
		item=&NearestWaterArea;
		break;
	case 70:
	case 80:
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
		StartupStore(_T("...... Cannot use nearestItem cat=%d <%s>%s"),category,Temp,NEWLINE);
		#endif
		return false;
		break;
  }

  Temp[MAXNEARESTTOPONAME-1]=_T('\0');

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
	_tcscpy(item->Name,Temp);
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
bool XShapeLabel::renderSpecial(HDC hDC, int x, int y, bool retval) {
  if (label && ((MapWindow::DeclutterLabels==MAPLABELS_ALLON)||(MapWindow::DeclutterLabels==MAPLABELS_ONLYTOPO))) {

	TCHAR Temp[100];
	int size = MultiByteToWideChar(CP_ACP, 0, label, -1, Temp, 100) - 1;
	//Do not waste time with null labels
	if (size <= 0) return false;						

	SetBkMode(hDC,TRANSPARENT);

	SIZE tsize;
	RECT brect;
	GetTextExtentPoint(hDC, Temp, size, &tsize);

	// shift label from center point of shape
	x+= NIBLSCALE(2); 
	y+= NIBLSCALE(2);
	brect.left = x-NIBLSCALE(3);
	brect.right = brect.left+tsize.cx+NIBLSCALE(3);
	brect.top = y-NIBLSCALE(3);
	brect.bottom = brect.top+tsize.cy+NIBLSCALE(3);

	if ( DeclutterMode != (DeclutterMode_t)dmDisabled ) {
		#if TOPOFASTLABEL
		if (!MapWindow::checkLabelBlock(&brect)) return false;
		#else
		if (!MapWindow::checkLabelBlock(brect)) return false;
		#endif
	}

	// Do not print outside boundaries of Draw area
	if (brect.bottom>=MapWindow::DrawRect.bottom ||
		brect.top<=MapWindow::DrawRect.top ||
		brect.left<=MapWindow::DrawRect.left ||
		brect.right>=MapWindow::DrawRect.right) return false;

	SetTextColor(hDC, RGB(0,50,50)); // PETROL too light at 66
    
	ExtTextOut(hDC, x, y, 0, NULL, Temp, size, NULL);
	return true; // 101016
  }
  return false; // 101016
}


void XShapeLabel::setlabel(const char* src) {
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
  if (label) free(label);
  label = (char*)malloc(strlen(src)+1);
  if (label) {
	strcpy(label,src);
  }
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

//       wsprintf(Scale,TEXT("%1.2f%c"),MapScale, autozoomstring);

// //////////////////////////////////////////////////////////////
// TOPOLOGY WRITER USED ONLY BY OLD MARKERS, not by LK anymore
// //////////////////////////////////////////////////////////////
#if USETOPOMARKS
//
TopologyWriter::~TopologyWriter() {
  if (shapefileopen) {
    Close();
    DeleteFiles();
  }
}


TopologyWriter::TopologyWriter(const TCHAR* shpname, COLORREF thecolor):
  Topology(shpname, thecolor, true) {

  Reset();
}

void TopologyWriter::DeleteFiles(void) {
  // Delete all files, since zziplib interface doesn't handle file modes
  // properly
  if (_tcslen(filename)>0) {
    TCHAR fname[MAX_PATH];
    _tcscpy(fname, filename);
    _tcscat(fname, _T(".shp"));
    DeleteFile(fname);
    _tcscpy(fname, filename);
    _tcscat(fname, _T(".shx"));
    DeleteFile(fname);
    _tcscpy(fname, filename);
    _tcscat(fname, _T(".dbf"));
    DeleteFile(fname);
  }
}

void TopologyWriter::CreateFiles(void) {
  // by default, now, this overwrites previous contents
  if (msSHPCreateFile(&shpfile, filename, SHP_POINT) == -1) {
  } else {
    TCHAR dbfname[100];
    _tcscpy(dbfname, filename);
    _tcscat(dbfname, _T(".dbf")); 
    shpfile.hDBF = msDBFCreate(dbfname);
    if (shpfile.hDBF == NULL)
	StartupStore(_T("------ TopologyWriter: msDBFCreate error%s"),NEWLINE);

    shapefileopen=true;
    Close();
  }
}

void TopologyWriter::Reset(void) {
  if (shapefileopen) {
    Close();
  }
  DeleteFiles();
  CreateFiles();
  Open();
}

void TopologyWriter::addPoint(double x, double y) {
  pointObj p = {x,y};

  if (shapefileopen) {
    msSHPWritePoint(shpfile.hSHP, &p);
    Close();
  }
  Open();

}
#endif // USETOPOMARKS
// //////////////////////////////////////////////////////////////



// The "OutputToInput" function sets the resulting polygon of this
// step up to be the input polygon for next step of the clipping
// algorithm. As the Sutherland-Hodgman algorithm is a polygon
// clipping algorithm, it does not handle line clipping very well. The
// modification so that lines may be clipped as well as polygons is
// included in this function. The code for this function is:

static void OutputToInput(unsigned int *inLength, 
			  POINT *inVertexArray, 
			  unsigned int *outLength, 
			  POINT *outVertexArray )
{ 
  if ((*inLength==2) && (*outLength==3)) //linefix
    {
      inVertexArray[0].x=outVertexArray [0].x;
      inVertexArray[0].y=outVertexArray [0].y;
      if ((outVertexArray[0].x==outVertexArray[1].x) 
          && (outVertexArray[0].y==outVertexArray[1].y)) /*First two vertices 
                                                      are same*/
        {
          inVertexArray[1].x=outVertexArray [2].x;
          inVertexArray[1].y=outVertexArray [2].y;
        }
      else                    /*First vertex is same as third vertex*/
        {
          inVertexArray[1].x=outVertexArray [1].x;
          inVertexArray[1].y=outVertexArray [1].y;
        }
      
      *inLength=2;
      
    } 
  else  /* set the outVertexArray as inVertexArray for next step*/
    {
      *inLength= *outLength;
      memcpy((void*)inVertexArray, (void*)outVertexArray, 
             (*outLength)*sizeof(POINT));
    }
}


// The "Inside" function returns TRUE if the vertex tested is on the
// inside of the clipping boundary. "Inside" is defined as "to the
// left of clipping boundary when one looks from the first vertex to
// the second vertex of the clipping boundary". The code for this
// function is:

/*
static bool Inside (const POINT *testVertex, const POINT *clipBoundary)
{
  if (clipBoundary[1].x > clipBoundary[0].x)              // bottom edge
    if (testVertex->y <= clipBoundary[0].y) return TRUE;
  if (clipBoundary[1].x < clipBoundary[0].x)              // top edge
   if (testVertex->y >= clipBoundary[0].y) return TRUE;
  if (clipBoundary[1].y < clipBoundary[0].y)              // right edge
    if (testVertex->x <= clipBoundary[1].x) return TRUE;
  if (clipBoundary[1].y > clipBoundary[0].y)              // left edge
    if (testVertex->x >= clipBoundary[1].x) return TRUE;
  return FALSE;
}
*/

#define INSIDE_LEFT_EDGE(a,b)   (a->x >= b[1].x)
#define INSIDE_BOTTOM_EDGE(a,b) (a->y <= b[0].y)
#define INSIDE_RIGHT_EDGE(a,b)  (a->x <= b[1].x)
#define INSIDE_TOP_EDGE(a,b)    (a->y >= b[0].y)

// The "Intersect" function calculates the intersection of the polygon
// edge (vertex s to p) with the clipping boundary. The code for this
// function is:


static bool Intersect (const POINT &first, const POINT &second, 
		       const POINT *clipBoundary,
		       POINT *intersectPt)
{
  float f;
  if (clipBoundary[0].y==clipBoundary[1].y)     /*horizontal*/
   {
     intersectPt->y=clipBoundary[0].y;
     if (second.y != first.y) {
       f = ((float)(second.x-first.x))/((float)(second.y-first.y));
       intersectPt->x= first.x + (long)(((clipBoundary[0].y-first.y)*f));
       return true;
     }
   } else { /*Vertical*/
    intersectPt->x=clipBoundary[0].x;
    if (second.x != first.x) {
      f = ((float)(second.y-first.y))/((float)(second.x-first.x));
      intersectPt->y=first.y + (long)(((clipBoundary[0].x-first.x)*f));
      return true;
    } 
  }
  return false; // no need to add point!
}


// The "Output" function moves "newVertex" to "outVertexArray" and
// updates "outLength".

static void Output(const POINT *newVertex, 
		   unsigned int *outLength, POINT *outVertexArray)
{
  if (*outLength) {
    if ((newVertex->x == outVertexArray[*outLength-1].x)
        &&(newVertex->y == outVertexArray[*outLength-1].y)) {
      // no need for duplicates
      return;
    }
  }
  outVertexArray[*outLength].x= newVertex->x;
  outVertexArray[*outLength].y= newVertex->y;
  (*outLength)++;
}


static bool ClipEdge(const bool &s_inside, 
		     const bool &p_inside, 
		     const POINT *clipBoundary, 
		     POINT *outVertexArray, 
		     const POINT *s,
		     const POINT *p,
		     unsigned int *outLength,
		     const bool fill) {

  if (fill) {
    if (p_inside && s_inside) {
      // case 1, save endpoint p
      return true;
    } else if (p_inside != s_inside) {
      POINT i;
      if (Intersect(*s, *p, clipBoundary, &i)) {
	Output(&i, outLength, outVertexArray);
      }
      // case 4, save intersection and endpoint
      // case 2, exit visible, save intersection i
      return p_inside;
    } else {
      // case 3, both outside, save nothing
      return false;
    }
  } else {
    if (p_inside) {
      return true;
    }
  }
  return false;
}


static unsigned int SutherlandHodgmanPolygoClip (POINT* inVertexArray,
						 POINT* outVertexArray,
						 const unsigned int inLength,
						 const POINT *clipBoundary, 
						 const bool fill,
						 const int mode)
{
  POINT *s, *p; /*Start, end point of current polygon edge*/ 
  unsigned int j;       /*Vertex loop counter*/
  unsigned int outLength = 0;

  if (inLength<1) return 0;

  s = inVertexArray + inLength-1;
  p = inVertexArray;

  bool s_inside, p_inside;

  /*Start with the last vertex in inVertexArray*/
  switch (mode) {
  case 0:
    for (j=inLength; j--; ) {
      s_inside = INSIDE_LEFT_EDGE(s,clipBoundary);
      p_inside = INSIDE_LEFT_EDGE(p,clipBoundary);
      /*Now s and p correspond to the vertices*/
      if (ClipEdge(s_inside, 
		   p_inside, 
		   clipBoundary, outVertexArray, s, p,
		   &outLength, fill || (p != inVertexArray))) {
	Output(p, &outLength, outVertexArray);
      }
      /*Advance to next pair of vertices*/
      s = p; p++;
    }
    break;
  case 1:
    for (j=inLength; j--; ) {
      s_inside = INSIDE_BOTTOM_EDGE(s,clipBoundary);
      p_inside = INSIDE_BOTTOM_EDGE(p,clipBoundary);
      if (ClipEdge(s_inside, p_inside, clipBoundary, outVertexArray, s, p,
		   &outLength, fill || (p != inVertexArray))) {
	Output(p, &outLength, outVertexArray);
      }
      s = p; p++;
    }
    break;
  case 2:
    for (j=inLength; j--; ) {
      s_inside = INSIDE_RIGHT_EDGE(s,clipBoundary);
      p_inside = INSIDE_RIGHT_EDGE(p,clipBoundary);
      if (ClipEdge(s_inside, p_inside, clipBoundary, outVertexArray, s, p,
		   &outLength, fill || (p != inVertexArray))) {
	Output(p, &outLength, outVertexArray);
      }
      s = p; p++;
    }
    break;
  case 3:
    for (j=inLength; j--; ) {
      s_inside = INSIDE_TOP_EDGE(s,clipBoundary);
      p_inside = INSIDE_TOP_EDGE(p,clipBoundary);
      if (ClipEdge(s_inside, p_inside, clipBoundary, outVertexArray, s, p,
		   &outLength, fill || (p != inVertexArray))) {
	Output(p, &outLength, outVertexArray);
      }
      s = p; p++;
    }
    break;
  }
  return outLength;
}    


// Classic traditional Clipping function using Sutherland Hodgman algo
static POINT clip_ptout[MAXCLIPPOLYGON];
static POINT clip_ptin[MAXCLIPPOLYGON];


void ClipPolygon(HDC hdc, POINT *m_ptin, unsigned int inLength, 
                 RECT rc, bool fill) {
  unsigned int outLength = 0;

  if (inLength>=MAXCLIPPOLYGON-1) {
    inLength=MAXCLIPPOLYGON-2;
  }
  if (inLength<2) {
    return;
  }

  memcpy((void*)clip_ptin, (void*)m_ptin, inLength*sizeof(POINT));

  // add extra point for final point if it doesn't equal the first
  // this is required to close some airspace areas that have missing
  // final point
  if (fill) {
    if ((m_ptin[inLength-1].x != m_ptin[0].x) &&
	(m_ptin[inLength-1].y != m_ptin[0].y)) {
      clip_ptin[inLength] = clip_ptin[0];
      inLength++;
    }
  }

  // PAOLO NOTE: IF CLIPPING WITH N>2 DOESN'T WORK,
  // TRY IFDEF'ing out THE FOLLOWING ADJUSTMENT TO THE CLIPPING RECTANGLE
  rc.top--;
  rc.bottom++;
  rc.left--;
  rc.right++;

  // OK, do the clipping
  POINT edge[5] = {{rc.left, rc.top}, 
		   {rc.left, rc.bottom},
		   {rc.right, rc.bottom},
		   {rc.right, rc.top},
		   {rc.left, rc.top}};
  //steps left_edge, bottom_edge, right_edge, top_edge
  for (int step=0; step<4; step++) {
    outLength = SutherlandHodgmanPolygoClip (clip_ptin, clip_ptout, 
					     inLength, 
					     edge+step, fill, step);
    OutputToInput(&inLength, clip_ptin, &outLength, clip_ptout);
  }

  if (fill) {
    if (outLength>2) {
      Polygon(hdc, clip_ptout, outLength);
    }
  } else {
    if (outLength>1) {
      Polyline(hdc, clip_ptout, outLength);
    }
  }
}



void Topology::SearchNearest(RECT rc) {

  if (!shapefileopen) return;

  int iskip = 1;
  rectObj screenRect = MapWindow::screenbounds_latlon;
  static POINT pt[MAXCLIPPOLYGON];

  for (int ixshp = 0; ixshp < shpfile.numshapes; ixshp++) {
    
	XShape *cshape = shpCache[ixshp];
	if (!cshape || cshape->hide) continue;    
	shapeObj *shape = &(cshape->shape);

	switch(shape->type) {

	   case(MS_SHAPE_POINT):

		if (checkVisible(*shape, screenRect)) {
			for (int tt = 0; tt < shape->numlines; tt++) {
				for (int jj=0; jj< shape->line[tt].numpoints; jj++) {
					cshape->nearestItem(scaleCategory, shape->line[tt].point[jj].x, shape->line[tt].point[jj].y);
				}
			}
		}
		break;

	   case(MS_SHAPE_LINE):
/*
		if (checkVisible(*shape, screenRect)) {
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
		}
*/
		break;
      
	   case(MS_SHAPE_POLYGON):

		if (checkVisible(*shape, screenRect)) {
			for (int tt = 0; tt < shape->numlines; tt ++) {
				int minx = rc.right;
				int msize = min(shape->line[tt].numpoints/iskip, MAXCLIPPOLYGON);

				MapWindow::LatLon2Screen(shape->line[tt].point, pt, msize*iskip, iskip);

				for (int jj=0; jj< msize; jj++) {
					if (pt[jj].x<=minx) {
						minx = pt[jj].x;
					}
				}

	  			cshape->nearestItem(scaleCategory, shape->line[tt].point[0].x, shape->line[tt].point[0].y);
			}
		}
		break;
      
	   default:
		break;

    } // switch type of shape
  } // for all shapes in this category
} // Topology SearchNearest

