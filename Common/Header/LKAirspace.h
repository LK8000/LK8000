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
#include <algorithm>
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
//Airspace warning events
typedef enum { aweNone, 
				//for FLY zones
				aweMovingInsideFly,
				awePredictedLeavingFly,
				aweLeavingFly,
				awePredictedEnteringFly,
				aweEnteringFly,
				aweMovingOutsideFly,
				
				//for NON-FLY zones
				aweMovingOutsideNonfly,
				awePredictedEnteringNonfly,
				aweEnteringNonfly,
				aweMovingInsideNonfly,
				aweLeavingNonFly
} AirspaceWarningEvent;
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
			_userwarningstate(awNone),
			_userwarningstateold(awNone),
			_userwarnackstate(awNone),
			_lastknownpos(),
			_lastknownalt(0),
			_lastknownagl(0),
			_pos_inside_last(false),
			_pos_inside_now(false),
			_warnevent(aweNone),
			_warneventold(aweNone),
			_warnacktimeout(0),
			_now(0)
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

  
  //Warning system
  // Calculate warning level based on last/next/predicted position
  void CalculateWarning(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
						bool *pos_in_flyzone, bool *pred_in_flyzone, 
						bool *pos_in_acked_nonfly_zone, bool *pred_in_acked_nonfly_zone);
  // Second pass warning level calculation
  bool FinishWarning(int now, 
					 bool pos_inside_fly_zone, bool pred_inside_fly_zone, 
					 bool pos_in_acked_nonfly_zone, bool pred_in_acked_nonfly_zone);
  // Calculate airspace distance from last known position (required by messages and dialog boxes)
  bool CalculateDistance(int *hDistance, int *Bearing, int *vDistance);
  
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
  
  //int WarningRepeatTimer() const { return _warn_repeat_time; }
  //void WarningRepeatTimer(int warnreptimer) { _warn_repeat_time = warnreptimer; }

  AirspaceWarningEvent WarningEvent() const { return _warnevent; }
  void SetAckTimeout();					// Set ack validity timeout

protected:
  TCHAR _name[NAME_SIZE + 1];
  int _type;
  AIRSPACE_ALT _base;
  AIRSPACE_ALT _top;
  rectObj _bounds;
  bool _flyzone;					// true if leaving generates warning
  AirspaceDrawStyle_t _drawstyle;
  // Warnings
  int _warn_repeat_time;  			// tick when repeat warning message if not acked
  AirspaceWarningState_t _userwarningstate;
  AirspaceWarningState_t _userwarningstateold;
  AirspaceWarningState_t _userwarnackstate;
  CGeoPoint _lastknownpos;			// last known position saved for calculations
  int _lastknownalt;
  int _lastknownagl;
  bool _pos_inside_last;
  bool _pos_inside_now;
  AirspaceWarningEvent _warnevent;
  AirspaceWarningEvent _warneventold;
  int _warnacktimeout;
  int _now;
  
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
typedef struct _AirspaceWarningMessage
{
  CAirspace *originator;
  AirspaceWarningEvent event;
  AirspaceWarningState_t warnstate;
} AirspaceWarningMessage;
typedef std::deque<AirspaceWarningMessage> AirspaceWarningMessageList;


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
  bool ClearAirspaceWarnings(const bool acknowledge, const bool ack_all_day = false);

  void AirspaceSetAckState(CAirspace &airspace, AirspaceWarningState_t ackstate);
  void AirspaceWarnListAckWarn(CAirspace &airspace);
  void AirspaceWarnListAckSpace(CAirspace &airspace);
  void AirspaceWarnListDailyAck(CAirspace &airspace);
  void AirspaceWarnListDailyAckCancel(CAirspace &airspace);
  
  //CAirspace* PopWarningMessagedAirspace();
  bool PopWarningMessage(AirspaceWarningMessage *msg);
  
  //Get airspace details (dlgAirspaceDetails)
  CAirspaceList GetVisibleAirspacesAtPoint(const double &lon, const double &lat) const;
  CAirspaceList GetAllAirspaces() const;
  CAirspaceList GetAirspacesInWarning() const;
  CAirspace GetAirspaceCopy(CAirspace* airspace) const;
  bool AirspaceCalculateDistance(CAirspace *airspace, int *hDistance, int *Bearing, int *vDistance);
  
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
  
  // Airspaces data
  mutable CCriticalSection _csairspaces;
  CAirspaceList _airspaces;			//ALL
  CAirspaceList _airspaces_near;	//Near
  bool _GlobalClearAirspaceWarnings;
  
  // Warning system data
  // User warning message queue
  AirspaceWarningMessageList _user_warning_queue;				// warnings to show

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

void ShowAirspaceWarningsToUser();

void ScreenClosestPoint(const POINT &p1, const POINT &p2, 
			const POINT &p3, POINT *p4, int offset);


#endif /* LKAIRSPACE */
#endif
