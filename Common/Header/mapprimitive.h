#ifndef MAPPRIMITIVE_H
#define MAPPRIMITIVE_H

#define MS_VERSION "4.0.1"

#define MS_FALSE 0
#define MS_TRUE 1

#define MS_MIN(a,b)     (((a)<(b))?(a):(b))
#define MS_MAX(a,b)	(((a)>(b))?(a):(b))
#define MS_ABS(a)	(((a)<0) ? -(a) : (a))
#define MS_SGN(a)	(((a)<0) ? -1 : 1)
#define MS_NINT(x)      ((x) >= 0.0 ? ((long) ((x)+.5)) : ((long) ((x)-.5)))

enum MS_RETURN_VALUE {MS_SUCCESS, MS_FAILURE, MS_DONE};
enum MS_SHAPE_TYPE {MS_SHAPE_POINT, MS_SHAPE_LINE, MS_SHAPE_POLYGON, MS_SHAPE_NULL};


#define MS_CELLSIZE(min,max,d)    ((max - min)/d)
#define MS_MAP2IMAGE_X(x,minx,cx) (MS_NINT((x - minx)/cx))
#define MS_MAP2IMAGE_Y(y,maxy,cy) (MS_NINT((maxy - y)/cy))
#define MS_IMAGE2MAP_X(x,minx,cx) (minx + cx*x)
#define MS_IMAGE2MAP_Y(y,maxy,cy) (maxy - cy*y)

#define MS_DEG_TO_RAD	.0174532925199432958
#define MS_RAD_TO_DEG   57.29577951


//???????????
#define MS_INDEX_EXTENSION ".qix"
#define MS_QUERY_EXTENSION ".qy"



/* feature primitives */
typedef struct {
  double minx, miny, maxx, maxy;
} rectObj;

typedef struct {
  double x;
  double y;
} vectorObj;

typedef struct {
  double x;
  double y;
  double m;  
} pointObj;

typedef struct {
  int numpoints;
  pointObj *point;
} lineObj;

typedef struct {
  int numlines;
  lineObj *line;
  rectObj bounds;

  int type; // MS_SHAPE_TYPE

  long index;
  int tileindex;

  int classindex;

  char *text;

  char **values;
  int numvalues;

} shapeObj;

typedef lineObj multipointObj;

/* attribute primatives */
typedef struct {
  char *name;
  long type;
  int index;
  int size;
  short numdecimals;
} itemObj;

#ifdef __cplusplus
extern "C" {
#endif

void msFreeShape(shapeObj *shape); // in mapprimative.c
void msInitShape(shapeObj *shape);
int msCopyShape(shapeObj *from, shapeObj *to);
void msComputeBounds(shapeObj *shape);
void msRectToPolygon(rectObj rect, shapeObj *poly);
void msClipPolylineRect(shapeObj *shape, rectObj rect);
void msClipPolygonRect(shapeObj *shape, rectObj rect);
//void msTransformShape(shapeObj *shape, rectObj extent, double cellsize, imageObj *image);
void msTransformShapeToPixel(shapeObj *shape, rectObj extent, double cellsize);
void msTransformPixelToShape(shapeObj *shape, rectObj extent, double cellsize);
//void msImageCartographicPolyline(gdImagePtr im, shapeObj *p, styleObj *style, symbolObj *symbol, int c, double size, double scalefactor);
int msPolylineLabelPoint(shapeObj *p, pointObj *lp, int min_length, double *angle, double *length);
int msPolygonLabelPoint(shapeObj *p, pointObj *lp, int min_dimension);
int msAddLine(shapeObj *p, lineObj *new_line);

int msPointInRect(pointObj *p, rectObj *rect); // in mapsearch.c
int msRectOverlap(const rectObj *a, const rectObj *b);		//const arguments fixed
int msRectContained(const rectObj *a, const rectObj *b);	//const arguments fixed
void msMergeRect(rectObj *a, rectObj *b);
double msDistancePointToPoint(pointObj *a, pointObj *b);
double msDistancePointToSegment(pointObj *p, pointObj *a, pointObj *b);
double msDistancePointToShape(pointObj *p, shapeObj *shape);
double msDistanceSegmentToSegment(pointObj *pa, pointObj *pb, pointObj *pc, pointObj *pd);
double msDistanceShapeToShape(shapeObj *shape1, shapeObj *shape2);
int msIntersectSegments(pointObj *a, pointObj *b, pointObj *c, pointObj *d);
int msPointInPolygon(pointObj *p, lineObj *c);
int msIntersectMultipointPolygon(multipointObj *points, shapeObj *polygon);
int msIntersectPointPolygon(pointObj *p, shapeObj *polygon);
int msIntersectPolylinePolygon(shapeObj *line, shapeObj *poly);
int msIntersectPolygons(shapeObj *p1, shapeObj *p2);
int msIntersectPolylines(shapeObj *line1, shapeObj *line2);

size_t msGetBitArraySize(int numbits); // in mapbits.c
char *msAllocBitArray(int numbits);
int msGetBit(char *array, int index);
void msSetBit(char *array, int index, int value);
void msFlipBit(char *array, int index);

void msFree(void *p);
void msFreeCharArray(char **array, int num_items);

#include <zzip/lib.h>

ZZIP_FILE *ppc_fopen(const TCHAR *filename, const char *mode);
#ifdef __cplusplus
}
#endif
#endif /* MAPPRIMITIVE_H */
