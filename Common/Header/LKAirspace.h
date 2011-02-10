#if !defined(AFX_LKAIRSPACE_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_LKAIRSPACE_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StdAfx.h"
#include "Sizes.h"
#include "mapshape.h"

#ifdef LKAIRSPACE
#include <tchar.h>

#include <vector>
#include <list>
using namespace std;

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))


typedef enum {abUndef=0, abMSL, abAGL, abFL} AirspaceAltBase_t;

typedef struct _AIRSPACE_ALT
{
  double Altitude;
  double FL;
  double AGL;
  AirspaceAltBase_t Base;  
} AIRSPACE_ALT;

// 
// HELPER CLASSES
//
class CGeoPoint
{
public:
  CGeoPoint() : _latitude(0.0), _longitude(0.0) {}
  CGeoPoint(const double &latitude, const double &longitude) : _latitude(latitude), _longitude(longitude) {}

  // Attributes interface
  double Latitude() const { return _latitude; }
  void Latitude(const double &latitude) { _latitude = latitude; }
  double Longitude() const { return _longitude; }
  void Longitude(const double &longitude) { _longitude = longitude; }

private:
  double _latitude;
  double _longitude;
};

typedef std::list<CGeoPoint> CGeoPointList;
typedef std::vector<POINT> POINTList;

// 
// AIRSPACE BASE CLASS
//
class CAirspace 
{
public:
  CAirspace() :
			_name(),
			_type( 0 ),
			_base(),
			_top(),
			_bounds(),
			_farvisible(false),
			_visible(0),
			_newwarnacknobrush(false)
			{}
  virtual ~CAirspace() {}

  
  virtual bool Inside(const double &longitude, const double &latitude) const { return false; }
  virtual void SetPoints(CGeoPointList &area_points) {}
  virtual void Dump() const;
  virtual void CalculateScreenPosition(const rectObj &screenbounds_latlon, const int iAirspaceMode[], const int iAirspaceBrush[], const double &ResMapScaleOverDistanceModify) {}
  virtual void Draw(HDC hDCTemp, const RECT &rc, bool param1) const {}
  virtual double Range(const double &longitude, const double &latitude, double &bearing) const { return 0.0; }
  void QnhChangeNotify();
  void SetFarVisible(rectObj *bounds_active);
  
  // Attributes interface
  void Init(const TCHAR *name, const int type, const AIRSPACE_ALT &base, const AIRSPACE_ALT &top) { _tcsncpy(_name, name, NAME_SIZE); _type = type; memcpy(&_base, &base, sizeof(_base)); memcpy(&_top, &top, sizeof(_top));}

  const TCHAR* Name() const { return _name; }
  const AIRSPACE_ALT* Top() const { return &_top; }
  const AIRSPACE_ALT* Base() const { return &_base; }
  const rectObj& Bounds() const { return _bounds; }
  
  unsigned char Visible() const { return _visible; }
  void Visible(unsigned char visible) { _visible = visible; } 

  int Type() const { return _type; }
  void Type(int type) { _type = type; } 
  
  bool NewWarnAckNoBrush() const { return _newwarnacknobrush; }
  void NewWarnAckNoBrush(bool newwarnacknobrush) { _newwarnacknobrush = newwarnacknobrush; }


protected:
  TCHAR _name[NAME_SIZE + 1];
  int _type;
  AIRSPACE_ALT _base;
  AIRSPACE_ALT _top;
  rectObj _bounds;
  bool _farvisible;
  //AIRSPACE_ACK _ack;
  unsigned char _visible;
  //unsigned char _warninglevel; // 0= no warning, 1= predicted incursion, 2= entered
  bool _newwarnacknobrush;

  void AirspaceAGLLookup(double av_lat, double av_lon);

};

// 
// AIRSPACE AREA CLASS
//
class CAirspace_Area: public CAirspace {
public:
  CAirspace_Area() : CAirspace() {}
  virtual ~CAirspace_Area() {};
  virtual bool Inside(const double &longitude, const double &latitude) const;
  virtual void SetPoints(CGeoPointList &area_points);
  virtual void Dump() const;
  virtual void CalculateScreenPosition(const rectObj &screenbounds_latlon, const int iAirspaceMode[], const int iAirspaceBrush[], const double &ResMapScaleOverDistanceModify);
  virtual void Draw(HDC hDCTemp, const RECT &rc, bool param1) const;
  virtual double Range(const double &longitude, const double &latitude, double &bearing) const;

private:
  CGeoPointList _geopoints;
  POINTList _screenpoints;
  int wn_PnPoly( const double &longitude, const double &latitude ) const;
  void CalcBounds();
};

// 
// AIRSPACE CIRCLE CLASS
//
class CAirspace_Circle: public CAirspace
{
public:
  CAirspace_Circle(const double &Center_Latitude, const double &Center_Longitude, const double &Airspace_Radius);
  virtual ~CAirspace_Circle() {}
  virtual bool Inside(const double &longitude, const double &latitude) const;
  virtual void Dump() const;
  virtual void CalculateScreenPosition(const rectObj &screenbounds_latlon, const int iAirspaceMode[], const int iAirspaceBrush[], const double &ResMapScaleOverDistanceModify);
  virtual void Draw(HDC hDCTemp, const RECT &rc, bool param1) const;
  virtual double Range(const double &longitude, const double &latitude, double &bearing) const;
  
private:
  POINT _screencenter;
  int _screenradius;
  double _latcenter;
  double _loncenter;
  double _radius;
  void ScanCircleBounds(double bearing);
  void CalcBounds();
};



//Warning system
class AirspaceInfo_c{

public:
  int    TimeOut;             // in systicks
  int    InsideAckTimeOut;    // downgrade auto ACK timer
  int    Sequence;            // Sequence nummer is equal for real and predicted calculation
  int    hDistance;           // horizontal distance in m
  int    vDistance;           // vertical distance in m
  int    Bearing;             // in deg
  DWORD  PredictedEntryTime;  // in ms
  int    Acknowledge;         // 0=not Acked, 1=Acked til closer, 2=Acked til leave, 3= Acked whole day
  bool   Inside;              // true if inside
  bool   Predicted;           // true if predicted inside, menas close and entry expected
  CAirspace *Airspace;
  int    SortKey;             // SortKey
  int    LastListIndex;       // Last index in List, used to sort items with same sort criteria
  int    ID;                  // Unique ID
  int    WarnLevel;           // WarnLevel 0 far away, 1 prdicted entry, 2 predicted entry and close, 3 inside      
};

typedef enum {asaNull, asaItemAdded, asaItemChanged, asaClearAll, asaItemRemoved, asaWarnLevelIncreased, asaProcessEnd, asaProcessBegin} AirspaceWarningNotifyAction_t;
typedef void (*AirspaceWarningNotifier_t)(AirspaceWarningNotifyAction_t Action, AirspaceInfo_c *AirSpace) ;

void AirspaceWarnListAddNotifier(AirspaceWarningNotifier_t Notifier);
void AirspaceWarnListRemoveNotifier(AirspaceWarningNotifier_t Notifier);
bool AirspaceWarnGetItem(unsigned int Index, AirspaceInfo_c &Item);
int AirspaceWarnGetItemCount(void);
int dlgAirspaceWarningInit(void);
int dlgAirspaceWarningDeInit(void);
void AirspaceWarnListClear(void);
void AirspaceWarnDoAck(int ID, int Ack);
int AirspaceWarnFindIndexByID(int ID);
void AirspaceWarnListInit(void);
void AirspaceWarnListDeInit(void);


// MapWindow interface ...
//bool dlgAirspaceWarningShowDlg(bool Force);

// double ProjectedDistance(double lon1, double lat1,
//                          double lon2, double lat2,
//                          double lon3, double lat3);

bool ValidAirspace(void);
void ScreenClosestPoint(const POINT &p1, const POINT &p2, 
			const POINT &p3, POINT *p4, int offset);


//
// Module interface to upper level code
//
typedef std::list<CAirspace*> CAirspaceList;
extern CAirspaceList Airspaces;

void ReadAirspace(ZZIP_FILE *fp);
void CloseAirspace();
void AirspaceQnhChangeNotify(double newQNH);

#define AIRSPACE_SCANSIZE_X 16
#define AIRSPACE_SCANSIZE_H 16
void ScanAirspaceLine(double *lats, double *lons, double *heights, 
		      int airspacetype[AIRSPACE_SCANSIZE_H][AIRSPACE_SCANSIZE_X]);
const CAirspace* FindNearestAirspace(const double &longitude, const double &latitude,
			 double *nearestdistance, double *nearestbearing, double *height=NULL);
void SortAirspace(void);


#endif /* LKAIRSPACE */
#endif
