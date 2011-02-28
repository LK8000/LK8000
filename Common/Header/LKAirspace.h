#if !defined(AFX_LKAIRSPACE_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_LKAIRSPACE_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StdAfx.h"
#include "Sizes.h"
#include "mapshape.h"

#ifdef LKAIRSPACE
#include "criticalsection.h"
#include "Calculations.h"
#include <tchar.h>

#include <vector>
#include <deque>
#include <list>
using namespace std;

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))

#define AIRSPACE_SCANSIZE_X 16
#define AIRSPACE_SCANSIZE_H 16

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

//Airspace warning and ack states
typedef enum {awNone=0, awPredicted, awWarning, awDailyAck} AirspaceWarningState_t;
//Airspace warning system internal states
typedef enum {awsNone, awsNew, awsCheckWarning, awsWarning } AirspaceWarningStateInternal_t;
//Airspace drawstyles
typedef enum {adsHidden, adsOutline, adsFilled } AirspaceDrawStyle_t;

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
			_flyzone(false),
			_drawstyle(adsHidden),
			_warnstate(awsNone),
			_userwarningstate(awNone),
			_userwarningstateold(awNone),
			_userwarnackstate(awNone),
			_warnid(0),
			_warnvdistance(0.0),
			_warnhdistance(0.0)
			{}
  virtual ~CAirspace() {}

  
  virtual bool Inside(const double &longitude, const double &latitude) const { return false; }
  virtual void SetPoints(CGeoPointList &area_points) {}
  virtual void Dump() const;
  virtual void CalculateScreenPosition(const rectObj &screenbounds_latlon, const int iAirspaceMode[], const int iAirspaceBrush[], const double &ResMapScaleOverDistanceModify) {}
  virtual void Draw(HDC hDCTemp, const RECT &rc, bool param1) const {}
  virtual double Range(const double &longitude, const double &latitude, double &bearing) const { return 0.0; }
  void QnhChangeNotify();
  bool GetFarVisible(const rectObj &bounds_active) const;
  
  // Attributes interface
  void Init(const TCHAR *name, const int type, const AIRSPACE_ALT &base, const AIRSPACE_ALT &top, bool flyzone) 
	  { _tcsncpy(_name, name, NAME_SIZE); _type = type; memcpy(&_base, &base, sizeof(_base)); memcpy(&_top, &top, sizeof(_top)); _flyzone = flyzone;}

  const TCHAR* Name() const { return _name; }
  const AIRSPACE_ALT* Top() const { return &_top; }
  const AIRSPACE_ALT* Base() const { return &_base; }
  const rectObj& Bounds() const { return _bounds; }
  const bool Flyzone() const { return _flyzone; }
  
  AirspaceDrawStyle_t DrawStyle() const { return _drawstyle; }
  void DrawStyle(AirspaceDrawStyle_t drawstyle) { _drawstyle = drawstyle; } 

  int Type() const { return _type; }
  void Type(int type) { _type = type; } 
  
  AirspaceWarningState_t UserWarningState() const { return _userwarningstate; }
  void UserWarningState(AirspaceWarningState_t userwarningstate) { _userwarningstate = userwarningstate; }
  
  AirspaceWarningState_t UserWarningStateOld() const { return _userwarningstateold; }
  void UserWarningStateOld(AirspaceWarningState_t userwarningstateold) { _userwarningstateold = userwarningstateold; }
  
  AirspaceWarningState_t UserWarnAckState() const { return _userwarnackstate; }
  void UserWarnAckState(AirspaceWarningState_t userwarnackstate) { _userwarnackstate = userwarnackstate; }
  
  AirspaceWarningStateInternal_t WarningState() const { return _warnstate; }
  void WarningState(AirspaceWarningStateInternal_t warnstate) { _warnstate = warnstate; }
  
  int WarningRepeatTimer() const { return _warn_repeat_time; }
  void WarningRepeatTimer(int warnreptimer) { _warn_repeat_time = warnreptimer; }

  int WarningID() const { return _warnid; }
  void WarningID(int warnid) { _warnid = warnid; }

  double WarnvDistance() const { return _warnvdistance; }
  void WarnvDistance(double warnvdistance) { _warnvdistance = warnvdistance; }

  double WarnhDistance() const { return _warnhdistance; }
  void WarnhDistance(double warnhdistance) { _warnhdistance = warnhdistance; }

protected:
  TCHAR _name[NAME_SIZE + 1];
  int _type;
  AIRSPACE_ALT _base;
  AIRSPACE_ALT _top;
  rectObj _bounds;
  bool _flyzone;					// true if leaving generates warning
  AirspaceDrawStyle_t _drawstyle;
  // Warnings
  AirspaceWarningStateInternal_t _warnstate;  
  int _warn_repeat_time;  			// tick when repeat warning message if not acked
  AirspaceWarningState_t _userwarningstate;
  AirspaceWarningState_t _userwarningstateold;
  AirspaceWarningState_t _userwarnackstate;
  int _warnid;
  double _warnvdistance;
  double _warnhdistance;
  
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
  void ScreenClosestPoint(const POINT &p1, const POINT &p2, 
			const POINT &p3, POINT *p4, int offset) const;
  double ScreenCrossTrackError(double lon1, double lat1,
			double lon2, double lat2,
			double lon3, double lat3,
			double *lon4, double *lat4) const;
			

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

// 
// AIRSPACE MANAGER CLASS
//
typedef std::deque<CAirspace*> CAirspaceList;

//Warning system 
typedef enum {asaNull, asaItemAdded, asaItemChanged, asaClearAll, asaItemRemoved, asaWarnLevelIncreased, asaProcessEnd, asaProcessBegin} AirspaceWarningNotifyAction_t;

class CAirspaceManager
{
public:
  static CAirspaceManager& Instance() { return _instance; }

  //HELPER FUNCTIONS
  bool CheckAirspaceAltitude(const double &Base, const double &Top) const;

  // Upper level interfaces
  void ReadAirspaces();
  void CloseAirspaces();
  void QnhChangeNotify(const double &newQNH);
  void ScanAirspaceLine(double lats[], double lons[], double heights[], 
		      int airspacetype[AIRSPACE_SCANSIZE_H][AIRSPACE_SCANSIZE_X]) const;
  const CAirspace* FindNearestAirspace(const double &longitude, const double &latitude,
			 double *nearestdistance, double *nearestbearing, double *height = NULL) const;
  void SortAirspaces(void);
  bool ValidAirspaces(void) const;

  //Warning system
  void AirspaceWarning (NMEA_INFO *Basic, DERIVED_INFO *Calculated);
  void AirspaceWarnListProcess(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
  bool ClearAirspaceWarnings(const bool acknowledge, const bool ack_all_day = false);
  bool AirspaceWarnListCalcDistance(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const CAirspace *airspace, int *hDistance, int *Bearing, int *vDistance);
  //dlgAirspaceWarning
  int AirspaceWarnGetItemCount();
  bool AirspaceWarnGetItem(unsigned int Index, CAirspace **Item);
  int AirspaceWarnFindIndexByID(int ID);

  void AirspaceWarnListAckWarn(CAirspace &airspace);
  void AirspaceWarnListAckSpace(CAirspace &airspace);
  void AirspaceWarnListDailyAck(CAirspace &airspace);
  void AirspaceWarnListDailyAckCancel(CAirspace &airspace);
  
  
  //Get airspace details (dlgAirspaceDetails)
  CAirspaceList GetVisibleAirspacesAtPoint(const double &lon, const double &lat) const;
  CAirspaceList GetAllAirspaces() const;

  //Mapwindow drawing
  void SetFarVisible(const rectObj &bounds_active);
  void CalculateScreenPositionsAirspace(const rectObj &screenbounds_latlon, const int iAirspaceMode[], const int iAirspaceBrush[], const double &ResMapScaleOverDistanceModify);
  const CAirspaceList& GetNearAirspacesRef() const;
  
  //Attributes
  unsigned int NumberofAirspaces() { CCriticalSection::CGuard guard(_csairspaces); return _airspaces.size(); }
  bool GlobalClearAirspaceWarnings() const { return _GlobalClearAirspaceWarnings; }


private:
  static CAirspaceManager _instance;
  CAirspaceManager(const CAirspaceManager&) : _GlobalClearAirspaceWarnings(false) {}
  CAirspaceManager& operator=(const CAirspaceManager&);
  ~CAirspaceManager() { CloseAirspaces(); }
  void DoFlashWarning(CAirspace &airspace);			// Prints flash warning messages to user
  
  // Airspaces data
  mutable CCriticalSection _csairspaces;
  CAirspaceList _airspaces;			//ALL
  CAirspaceList _airspaces_near;	//Near
  bool _GlobalClearAirspaceWarnings;
  

  // Warning system data
  mutable CCriticalSection _cswarnlist;
  CAirspaceList _airspaces_warning;		//Airspaces in warning state>0
  int _static_unique;														//TODO remove this hack
  void AirspaceWarnListSort();
  void AirspaceWarnListClear();

  //Openair parsing functions, internal use
  void FillAirspacesFromOpenAir(ZZIP_FILE *fp);
  bool StartsWith(const TCHAR *Text, const TCHAR *LookFor) const;
  void ReadAltitude(const TCHAR *Text, AIRSPACE_ALT *Alt) const;
  bool ReadCoords(TCHAR *Text, double *X, double *Y) const;
  bool CalculateArc(TCHAR *Text, CGeoPointList *_geopoints, double &CenterX, const double &CenterY, const int &Rotation) const;
  bool CalculateSector(TCHAR *Text, CGeoPointList *_geopoints, double &CenterX, const double &CenterY, const int &Rotation) const;
  

};


//dlgAirspaceWarning
int dlgAirspaceWarningInit(void);
int dlgAirspaceWarningDeInit(void);
void dlgAirspaceWarningNotify(AirspaceWarningNotifyAction_t Action, CAirspace *Airspace);


void ScreenClosestPoint(const POINT &p1, const POINT &p2, 
			const POINT &p3, POINT *p4, int offset);


#endif /* LKAIRSPACE */
#endif
