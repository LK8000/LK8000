#if !defined(AFX_LKAIRSPACE_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_LKAIRSPACE_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_


#include "Thread/Mutex.hpp"
#include "Point2D.h"
#include "Math/Point2D.hpp"

#include <tchar.h>
#include <vector>
#include <queue>
#include <list>
#include <algorithm>
#include <zzip/zzip.h>
#include "Screen/LKSurface.h"
#include "Geographic/GeoPoint.h"
#include "Airspace.h"
#include "vertical_bound.h"
#include "Sizes.h"
#include "../Topology/shapelib/mapprimitive.h"

#include "Renderer/AirspaceRenderer.h"
#include "airspace_mode.h"

class ScreenProjection;
class MD5;
struct NMEA_INFO;
struct DERIVED_INFO;

// changed by AlphaLima since we have a second airspace view to next waypoint,
// the waypoint can be much more far away (e.g.  167km for a 500km FAI triangle)
// the resolution turns to be too inaccurate
// so tha small (<5km airspaced) (e.g. dangerous areas) will not be shown
// with AIRSPACE_SCANSIZE_X 64 I tried to make a compromise between resolution and speed on slow devices
#ifdef WINDOWSPC
#define AIRSPACE_SCANSIZE_X 800
#else
#define AIRSPACE_SCANSIZE_X 160
#endif
#define GC_MAX_POLYGON_PTS (2*AIRSPACE_SCANSIZE_X+4)
#define MAX_NO_SIDE_AS 60

using RasterPointList = std::vector<RasterPoint>;


//Airspace warning and ack levels
enum AirspaceWarningLevel_t {
  awNone=0,
  awYellow,
  awRed
};
//Airspace warning events
enum  AirspaceWarningEvent { 
  aweNone,
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
};
//Airspace drawstyles
enum AirspaceDrawStyle_t {
  adsHidden,
  adsOutline,
  adsFilled,
  adsDisabled
};
//Airspace warning drawstyles
enum AirspaceWarningDrawStyle_t {
  awsHidden,
  awsBlack,
  awsAmber,
  awsRed
};

//
// AIRSPACE BASE CLASS
//
class CAirspaceBase {
 public:
  CAirspaceBase() = default;
  virtual ~CAirspaceBase() {}


  // Check if an altitude vertically inside in this airspace
  bool IsAltitudeInside(int alt, int agl, int extension=0) const;
  // QNH change nofitier, called when global qhn changed
  void QnhChangeNotify();
  // compare airspce name and type for grouping
  bool IsSame( CAirspaceBase &as2 ) const;
  // Warning system
  // Set ack validity timeout according to config prameter
  void SetAckTimeout();
  // check if airspace was already acknolaged
  bool Acknowledged() const;
  // get nearest distance info to this airspace, returns true if distances calculated by warning system
  bool GetDistanceInfo(bool &inside, int &hDistance, int &Bearing, int &vDistance) const;
  // get warning point coordinates, returns true if airspace has valid distances calculated
  bool GetWarningPoint(double &longitude, double &latitude, AirspaceWarningDrawStyle_t &hdrawstyle, int &vDistance, AirspaceWarningDrawStyle_t &vdrawstyle) const;
  // Reset warnings
  void ResetWarnings();

  // Attributes interface
  // Initialize instance attributes
  void Init(const TCHAR *name, int type, vertical_bound &&base, vertical_bound &&top, bool flyzone, const TCHAR *comment = NULL);

  const TCHAR* TypeName() const;
  const TCHAR* TypeNameShort() const;
  const LKColor& TypeColor() const;
  const LKBrush& TypeBrush() const;

  const TCHAR* Name() const { return _name; }
  const TCHAR* Comment() const {
    return _comment.c_str();
  }

  const vertical_bound& Top() const { return _ceiling; }
  const vertical_bound& Base() const { return _floor; }

  const rectObj& Bounds() const { return _bounds; }
  bool Flyzone() const { return _flyzone; }
  void FlyzoneToggle() { _flyzone = !_flyzone; }
  void Flyzone(bool active) { _flyzone = active; }

  int LabelPriority() const { return _labelpriority; }
  // Label priority sequencing
  void LabelPriorityInc() { if (_labelpriority<10) ++_labelpriority; }        // Increase priority
  void LabelPriorityZero() { _labelpriority=0; }                            // Zero priority

  AirspaceDrawStyle_t DrawStyle() const { return _drawstyle; }
  void DrawStyle(AirspaceDrawStyle_t drawstyle) { _drawstyle = drawstyle; }

  int Type() const { return _type; }
  void Type(int type) { _type = type; }

  int Enabled() const { return _enabled; }
  void Enabled(bool enabled) { _enabled = enabled; }

  void ExceptSaturday(bool b) {
    _except_saturday = b;
  }
  
  bool ExceptSaturday() {
    return _except_saturday;
  }
  
  void ExceptSunday(bool b) {
    _except_sunday = b;
  }
  
  bool ExceptSunday() {
    return _except_sunday;
  }

  int Selected() const { return _selected; }
  void Selected(bool selected) { _selected = selected; }

  int LastCalculatedHDistance() const { return _hdistance; }            // Get last calculated horizontal distance (LKCalculations.cpp / DoAirspaces())
  int LastCalculatedVDistance() const { return _vdistance; }            // Get last calculated vertical distance (LKCalculations.cpp / DoAirspaces())
  int LastCalculatedBearing() const { return _bearing; }                // Get last calculated bearing (LKCalculations.cpp / DoAirspaces())
  int LastTrackBearing() const { return _lastknowntrackbearing; }       // Get last track bearing (LKCalculations.cpp / DoAirspaces())

  AirspaceWarningLevel_t WarningLevel() const { return _warninglevel; }
  void WarningLevel(AirspaceWarningLevel_t userwarningstate) { _warninglevel = userwarningstate; }

  AirspaceWarningLevel_t WarningAckLevel() const { return _warningacklevel; }
  void WarningAckLevel(AirspaceWarningLevel_t userwarnackstate) { _warningacklevel = userwarnackstate; }

  AirspaceWarningEvent WarningEvent() const { return _warnevent; }

protected:
  TCHAR _name[NAME_SIZE + 1] = {};                    // Name

  tstring _comment;       // extended airspace informations e.g. for Notams

  int _type = OTHER;                                    // type (class) of airspace

  vertical_bound _ceiling = {};                            // top altitude
  vertical_bound _floor = {};                            // base altitude

  rectObj _bounds = {};                                // airspace bounds
  bool _flyzone = false;                                // true if this is a normally fly zone (leaving generates warning)
  AirspaceDrawStyle_t _drawstyle = adsHidden;                // draw mode

  // Warning system data
  int _warn_repeat_time;                          // time when repeat warning message if not acked
  int _warn_ack_timeout;                          // ack expiring time

  AirspaceWarningLevel_t _warninglevel = awNone;            // actual warning level
  AirspaceWarningLevel_t _warninglevelold = awNone;        // warning level in last cycle
  AirspaceWarningLevel_t _warningacklevel = awNone;        // actual ack level
  bool _pos_inside_last = false;                        // last horizontal inside saved for calculations
  bool _pos_inside_now = false;                            // inside now  saved for calculations
  AirspaceWarningEvent _warnevent = aweNone;                // calculated warning event
  AirspaceWarningEvent _warneventold = aweNone;            // last calculated warning event
  // Values used by different dialog boxes, like dlgLKAirspace, dlgAirspace, and warning system also
  bool _distances_ready = false;        // Distances calculated on this airspace
  int _vdistance = 0;                // vertical distance to actual position
  int _hdistance = 0;                // horizontal distance to actual position
  int _3ddistance = 0;               // 3d distance to actual position
  int _bearing = 0;                    // bearing from actual position
  short int _labelpriority = 0;            // warning label drawing priority to sequence labels on map
  bool _vwarninglabel_hide = false;     // Hide vertical warning label
  bool _hwarninglabel_hide = false;     // Hide horizontal warning label
  bool _enabled = true;                // Airspace enabled for operations
  bool _selected = false;               // Airspace selected (for distance calc infoboxes)

  bool _except_saturday = false; // airspace disabled saturday
  bool _except_sunday = false; // airspace disabled sunday


  // Private functions
  void AGLLookup(const GeoPoint& position, double *basealt_out, double *topalt_out) const;

  // Class attributes
  static bool _pos_in_flyzone;                // for flyzone warning refining
  static bool _pred_in_flyzone;                // for flyzone warning refining
  static bool _pos_in_acked_nonfly_zone;    // for flyzone warning refining
  static bool _pred_in_acked_nonfly_zone;    // for flyzone warning refining
  static int _now;                            // recent time
  static int _hdistancemargin;                // used horizontal distance margin
  static CPoint2D _lastknownpos;            // last known position saved for calculations
  static int _lastknownalt;                    // last known alt saved for calculations
  static int _lastknownagl;                    // last known agl saved for calculations
  static int _lastknownheading;                // last known heading saved for calculations
  static int _lastknowntrackbearing;           // last known track bearing saved for calculations
  static bool _pred_blindtime;                 // disable predicted position based warnings near takeoff
};

class  CAirspace;
using CAirspacePtr = std::shared_ptr<CAirspace>;
using CAirspaceWeakPtr = std::weak_ptr<CAirspace>;

class CAirspace : 
        public CAirspaceBase, 
        public std::enable_shared_from_this<CAirspace>
{
public:

    CAirspace() : CAirspaceBase() { }

    CAirspace(CPoint2DArray&& points) : _geopoints(std::forward<CPoint2DArray>(points)) { }

    virtual ~CAirspace() { }

    // Check if a point horizontally inside in this airspace
    virtual bool IsHorizontalInside(const double &longitude, const double &latitude) const = 0;
    // Calculate drawing coordinates on screen
    virtual void CalculateScreenPosition(const rectObj &screenbounds_latlon, const airspace_mode_array& aAirspaceMode, const int iAirspaceBrush[], const RECT& rcDraw, const ScreenProjection& _Proj);
    // Draw airspace on map
    void DrawOutline(LKSurface& Surface, PenReference pen) const;
    void FillPolygon(LKSurface& Surface, const LKBrush& brush) const;

    // Calculate nearest horizontal distance and bearing to the airspace from a given point
    virtual double Range(const double &longitude, const double &latitude, double &bearing) const  = 0;

    double Range(const GeoPoint& position, double &bearing) const {
      return Range(position.longitude, position.latitude, bearing);
    }

    // update hash with airspace common properties
    void Hash(MD5& md5) const;

    // Calculate unique hash code for this airspace
    virtual std::string Hash() const = 0;

    // Warning system
    // Step1: At the start of warning calculation, set class attributes to init values
    static void StartWarningCalculation(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
    // Step2: Calculate warning level on airspace based on last/next/predicted position
    void CalculateWarning(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
    // Step3: Second pass warning level calculation on airspace
    bool FinishWarning();
    // Calculate airspace distance from last known position (used by warning system and dialog boxes)
    bool CalculateDistance(int *hDistance, int *Bearing, int *vDistance, double Longitude = _lastknownpos.Longitude(), double Latitude  = _lastknownpos.Latitude(), int Altitude = _lastknownalt );

    bool CheckVisible() const;

    static void ResetSideviewNearestInstance() { _sideview_nearest_instance.reset(); }
    static CAirspacePtr GetSideviewNearestInstance() { return _sideview_nearest_instance.lock(); }

protected:
    // polygon points : circular airspace are also stored like polygon because that avoid to calculate geographic coordinate for each drawing
    // previous version draw circular airspace using circle, but it's wrong, circle in geographic coordinate are ellipsoid in screen coordinate.
    CPoint2DArray _geopoints;

    // rendrer is modified by DrawThread, never use it in another thread !!
    std::unique_ptr<AirspaceRenderer> rendrer;

    ////////////////////////////////////////////////////////////////////////////////
    // Draw Picto methods
    //  this methods are NEVER used at same time of airspace loading
    //  therefore it can be considered is thread safe
public:
    virtual void DrawPicto(LKSurface& Surface, const RECT &rc) const;
protected:
    virtual void CalculatePictPosition(const RECT& rcDraw, double zoom, RasterPointList &screenpoints_picto) const;
    ////////////////////////////////////////////////////////////////////////////////

    static CAirspaceWeakPtr _sideview_nearest_instance;         // collect nearest airspace instance for sideview during warning calculations
};

struct AirSpaceSideViewSTRUCT {
  RECT rc;
  POINT apPolygon[GC_MAX_POLYGON_PTS];
  int iNoPolyPts;
  int iIdx;
  int iAreaSize;
  int aiLable;
  int iType;
  PixelScalar iMaxBase;
  PixelScalar iMinTop;
  BOOL bRectAllowed;
  BOOL bEnabled;
  TCHAR szAS_Name[NAME_SIZE + 1];
  CAirspacePtr psAS;
};

#define VERTICAL    false
#define HORIZONZTAL true
struct AirSpaceSonarLevelStruct {
  int iDistantrance;      /* distance to airspace      */
  int iSoundDelay;        /* delaytime  for repetition */
  bool bV_H_switch;       /* vert / horiz switch       */
  TCHAR szSoundFilename[80];
};




//
// AIRSPACE AREA CLASS
//
class CAirspace_Area: public CAirspace {
public:
  explicit CAirspace_Area(CPoint2DArray &&Area_Points);
  ~CAirspace_Area() {};

  // Check if a point horizontally inside in this airspace
  bool IsHorizontalInside(const double &longitude, const double &latitude) const override ;

  // Calculate nearest horizontal distance and bearing to the airspace from a given point
  double Range(const double &longitude, const double &latitude, double &bearing) const override;
  // Calculate unique hash code for this airspace
  std::string Hash() const override;

private:

  // Winding number calculation to check a point is horizontally inside polygon
  int wn_PnPoly( const double &longitude, const double &latitude ) const;
  // Calculate airspace bounds
  void CalcBounds();

////////////////////////////////////////////////////////////////////////////////
// Draw Picto methods
//  this methods are NEVER used at same time of airspace loading
//  therefore we can be considered is thread safe
protected:
  void CalculatePictPosition(const RECT& rcDraw,  double zoom, RasterPointList &screenpoints_picto) const override;
////////////////////////////////////////////////////////////////////////////////
};

//
// AIRSPACE CIRCLE CLASS
//
class CAirspace_Circle: public CAirspace
{
public:
  CAirspace_Circle(const GeoPoint &Center, double Radius);
  ~CAirspace_Circle() {}

  // Check if a point horizontally inside in this airspace
  bool IsHorizontalInside(const double &longitude, const double &latitude) const override;
  // Calculate nearest horizontal distance and bearing to the airspace from a given point
  double Range(const double &longitude, const double &latitude, double &bearing) const override;
  // Calculate unique hash code for this airspace
  std::string Hash() const override;

private:

  GeoPoint _center; // center point latitude longitude
  double _radius;            // radius

////////////////////////////////////////////////////////////////////////////////
// Draw Picto methods
//  this methods are NEVER used at same time of airspace loading
//  therefore we can be considered is thread safe
protected:
  void CalculatePictPosition(const RECT& rcDraw,  double zoom, RasterPointList &screenpoints_picto) const override;
////////////////////////////////////////////////////////////////////////////////
};

//
// AIRSPACE MANAGER HELPERS
//
using CAirspaceList = std::deque<CAirspacePtr>;

//Warning system generated message
struct AirspaceWarningMessage {
  CAirspaceWeakPtr originator;           // airspace instance
  AirspaceWarningEvent event;            // message cause
  AirspaceWarningLevel_t warnlevel;      // warning level

  bool operator<(const AirspaceWarningMessage& other) const {
    return (warnlevel < other.warnlevel);
  }
};
// Warning message queue
using AirspaceWarningMessageList = std::priority_queue<AirspaceWarningMessage>;

namespace rapidxml { 
  // Forward declarations
  template<class Ch> class xml_node;
}

//
// AIRSPACE MANAGER CLASS
//
class CAirspaceManager
{
    
  using xml_node = class rapidxml::xml_node<char>;

public:
  static CAirspaceManager& Instance() {
    static CAirspaceManager _instance;
    return _instance;
  }

  //HELPER FUNCTIONS
  static const TCHAR* GetAirspaceTypeText(int type);
  static const TCHAR* GetAirspaceTypeShortText(int type);


  // Upper level interfaces
  void ReadAirspaces();
  void CloseAirspaces();
  #if ASPWAVEOFF
  void AirspaceDisableWaveSectors();
  #endif
  void QnhChangeNotify();

  int ScanAirspaceLineList(const double (&lats)[AIRSPACE_SCANSIZE_X], const double (&lons)[AIRSPACE_SCANSIZE_X],
                        const double (&terrain_heights)[AIRSPACE_SCANSIZE_X],
                        AirSpaceSideViewSTRUCT (&airspacetype)[MAX_NO_SIDE_AS]) const;

  void SortAirspaces();
  bool ValidAirspaces() const;
  //Warning system
  void AirspaceWarning (NMEA_INFO *Basic, DERIVED_INFO *Calculated);
  bool AirspaceWarningIsGoodPosition(float longitude, float latitude, int alt, int agl) const;

  void AirspaceApply(CAirspace &airspace, std::function<void(CAirspace&)> func);

  void AirspaceSetAckLevel(CAirspace &airspace, AirspaceWarningLevel_t ackstate);
  void AirspaceAckWarn(CAirspace &airspace);
  void AirspaceAckSpace(CAirspace &airspace);
  void AirspaceDisable(CAirspace &airspace);
  void AirspaceEnable(CAirspace &airspace);
  void AirspaceFlyzoneToggle(CAirspace &airspace);

  bool PopWarningMessage(AirspaceWarningMessage *msg);
  void AirspaceWarningLabelPrinted(CAirspace &airspace, bool success);

  CAirspaceList GetVisibleAirspacesAtPoint(const double &lon, const double &lat) const;
  CAirspaceList GetNearAirspacesAtPoint(const double &lon, const double &lat, long range) const;

  const CAirspaceList GetAllAirspaces() const;
  const CAirspaceList& GetAirspacesForWarningLabels();
  CAirspaceList GetAirspacesInWarning() const;
  CAirspaceBase GetAirspaceCopy(const CAirspacePtr& airspace) const;
  bool AirspaceCalculateDistance(const CAirspacePtr& airspace, int *hDistance, int *Bearing, int *vDistance);
  void AirspaceSetSelect(const CAirspacePtr& airspace);

  //Mapwindow drawing
  void SetFarVisible(const rectObj &bounds_active);
  void CalculateScreenPositionsAirspace(const rectObj &screenbounds_latlon, const airspace_mode_array& aAirspaceMode, const int iAirspaceBrush[], const RECT& rcDraw, const ScreenProjection& _Proj);
  const CAirspaceList& GetNearAirspacesRef() const;

  //Nearest page 2.4
  void SelectAirspacesForPage24(const double latitude, const double longitude, const double interest_radius);
  void CalculateDistancesForPage24();
  CAirspaceList& GetAirspacesForPage24() { return _airspaces_page24; }

  // Get nearest instance for sideview drawing & sonar system
  CAirspacePtr GetNearestAirspaceForSideview() { return _sideview_nearest.lock(); }

  //Locking
  Mutex& MutexRef() const { return _csairspaces; }

  /// to Enable/disable aispace depending of day num (SAT/SUN)
  void AutoDisable(const NMEA_INFO& info);

private:

  CAirspaceManager() = default;
  CAirspaceManager(const CAirspaceManager&) = delete;
  CAirspaceManager& operator=(const CAirspaceManager&) = delete;
  ~CAirspaceManager() { CloseAirspaces(); }

  // Airspaces data
  mutable Mutex _csairspaces; // recursive mutex is needed.
  CAirspaceList _airspaces;             // ALL
  CAirspaceList _airspaces_near;        // Near, in reachable range for warnings
  CAirspaceList _airspaces_page24;      // Airspaces for nearest 2.4 page
  CAirspaceWeakPtr _selected_airspace;         // Selected airspace
  CAirspaceWeakPtr _sideview_nearest;         // Neasrest asp instance for sideview

  unsigned last_day_of_week = ~0; // used for auto disable airspace SAT/SUN

  // Warning system data
  // User warning message queue
  AirspaceWarningMessageList _user_warning_queue;                // warnings to show
  CAirspaceList _airspaces_of_interest;

  //Openair parsing functions, internal use
  bool FillAirspacesFromOpenAir(const TCHAR* szFile);
  void CreateAirspace(const TCHAR* Name, CPoint2DArray& Polygon, double Radius, const GeoPoint& Center,
                      int Type, vertical_bound&& Base, vertical_bound&& Top, const tstring& Comment,
                      bool flyzone, bool enabled, bool except_saturday, bool except_sunday);

  static bool StartsWith(const TCHAR *Text, const TCHAR *LookFor);

  static bool ReadCoords(TCHAR *Text, double *X, double *Y);
  static bool CalculateArc(TCHAR *Text, CPoint2DArray *_geopoints, double Center_lon, double Center_lat, int Rotation);
  static bool CalculateSector(TCHAR *Text, CPoint2DArray *_geopoints, double Center_lon, double Center_lat, int Rotation);
  static bool CorrectGeoPoints(CPoint2DArray &points);

  static void AddGeodesicLine(CPoint2DArray &points, double lat, double lon);
  static void AddGeodesicLine_FAI(CPoint2DArray &points, double lat, double lon);
#ifdef _WGS84
  static void AddGeodesicLine_WGS84(CPoint2DArray &points, double lat, double lon);
#endif

  bool FillAirspacesFromOpenAIP(const TCHAR* szFile);

  //Airspace setting save/restore functions
  void SaveSettings() const;
  void LoadSettings();

};


//dlgAirspaceWarning
int dlgAirspaceWarningInit();
int dlgAirspaceWarningDeInit();

void ShowAirspaceWarningsToUser();


//Data struct for nearest airspace pages
struct LKAirspace_Nearest_Item {
  LKAirspace_Nearest_Item() = default;
  LKAirspace_Nearest_Item(const CAirspacePtr& pAsp) = delete;

  LKAirspace_Nearest_Item& operator=(const CAirspacePtr& pAsp);

  bool Valid = false;                       // Struct item is valid
  TCHAR Name[NAME_SIZE + 1];                // 1)  Name of airspace . We shall use only 15 to 25 characters max in any case
  TCHAR Type[5];                            // 2)  Type of airspace    like CTR   A B C etc.    we use 3-4 chars
  double Distance;                          // 3)  Distance
  double Bearing;                           // 4)  Bearing (so we can sort by airspaces we have in front of us, for example)
  bool Enabled;                             // 5)  Active - not active
  bool Selected;                            // 6)  Selected / Not selected (infobox calc)
  bool Flyzone;                             // 7)  True if this is a flyzone (normally fly-in zone)
  AirspaceWarningLevel_t WarningLevel;      // 8)  Actual warning level fro this airspace
  AirspaceWarningLevel_t WarningAckLevel;   // 9)  Actual ack level fro this airspace

  CAirspaceWeakPtr Pointer;                 // 10) Pointer to CAirspace class for further operations (don't forget CAirspacemanager mutex!)
};

#endif
