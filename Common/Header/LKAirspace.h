#if !defined(AFX_LKAIRSPACE_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_LKAIRSPACE_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StdAfx.h"
#include "Sizes.h"
#include "mapshape.h"

#ifdef LKAIRSPACE
#include "CriticalSection.h"
#include "Calculations.h"
#include "Point2D.h"

#include <tchar.h>
#include <vector>
#include <deque>
#include <list>
#include <algorithm>

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

typedef std::vector<POINT> POINTList;

//Airspace warning and ack levels
typedef enum {awNone=0, awYellow, awRed, awDailyAck} AirspaceWarningLevel_t;
//Airspace warning events
typedef enum { aweNone, 
                //for FLY zones
                aweMovingInsideFly,
                awePredictedLeavingFly,
                aweLeavingFly,
                awePredictedEnteringFly,
                aweEnteringFly,
                aweMovingOutsideFly,
                aweNearOutsideFly,

                //for NON-FLY zones
                aweMovingOutsideNonfly,
                awePredictedEnteringNonfly,
                aweEnteringNonfly,
                aweMovingInsideNonfly,
                aweLeavingNonFly,
                aweNearInsideNonfly
} AirspaceWarningEvent;
//Airspace drawstyles
typedef enum {adsHidden, adsOutline, adsFilled } AirspaceDrawStyle_t;
//Airspace warning drawstyles
typedef enum {awsBlack, awsAmber, awsRed } AirspaceWarningDrawStyle_t;

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
            _warninglevel(awNone),
            _warninglevelold(awNone),
            _warningacklevel(awNone),
            _pos_inside_last(false),
            _pos_inside_now(false),
            _warnevent(aweNone),
            _warneventold(aweNone),
            _warnacktimeout(0),
            _distances_ready(false),
            _vdistance(0),
            _hdistance(0),
            _bearing(0),
            _labelpriority(0)
            {}
  virtual ~CAirspace() {}


  // Check if a point horizontally inside in this airspace
  virtual bool IsHorizontalInside(const double &longitude, const double &latitude) const { return false; }
  // Check if an altitude vertically inside in this airspace
          bool IsAltitudeInside(int alt, int agl, int extension=0) const;
  // Set polygon points
  virtual void SetPoints(CPoint2DArray &area_points) {}
  // Dump this airspace to runtime.log
  virtual void Dump() const;
  // Calculate drawing coordinates on screen
  virtual void CalculateScreenPosition(const rectObj &screenbounds_latlon, const int iAirspaceMode[], const int iAirspaceBrush[], const double &ResMapScaleOverDistanceModify) {}
  // Draw airspace on map
  virtual void Draw(HDC hDCTemp, const RECT &rc, bool param1) const {}
  // Calculate nearest horizontal distance and bearing to the airspace from a given point
  virtual double Range(const double &longitude, const double &latitude, double &bearing) const { return 0.0; }
  // QNH change nofitier, called when global qhn changed
  void QnhChangeNotify();

  // Warning system
  // Step1: At the start of warning calculation, set class attributes to init values
  static void StartWarningCalculation(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
  // Step2: Calculate warning level on airspace based on last/next/predicted position
  void CalculateWarning(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
  // Step3: Second pass warning level calculation on airspace
  bool FinishWarning();
  // Calculate airspace distance from last known position (used by warning system and dialog boxes)
  bool CalculateDistance(int *hDistance, int *Bearing, int *vDistance);
  // Set ack validity timeout according to config prameter
  void SetAckTimeout();
  // get nearest distance info to this airspace, returns true if distances calculated by warning system
  bool GetDistanceInfo(bool &inside, int &hDistance, int &Bearing, int &vDistance) const;
  // get nearest vertical distance to this airspace, returns true if distances calculated by warning system
  bool GetVDistanceInfo(int &vDistance, AirspaceWarningDrawStyle_t &drawstyle) const;
  // get warning point coordinates, returns true if airspace has valid distances calculated
  bool GetWarningPoint(double &longitude, double &latitude) const;
  // Reset warnings
  void ResetWarnings();
  
  // Attributes interface
  // Initialize instance attributes
  void Init(const TCHAR *name, const int type, const AIRSPACE_ALT &base, const AIRSPACE_ALT &top, bool flyzone) 
      { _tcsncpy(_name, name, NAME_SIZE); _type = type; memcpy(&_base, &base, sizeof(_base)); memcpy(&_top, &top, sizeof(_top)); _flyzone = flyzone;}

  const TCHAR* Name() const { return _name; }
  const AIRSPACE_ALT* Top() const { return &_top; }
  const AIRSPACE_ALT* Base() const { return &_base; }
  const rectObj& Bounds() const { return _bounds; }
  bool Flyzone() const { return _flyzone; }
  void FlyzoneToggle() { _flyzone = !_flyzone; }
  int LabelPriority() const { return _labelpriority; }
  // Label priority sequencing
  void LabelPriorityInc() { if (_labelpriority<10) ++_labelpriority; }        // Increase priority
  void LabelPriorityZero() { _labelpriority=0; }                            // Zero priority
  
  AirspaceDrawStyle_t DrawStyle() const { return _drawstyle; }
  void DrawStyle(AirspaceDrawStyle_t drawstyle) { _drawstyle = drawstyle; } 

  int Type() const { return _type; }
  void Type(int type) { _type = type; } 
  
  AirspaceWarningLevel_t WarningLevel() const { return _warninglevel; }
  void WarningLevel(AirspaceWarningLevel_t userwarningstate) { _warninglevel = userwarningstate; }
  
  AirspaceWarningLevel_t WarningAckLevel() const { return _warningacklevel; }
  void WarningAckLevel(AirspaceWarningLevel_t userwarnackstate) { _warningacklevel = userwarnackstate; }
  
  AirspaceWarningEvent WarningEvent() const { return _warnevent; }

  // Get class attributes for infobox values
  static TCHAR* GetNearestName() { return _nearestname; }
  static int GetNearestHDistance() { return _nearesthdistance; }
  static int GetNearestVDistance() { return _nearestvdistance; }


protected:
  TCHAR _name[NAME_SIZE + 1];                    // Name
  int _type;                                    // type (class) of airspace
  AIRSPACE_ALT _base;                            // base altitude
  AIRSPACE_ALT _top;                            // top altitude
  rectObj _bounds;                                // airspace bounds
  bool _flyzone;                                // true if this is a normally fly zone (leaving generates warning)
  AirspaceDrawStyle_t _drawstyle;                // draw mode
  
  // Warning system data
  int _warn_repeat_time;                          // time when repeat warning message if not acked
  AirspaceWarningLevel_t _warninglevel;            // actual warning level
  AirspaceWarningLevel_t _warninglevelold;        // warning level in last cycle
  AirspaceWarningLevel_t _warningacklevel;        // actual ack level
  bool _pos_inside_last;                        // last horizontal inside saved for calculations
  bool _pos_inside_now;                            // inside now  saved for calculations
  AirspaceWarningEvent _warnevent;                // calculated warning event
  AirspaceWarningEvent _warneventold;            // last calculated warning event
  int _warnacktimeout;                            // ack expiring time
  // Values used by different dialog boxes, like dlgLKAirspace, dlgAirspace, and warning system also
  bool _distances_ready;        // Distances calculated on this airspace
  int _vdistance;                // vertical distance to actual position
  int _hdistance;                // horizontal distance to actual position
  int _bearing;                    // bearing from actual position
  short int _labelpriority;            // warning label drawing priority to sequence labels on map
  
  // Private functions
  void AirspaceAGLLookup(double av_lat, double av_lon);

  // Class attributes
  static int _nearesthdistance;                // collecting horizontal distance to infobox
  static int _nearestvdistance;                // collecting vertical distance to infobox
  static TCHAR *_nearestname;                // collecting nearest name to infobox
  static bool _pos_in_flyzone;                // for flyzone warning refining
  static bool _pred_in_flyzone;                // for flyzone warning refining
  static bool _pos_in_acked_nonfly_zone;    // for flyzone warning refining
  static bool _pred_in_acked_nonfly_zone;    // for flyzone warning refining
  static int _now;                            // recent time
  static int _hdistancemargin;                // used horizontal distance margin
  static CPoint2D _lastknownpos;            // last known position saved for calculations
  static int _lastknownalt;                    // last known alt saved for calculations
  static int _lastknownagl;                    // last known agl saved for calculations

};

// 
// AIRSPACE AREA CLASS
//
class CAirspace_Area: public CAirspace {
public:
  CAirspace_Area() : CAirspace() {}
  virtual ~CAirspace_Area() {};

  // Check if a point horizontally inside in this airspace
  virtual bool IsHorizontalInside(const double &longitude, const double &latitude) const;
  // Set polygon points
  virtual void SetPoints(CPoint2DArray &area_points);
  // Dump this airspace to runtime.log
  virtual void Dump() const;
  // Calculate drawing coordinates on screen
  virtual void CalculateScreenPosition(const rectObj &screenbounds_latlon, const int iAirspaceMode[], const int iAirspaceBrush[], const double &ResMapScaleOverDistanceModify);
  // Draw airspace on map
  virtual void Draw(HDC hDCTemp, const RECT &rc, bool param1) const;
  // Calculate nearest horizontal distance and bearing to the airspace from a given point
  virtual double Range(const double &longitude, const double &latitude, double &bearing) const;

private:
  CPoint2DArray _geopoints;        // polygon points
  POINTList _screenpoints;        // screen coordinates
  
  // Winding number calculation to check a point is horizontally inside polygon
  int wn_PnPoly( const double &longitude, const double &latitude ) const;
  // Calculate airspace bounds
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

  // Check if a point horizontally inside in this airspace
  virtual bool IsHorizontalInside(const double &longitude, const double &latitude) const;
  // Dump this airspace to runtime.log
  virtual void Dump() const;
  // Calculate drawing coordinates on screen
  virtual void CalculateScreenPosition(const rectObj &screenbounds_latlon, const int iAirspaceMode[], const int iAirspaceBrush[], const double &ResMapScaleOverDistanceModify);
  // Draw airspace on map
  virtual void Draw(HDC hDCTemp, const RECT &rc, bool param1) const;
  // Calculate nearest horizontal distance and bearing to the airspace from a given point
  virtual double Range(const double &longitude, const double &latitude, double &bearing) const;
  
private:
  POINT _screencenter;        // center point in screen coordinates
  int _screenradius;        // radius in screen coordinates
  double _latcenter;        // center point latitude
  double _loncenter;        // center point longitude
  double _radius;            // radius
  
  // Bound calculation helper function
  void ScanCircleBounds(double bearing);
  // Calculate airspace bounds
  void CalcBounds();
};

// 
// AIRSPACE MANAGER HELPERS
//
typedef std::deque<CAirspace*> CAirspaceList;

//Warning system generated message
typedef struct _AirspaceWarningMessage
{
  CAirspace *originator;                // airspace instance
  AirspaceWarningEvent event;            // message cause
  AirspaceWarningLevel_t warnlevel;        // warning level
} AirspaceWarningMessage;
// Warning message queue
typedef std::deque<AirspaceWarningMessage> AirspaceWarningMessageList;

//
// AIRSPACE MANAGER CLASS
//
class CAirspaceManager
{
public:
  static CAirspaceManager& Instance() { return _instance; }

  //HELPER FUNCTIONS
  bool CheckAirspaceAltitude(const double &Base, const double &Top) const;
  TCHAR* GetAirspaceTypeText(int type) const;
  TCHAR* GetAirspaceTypeShortText(int type) const;
  void GetAirspaceAltText(TCHAR *buffer, int bufferlen, const AIRSPACE_ALT *alt) const;

  // Upper level interfaces
  void ReadAirspaces();
  void CloseAirspaces();
  void QnhChangeNotify(const double &newQNH);
  void ScanAirspaceLine(double lats[], double lons[], double heights[], 
              int airspacetype[AIRSPACE_SCANSIZE_H][AIRSPACE_SCANSIZE_X]) const;
  CAirspace* FindNearestAirspace(const double &longitude, const double &latitude,
             double *nearestdistance, double *nearestbearing, double *height = NULL) const;
  void SortAirspaces(void);
  bool ValidAirspaces(void) const;

  //Warning system
  void AirspaceWarning (NMEA_INFO *Basic, DERIVED_INFO *Calculated);
  bool AirspaceWarningIsGoodPosition(float longitude, float latitude, int alt, int agl) const;

  void AirspaceSetAckLevel(CAirspace &airspace, AirspaceWarningLevel_t ackstate);
  void AirspaceAckWarn(CAirspace &airspace);
  void AirspaceAckSpace(CAirspace &airspace);
  void AirspaceAckDaily(CAirspace &airspace);
  void AirspaceAckDailyCancel(CAirspace &airspace);
  void AirspaceFlyzoneToggle(CAirspace &airspace);
  
  bool PopWarningMessage(AirspaceWarningMessage *msg);
  void AirspaceWarningLabelPrinted(CAirspace &airspace, bool success);
  
  //Get airspace details (dlgAirspaceDetails)
  CAirspaceList GetVisibleAirspacesAtPoint(const double &lon, const double &lat) const;
  const CAirspaceList GetAllAirspaces() const;
  const CAirspaceList GetAirspacesForWarningLabels();
  CAirspaceList GetAirspacesInWarning() const;
  CAirspace GetAirspaceCopy(const CAirspace* airspace) const;
  bool AirspaceCalculateDistance(CAirspace *airspace, int *hDistance, int *Bearing, int *vDistance);
  
  //Mapwindow drawing
  void SetFarVisible(const rectObj &bounds_active);
  void CalculateScreenPositionsAirspace(const rectObj &screenbounds_latlon, const int iAirspaceMode[], const int iAirspaceBrush[], const double &ResMapScaleOverDistanceModify);
  const CAirspaceList& GetNearAirspacesRef() const;
  
  //Attributes
  unsigned int NumberofAirspaces() { CCriticalSection::CGuard guard(_csairspaces); return _airspaces.size(); }

  //Locking
  CCriticalSection& MutexRef() const { return _csairspaces; }

private:
  static CAirspaceManager _instance;
  CAirspaceManager(const CAirspaceManager&) {}
  CAirspaceManager& operator=(const CAirspaceManager&);
  ~CAirspaceManager() { CloseAirspaces(); }
  
  // Airspaces data
  mutable CCriticalSection _csairspaces;
  CAirspaceList _airspaces;            //ALL
  CAirspaceList _airspaces_near;    //Near
  
  // Warning system data
  // User warning message queue
  AirspaceWarningMessageList _user_warning_queue;                // warnings to show
  CAirspaceList _airspaces_of_interest;
  
  //Openair parsing functions, internal use
  void FillAirspacesFromOpenAir(ZZIP_FILE *fp);
  bool StartsWith(const TCHAR *Text, const TCHAR *LookFor) const;
  void ReadAltitude(const TCHAR *Text, AIRSPACE_ALT *Alt) const;
  bool ReadCoords(TCHAR *Text, double *X, double *Y) const;
  bool CalculateArc(TCHAR *Text, CPoint2DArray *_geopoints, double &CenterX, const double &CenterY, const int &Rotation) const;
  bool CalculateSector(TCHAR *Text, CPoint2DArray *_geopoints, double &CenterX, const double &CenterY, const int &Rotation) const;
  void CorrectGeoPoints(CPoint2DArray &points);
  

};


//dlgAirspaceWarning
int dlgAirspaceWarningInit(void);
int dlgAirspaceWarningDeInit(void);

void ShowAirspaceWarningsToUser();

void ScreenClosestPoint(const POINT &p1, const POINT &p2, 
             const POINT &p3, POINT *p4, int offset);


#endif /* LKAIRSPACE */
#endif
