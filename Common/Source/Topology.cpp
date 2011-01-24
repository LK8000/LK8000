/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Topology.cpp,v 8.5 2010/12/12 23:24:48 root Exp root $
*/

#include "StdAfx.h"
#include <ctype.h> // needed for Wine
#include "Topology.h"
#include "options.h"
#include "externs.h"
#include "wcecompat/ts_string.h"

extern HFONT MapLabelFont;

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
Topology::Topology(const char* shpname, COLORREF thecolor, bool doappend) {

  append = doappend;
  memset((void*)&shpfile, 0 ,sizeof(shpfile));
  shapefileopen = false;
  triggerUpdateCache = false;
  scaleThreshold = 0;
  shpCache= NULL;
  hBitmap = NULL;

  in_scale = false;

  // filename aleady points to _MAPS subdirectory!
  strcpy( filename, shpname ); 
  hPen = (HPEN)CreatePen(PS_SOLID, 1, thecolor);
  hbBrush=(HBRUSH)CreateSolidBrush(thecolor);
  Open();
}


void Topology::Open() {

  TCHAR ufname[MAX_PATH]; // 091105
  shapefileopen = false;

  if (append) {
    if (msSHPOpenFile(&shpfile, (char*)"rb+", filename) == -1) {
	ascii2unicode(filename,ufname);
	// StartupStore(_T(". Topology: Open: append failed for <%s> (this can be normal)%s"),ufname,NEWLINE);
      return;
    }
  } else {
    if (msSHPOpenFile(&shpfile, (char*)"rb", filename) == -1) {
	ascii2unicode(filename,ufname);
	StartupStore(_T("------ Topology: Open FAILED for <%s>%s"),ufname,NEWLINE);
      return;
    }
  }
  ascii2unicode(filename,ufname); // 091105
  // StartupStore(_T(". Topology: Open <%s>%s"),ufname,NEWLINE);

  scaleThreshold = 1000.0;
  shpCache = (XShape**)malloc(sizeof(XShape*)*shpfile.numshapes);
  if (shpCache) {
    shapefileopen = true;
    for (int i=0; i<shpfile.numshapes; i++) {
      shpCache[i] = NULL;
    }
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
    msSHPCloseFile(&shpfile);
    shapefileopen = false;  // added sgi
  }
}


Topology::~Topology() {
  Close();
  DeleteObject((HPEN)hPen);
  DeleteObject((HBRUSH)hbBrush);    
  if (hBitmap) {
    DeleteObject(hBitmap);
  }
}


bool Topology::CheckScale(void) {
  #if LKTOPO
  if (scaleCategory==10)
	return (MapWindow::MapScale <= scaleDefaultThreshold);
  else
  #endif
  return (MapWindow::MapScale <= scaleThreshold);
}

void Topology::TriggerIfScaleNowVisible(void) {
  triggerUpdateCache |= (CheckScale() != in_scale);
}

void Topology::flushCache() {
  for (int i=0; i<shpfile.numshapes; i++) {
    removeShape(i);
  }
  shapes_visible_count = 0;
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
    return;
  }

  if (purgeonly) return;

  triggerUpdateCache = false;
 
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
}


XShape* Topology::addShape(const int i) {
  XShape* theshape = new XShape();
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

  #if LKTOPO
  bool nolabels=false;
  if (scaleCategory==10) {
	// for water areas, use scaleDefault
	if ( MapWindow::MapScale>scaleDefaultThreshold) {
		return;
	}
	// since we just checked category 10, if we are over scale we set nolabels
	if ( MapWindow::MapScale>scaleThreshold) nolabels=true;
  } else 
  #endif
  if (MapWindow::MapScale > scaleThreshold) return;

  // TODO code: only draw inside screen!
  // this will save time with rendering pixmaps especially
  // checkVisible does only check lat lon , not screen pixels..
  // We need to check also screen.

  HPEN  hpOld;
  HBRUSH hbOld;
  HFONT hfOld;

  hpOld = (HPEN)SelectObject(hdc,hPen);
  hbOld = (HBRUSH)SelectObject(hdc, hbBrush);
  hfOld = (HFONT)SelectObject(hdc, MapLabelFont);

  // get drawing info
    
  int iskip = 1;
  
#if LKTOPO
  // attempt to bugfix 100615 polyline glitch with zoom over 33Km
  // do not skip points, if drawing coast lines which have a scaleThreshold of 100km!
  // != 5 and != 10
  if (scaleCategory>10) { 
#endif
  if (MapWindow::MapScale>0.25*scaleThreshold) {
    iskip = 2;
  } 
  if (MapWindow::MapScale>0.5*scaleThreshold) {
    iskip = 3;
  }
  if (MapWindow::MapScale>0.75*scaleThreshold) {
    iskip = 4;
  }
#if LKTOPO
  }
#endif

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
	if (scaleCategory<90 || (MapWindow::MapScale<2)) dobitmap=true;
	// first a latlon overlap check, only approximated because of fastcosine in latlon2screen
	if (checkVisible(*shape, screenRect))
		for (int tt = 0; tt < shape->numlines; tt++) {
			for (int jj=0; jj< shape->line[tt].numpoints; jj++) {
				POINT sc;
				MapWindow::LatLon2Screen(shape->line[tt].point[jj].x, shape->line[tt].point[jj].y, sc);
				if (dobitmap) {
					// bugfix 101212 missing case for scaleCategory 0 (markers)
					if (scaleCategory==0||cshape->renderSpecial(hdc, sc.x, sc.y,labelprinted)) 
						MapWindow::DrawBitmapIn(hdc, sc, hBitmap);
				} else {
					cshape->renderSpecial(hdc, sc.x, sc.y,labelprinted);
				}
			}
		}
	}

	#else
	// -------------------------- PRINTING ICONS ---------------------------------------------
	#if (LKTOPO && TOPOFAST)
	#if 101016
	// no bitmaps for small town over a certain zoom level and no bitmap if no label at all levels
	bool nobitmap=false, noiconwithnolabel=false;
	if (scaleCategory==90 || scaleCategory==100) {
		noiconwithnolabel=true;
		if (MapWindow::MapScale>4) nobitmap=true;
	}
	#else
	// do not print bitmaps for small town over a certain zoom level
	bool nobitmap=false;
	if (scaleCategory==90 && (MapWindow::MapScale>4))
		nobitmap=true;
	else 
	if (scaleCategory==100 && (MapWindow::MapScale>4)) nobitmap=true;
	#endif
	#endif

	//#if TOPOFASTLABEL
	if (checkVisible(*shape, screenRect))
		for (int tt = 0; tt < shape->numlines; tt++) {
			for (int jj=0; jj< shape->line[tt].numpoints; jj++) {
				POINT sc;
				MapWindow::LatLon2Screen(shape->line[tt].point[jj].x, shape->line[tt].point[jj].y, sc);
	
				#if (LKTOPO && TOPOFAST)
				if (!nobitmap)
				#endif
				#if 101016
				// only paint icon if label is printed too
				if (noiconwithnolabel) {
					if (cshape->renderSpecial(hdc, sc.x, sc.y,labelprinted))
						MapWindow::DrawBitmapIn(hdc, sc, hBitmap);
				} else {
					MapWindow::DrawBitmapIn(hdc, sc, hBitmap);
					cshape->renderSpecial(hdc, sc.x, sc.y,labelprinted);
				}
				#else
				MapWindow::DrawBitmapIn(hdc, sc, hBitmap);
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

	#if LKTOPO
	// if it's a water area (nolabels), print shape up to defaultShape, but print
	// labels only up to custom label levels
	if ( nolabels ) {
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
			}
		}
	} else 
	#endif
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
        
  SelectObject(hdc, hbOld);
  SelectObject(hdc, hpOld);
  SelectObject(hdc, (HFONT)hfOld);

}



TopologyLabel::TopologyLabel(const char* shpname, COLORREF thecolor, int field1):Topology(shpname, thecolor) 
{
  //sjt 02nov05 - enabled label fields
  setField(max(0,field1)); 
  // JMW this is causing XCSoar to crash on my system!
};

TopologyLabel::~TopologyLabel()
{
}


void TopologyLabel::setField(int i) {
  field = i;
}

XShape* TopologyLabel::addShape(const int i) {

  XShapeLabel* theshape = new XShapeLabel();
  theshape->load(&shpfile,i);
  theshape->setlabel(msDBFReadStringAttribute( shpfile.hDBF, i, field));
  return theshape;
}

// Print topology labels
bool XShapeLabel::renderSpecial(HDC hDC, int x, int y, bool retval) {
#if LKTOPO
  if (label && ((MapWindow::DeclutterLabels==MAPLABELS_ALLON)||(MapWindow::DeclutterLabels==MAPLABELS_ONLYTOPO))) {
#else
  if (label && (MapWindow::DeclutterLabels<MAPLABELS_ALLOFF)) {
#endif

	TCHAR Temp[100];
	int size = MultiByteToWideChar(CP_ACP, 0, label, -1, Temp, 100) - 1;			//ANSI to UNICODE
	if (size <= 0) return false;													//Do not waste time with null labels

	SetBkMode(hDC,TRANSPARENT);

	#ifndef LK8000_OPTIMIZE
	if (ispunct(Temp[0])) {
		double dTemp;
      
		Temp[0]='0';
		dTemp = StrToDouble(Temp,NULL);
		dTemp = ALTITUDEMODIFY*dTemp;
		if (dTemp > 999)
			wsprintf(Temp,TEXT("%.1f"),(dTemp/1000));
		else
			wsprintf(Temp,TEXT("%d"),int(dTemp));
	}
	#endif

	SIZE tsize;
	RECT brect;
	GetTextExtentPoint(hDC, Temp, size, &tsize);

	if ( DeclutterMode != (DeclutterMode_t)dmDisabled ) {
		// shift label from center point of shape
		x+= NIBLSCALE(2); 
		y+= NIBLSCALE(2);
		brect.left = x-NIBLSCALE(3);
		brect.right = brect.left+tsize.cx+NIBLSCALE(3);
		brect.top = y-NIBLSCALE(3);
		brect.bottom = brect.top+tsize.cy+NIBLSCALE(3);
		#if TOPOFASTLABEL
		if (!MapWindow::checkLabelBlock(&brect)) return false;
		#else
		if (!MapWindow::checkLabelBlock(brect)) return false;
		#endif
	}
	SetTextColor(hDC, RGB_PETROL);
    
	ExtTextOut(hDC, x, y, 0, NULL, Temp, size, NULL);
	return true; // 101016
  }
  return false; // 101016
}


void XShapeLabel::setlabel(const char* src) {
  if (src[0] && 												//Null label condition fixed
      (strcmp(src,"UNK") != 0) &&
      (strcmp(src,"RAILWAY STATION") != 0) &&
      (strcmp(src,"RAILROAD STATION") != 0)
      ) {
    if (label) free(label);
    label = (char*)malloc(strlen(src)+1);
    if (label) {
      strcpy(label,src);
    }
    hide=false;
  } else {
    if (label) {
      free(label);
      label= NULL;
    }
    hide=true;
  }
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


TopologyWriter::~TopologyWriter() {
  if (shapefileopen) {
    Close();
    DeleteFiles();
  }
}


TopologyWriter::TopologyWriter(const char* shpname, COLORREF thecolor):
  Topology(shpname, thecolor, true) {

  Reset();
}


void TopologyWriter::DeleteFiles(void) {
  // Delete all files, since zziplib interface doesn't handle file modes
  // properly
  if (strlen(filename)>0) {
    TCHAR fname[MAX_PATH];
    ascii2unicode(filename, fname);
    _tcscat(fname, TEXT(".shp"));
	#if ALPHADEBUG
	StartupStore(_T(". TopologyWriter: deleting <%s>%s"),fname,NEWLINE);
	#endif
    DeleteFile(fname);
    ascii2unicode(filename, fname);
    _tcscat(fname, TEXT(".shx"));
	#if ALPHADEBUG
	StartupStore(_T(". TopologyWriter: deleting <%s>%s"),fname,NEWLINE);
	#endif
    DeleteFile(fname);
    ascii2unicode(filename, fname);
    _tcscat(fname, TEXT(".dbf"));
	#if ALPHADEBUG
	StartupStore(_T(". TopologyWriter: deleting <%s>%s"),fname,NEWLINE);
	#endif
    DeleteFile(fname);
  }
}


void TopologyWriter::CreateFiles(void) {
  // by default, now, this overwrites previous contents

  TCHAR ufname[MAX_PATH]; // 091105
  if (msSHPCreateFile(&shpfile, filename, SHP_POINT) == -1) {
  } else {
    char dbfname[100];
    strcpy(dbfname, filename );
    strcat(dbfname, ".dbf"); 
    ascii2unicode(dbfname,ufname); // 091105
    StartupStore(_T(". TopologyWriter: creating <%s>%s"),ufname,NEWLINE);
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


#if LKCLIP
extern int TwoAxisPolyClip( POINT *theSourcePolygon, int nVerts, RECT *clipWindow, POINT *clip_Pass1, POINT *clippedPolygon);


static POINT clip_ptin[MAXCLIPPOLYGON];
static POINT clip_ptout1[MAXCLIPPOLYGON];
static POINT clip_ptout[MAXCLIPPOLYGON];

void ClipPolygon(HDC hdc, POINT *m_ptin, unsigned int inLength, RECT rc, bool fill) {
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
	if ((m_ptin[inLength-1].x != m_ptin[0].x) && (m_ptin[inLength-1].y != m_ptin[0].y)) {
		clip_ptin[inLength] = clip_ptin[0];
		inLength++;
	}
  }

  rc.top--;
  rc.bottom++;
  rc.left--;
  rc.right++;

  outLength = TwoAxisPolyClip (clip_ptin, inLength, &rc, clip_ptout1, clip_ptout);


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


#else
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
#endif

#if LKCLIP

#define vertex POINT
#define theClipWindow RECT

int TwoAxisPolyClip(vertex *theSourcePolygon, int nVerts, theClipWindow *clipWindow, vertex *clip_Pass1, vertex *clippedPolygon)
{

/* A general two-pass, two-axis polygon clipping routine.
   By Jose Miguel Commins Gonzalez         Email: axora@myrealbox.com or axora@axora.net

   Revised C version, 9 January 2003, with a new brutally efficient condition pipeline.

   This source code and any derivatives is/are entirely in the Public Domain.



								  Vertices are arranged as 'struct vertex' (see above).
   				int nVerts			: Number of vertices in the source polygon.
				theClipWindow *clipWindow	: The address pointing to the clip window structure.
								  The clip window is arranged as 'struct theClipWindow' (see above).
				vertex *clip_Pass1		: The address pointing to the first pass output polygon.
								  Vertices are arranged as 'struct vertex' (see above).
				vertex *clippedPolygon		: The address pointing to the second (final) pass output polygon.
								  Vertices are arranged as 'struct vertex' (see above).
	RETURNS:
				INT				: Number of vertices in output polygon.
								  If this value is < 3, then polygon is not visible.

*/



int	i, j, pIndex;
long int	ox1, oy1, clip_left, clip_right, clip_bottom, clip_top;
long int	rx1, ry1, rx2, ry2;

// Initialise a few vars - this is mostly done for register optimisations.
clip_left = clipWindow->left;
clip_right = clipWindow->right;
clip_bottom = clipWindow->bottom;
clip_top = clipWindow->top;

// Initialise the destination output polygon index to zero - at the end of the pass this holds how many vertices the pass has produced.
pIndex = 0;
// Initialise the variables used to walk along the polygon and join the lines together.
rx1 = theSourcePolygon[0].x;
ry1 = theSourcePolygon[0].y;

j = 1;

// Now clip the polygon to the X axis.

for (i = 0; i < nVerts; i++) {
	rx2 = theSourcePolygon[j].x;
	ry2 = theSourcePolygon[j++].y;
	ox1 = rx2;
	oy1 = ry2;

	// Check for line orientation and deal with the lines accordingly.
	if (rx1 > rx2) {

		// We are in right-to-left orientation - perform a quick bounds check for lines completely outside the clip boundary.
		if (rx1 < clip_left || rx2 > clip_right)
			goto lr_boundOut;

		// Store the line's rightmost coordinate - this is overwritten if a clip occurs; if not, it forms part of the polygon edge.
		clip_Pass1[pIndex].x = rx1;
		clip_Pass1[pIndex++].y = ry1;

		// Test if the line's rightmost coordinate crosses the right-side clip boundary; if so, clip and overwrite the polygon edge previously stored.
		if (rx1 > clip_right) {
			ry1 = ry1 + ((ry2 - ry1) * (clip_right - rx1)) / (rx2 - rx1);
			rx1 = clip_right;
			clip_Pass1[pIndex - 1].x = rx1;
			clip_Pass1[pIndex - 1].y = ry1;
		}

		// Test if the line's leftmost coordinate crosses the left-side clip boundary; if so, clip and add the new polygon edge.
		if (rx2 < clip_left) {
			ry2 = ry2 + ((ry1 - ry2) * (clip_left - rx2)) / (rx1 - rx2);
			rx2 = clip_left;
			clip_Pass1[pIndex].x = rx2;
			clip_Pass1[pIndex++].y = ry2;
		}

	} else {

		// Same as above, but now handling left-to-right orientation.
		if (rx2 < clip_left || rx1 > clip_right)
			goto lr_boundOut;

		clip_Pass1[pIndex].x = rx1;
		clip_Pass1[pIndex++].y = ry1;

		if (rx1 < clip_left) {
			ry1 = ry1 + ((ry2 - ry1) * (clip_left - rx1)) / (rx2 - rx1);
			rx1 = clip_left;
			clip_Pass1[pIndex - 1].x = rx1;
			clip_Pass1[pIndex - 1].y = ry1;
		}

		if (rx2 > clip_right) {
			ry2 = ry2 + ((ry1 - ry2) * (clip_right - rx2)) / (rx1 - rx2);
			rx2 = clip_right;
			clip_Pass1[pIndex].x = rx2;
			clip_Pass1[pIndex++].y = ry2;
		}

	}


lr_boundOut:
	rx1 = ox1;
	ry1 = oy1;

// If it is the last vertex, the line to be clipped is the polygon's last vertex -> first vertex.
	if(j == nVerts) j = 0;

	}


// Now clip to another axis - same as above, here using the Y axis to clip to.

// Quick check if the left-right clip chopped out the polygon entirely; if so, don't bother with this clip as we have no polygon to clip with.
if(pIndex < 3)
	return(pIndex);

nVerts = pIndex;
pIndex = 0;

rx1 = clip_Pass1[0].x;
ry1 = clip_Pass1[0].y;

j = 1;

for (i = 0; i < nVerts; i++) {
	rx2 = clip_Pass1[j].x;
	ry2 = clip_Pass1[j++].y;
	ox1 = rx2;
	oy1 = ry2;

	if (ry1 > ry2) {

		if (ry1 < clip_top || ry2 > clip_bottom)
			goto tb_boundOut;

		clippedPolygon[pIndex].x = rx1;
		clippedPolygon[pIndex++].y = ry1;

		if (ry1 > clip_bottom) {
			rx1 = rx1 + ((rx2 - rx1) * (clip_bottom - ry1)) / (ry2 - ry1);
			ry1 = clip_bottom;
			clippedPolygon[pIndex - 1].x = rx1;
			clippedPolygon[pIndex - 1].y = ry1;
		}

		if (ry2 < clip_top) {
			rx2 = rx2 + ((rx1 - rx2) * (clip_top - ry2)) / (ry1 - ry2);
			ry2 = clip_top;
			clippedPolygon[pIndex].x = rx2;
			clippedPolygon[pIndex++].y = ry2;
		}

	} else {

		if (ry2 < clip_top || ry1 > clip_bottom)
			goto tb_boundOut;

		clippedPolygon[pIndex].x = rx1;
		clippedPolygon[pIndex++].y = ry1;

		if (ry1 < clip_top) {
			rx1 = rx1 + ((rx2 - rx1) * (clip_top - ry1)) / (ry2 - ry1);
			ry1 = clip_top;
			clippedPolygon[pIndex - 1].x = rx1;
			clippedPolygon[pIndex - 1].y = ry1;
		}

		if (ry2 > clip_bottom) {
			rx2 = rx2 + ((rx1 - rx2) * (clip_bottom - ry2)) / (ry1 - ry2);
			ry2 = clip_bottom;
			clippedPolygon[pIndex].x = rx2;
			clippedPolygon[pIndex++].y = ry2;
		}

	}


tb_boundOut:
	rx1 = ox1;
	ry1 = oy1;

	if(j == nVerts) j = 0;

	}


		return(pIndex);
}

#endif
