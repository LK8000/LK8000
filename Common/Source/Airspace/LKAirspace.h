#if !defined(AFX_LKAIRSPACE_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_LKAIRSPACE_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_


#include "Thread/Mutex.hpp"
#include "Point2D.h"
#include "Math/Point2D.hpp"

#include <tchar.h>
#include <vector>
#include <deque>
#include <list>
#include <algorithm>
#include <zzip/zzip.h>
#include "Screen/LKSurface.h"
#include "Geographic/GeoPoint.h"
#include "Airspace.h"

class ScreenProjection;
class MD5;

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
// Define this, if airspace nearest distance infoboxes will use selected airpsace only
// In this case infoboxes show distance to selected airspace only.
// I use this define, because lot of variables and calculations not needed, if we use
// selected airspace for infoboxes. - Kalman
// Will be permanent in the future.
#define LKAIRSP_INFOBOX_USE_SELECTED

enum AirspaceAltBase_t {
  abUndef = 0,
  abMSL,
  abAGL,
  abFL
};

struct AIRSPACE_ALT {
  double Altitude;
  double FL;
  double AGL;
  AirspaceAltBase_t Base;
};

#ifdef HAVE_GLES
using ScreenPointList = std::vector<FloatPoint>;
#else
using ScreenPointList = std::vector<RasterPoint>;
#endif
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
class CAirspaceBase
{
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
  void Init(const TCHAR *name, const int type, const AIRSPACE_ALT &base, const AIRSPACE_ALT &top, bool flyzone, const TCHAR *comment = NULL);

  const TCHAR* TypeName() const;
  const LKColor& TypeColor() const;
  const LKBrush& TypeBrush() const;

  const TCHAR* Name() const { return _name; }
  const TCHAR* Comment() const { return _shared_comment.get(); }

  const AIRSPACE_ALT* Top() const { return &_top; }
  const AIRSPACE_ALT* Base() const { return &_base; }
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

#ifndef LKAIRSP_INFOBOX_USE_SELECTED
  // Get class attributes for infobox values
  static TCHAR* GetNearestHName() { return _nearesthname; }
  static TCHAR* GetNearestVName() { return _nearestvname; }
  static int GetNearestHDistance() { return _nearesthdistance; }
  static int GetNearestVDistance() { return _nearestvdistance; }
#endif

protected:
  TCHAR _name[NAME_SIZE + 1] = {};                    // Name

  std::shared_ptr<TCHAR> _shared_comment ;       // extended airspace informations e.g. for Notams

  int _type = 0;                                    // type (class) of airspace
  AIRSPACE_ALT _base = {};                            // base altitude
  AIRSPACE_ALT _top = {};                            // top altitude
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
  void AirspaceAGLLookup(double av_lat, double av_lon, double *basealt_out, double *topalt_out) const;

  // Class attributes
#ifndef LKAIRSP_INFOBOX_USE_SELECTED
  static int _nearesthdistance;                // collecting horizontal distance to infobox
  static int _nearestvdistance;                // collecting vertical distance to infobox
  static TCHAR *_nearesthname;                // collecting nearest horizontal name to infobox
  static TCHAR *_nearestvname;                // collecting nearest vertical name to infobox
#endif
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

class CAirspace : public CAirspaceBase {
public:

    CAirspace() : CAirspaceBase() { }

    CAirspace(CPoint2DArray&& points) : _geopoints(std::forward<CPoint2DArray>(points)) { }

    virtual ~CAirspace() { }

    // Check if a point horizontally inside in this airspace
    virtual bool IsHorizontalInside(const double &longitude, const double &latitude) const = 0;
    // Dump this airspace to runtime.log
    virtual void Dump() const = 0;
    // Calculate drawing coordinates on screen
    virtual void CalculateScreenPosition(const rectObj &screenbounds_latlon, const int iAirspaceMode[], const int iAirspaceBrush[], const RECT& rcDraw, const ScreenProjection& _Proj);
    // Draw airspace on map
    virtual void Draw(LKSurface& Surface, bool fill) const;
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

    static void ResetSideviewNearestInstance() { _sideview_nearest_instance = NULL; }
    static CAirspace* GetSideviewNearestInstance() { return _sideview_nearest_instance; }

protected:
    // polygon points : circular airspace are also stored like polygon because that avoid to calculate geographic coordinate for each drawing
    // previous version draw circular airspace using circle, but it's wrong, circle in geographic coordinate are ellipsoid in screen coordinate.
    CPoint2DArray _geopoints;

    // this 2 array are modified by DrawThread, never use it in another thread !!
    ScreenPointList _screenpoints; // this is member for reduce memory alloc, but is used only by CalculateScreenPosition();
    RasterPointList _screenpoints_clipped;

    ////////////////////////////////////////////////////////////////////////////////
    // Draw Picto methods
    //  this methods are NEVER used at same time of airspace loading
    //  therefore it can be considered is thread safe
public:
    virtual void DrawPicto(LKSurface& Surface, const RECT &rc) const;
protected:
    virtual void CalculatePictPosition(const RECT& rcDraw, double zoom, RasterPointList &screenpoints_picto) const;
    ////////////////////////////////////////////////////////////////////////////////

    static CAirspace* _sideview_nearest_instance;         // collect nearest airspace instance for sideview during warning calculations
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
  CAirspace *psAS;
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
  // Dump this airspace to runtime.log
  void Dump() const override;

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
  // Dump this airspace to runtime.log
  void Dump() const override;

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
using CAirspaceList = std::deque<CAirspace*>;

//Warning system generated message
struct AirspaceWarningMessage {
  CAirspace *originator;                // airspace instance
  AirspaceWarningEvent event;            // message cause
  AirspaceWarningLevel_t warnlevel;        // warning level
};
// Warning message queue
using AirspaceWarningMessageList = std::deque<AirspaceWarningMessage>;

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
  static CAirspaceManager& Instance() { return _instance; }

  //HELPER FUNCTIONS
  static bool CheckAirspaceAltitude(const AIRSPACE_ALT &Base, const AIRSPACE_ALT &Top);
  static const TCHAR* GetAirspaceTypeText(int type);
  static const TCHAR* GetAirspaceTypeShortText(int type);
  static void GetAirspaceAltText(TCHAR *buffer, int bufferlen, const AIRSPACE_ALT *alt);
  static void GetSimpleAirspaceAltText(TCHAR *buffer, int bufferlen, const AIRSPACE_ALT *alt);


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

  CAirspace* FindNearestAirspace(const double &longitude, const double &latitude,
             double *nearestdistance, double *nearestbearing, double *height = NULL) const;
  void SortAirspaces();
  bool ValidAirspaces() const;
  //Warning system
  void AirspaceWarning (NMEA_INFO *Basic, DERIVED_INFO *Calculated);
  bool AirspaceWarningIsGoodPosition(float longitude, float latitude, int alt, int agl) const;

  void AirspaceSetAckLevel(CAirspace &airspace, AirspaceWarningLevel_t ackstate);
  void AirspaceAckWarn(CAirspace &airspace);
  void AirspaceAckSpace(CAirspace &airspace);
  void AirspaceDisable(CAirspace &airspace);
  void AirspaceEnable(CAirspace &airspace);
  void AirspaceFlyzoneToggle(CAirspace &airspace);

  bool PopWarningMessage(AirspaceWarningMessage *msg);
  void AirspaceWarningLabelPrinted(CAirspace &airspace, bool success);

  //Get/Set airspace details (dlgAirspaceDetails)
  CAirspaceList GetVisibleAirspacesAtPoint(const double &lon, const double &lat) const;
  CAirspaceList GetNearAirspacesAtPoint(const double &lon, const double &lat, long range) const;

  const CAirspaceList GetAllAirspaces() const;
  const CAirspaceList& GetAirspacesForWarningLabels();
  CAirspaceList GetAirspacesInWarning() const;
  CAirspaceBase GetAirspaceCopy(const CAirspaceBase* airspace) const;
  bool AirspaceCalculateDistance(CAirspace *airspace, int *hDistance, int *Bearing, int *vDistance);
  void AirspaceSetSelect(CAirspace &airspace);

  //Mapwindow drawing
  void SetFarVisible(const rectObj &bounds_active);
  void CalculateScreenPositionsAirspace(const rectObj &screenbounds_latlon, const int iAirspaceMode[], const int iAirspaceBrush[], const RECT& rcDraw, const ScreenProjection& _Proj);
  const CAirspaceList& GetNearAirspacesRef() const;

  //Nearest page 2.4
  void SelectAirspacesForPage24(const double latitude, const double longitude, const double interest_radius);
  void CalculateDistancesForPage24();
  CAirspaceList GetAirspacesForPage24() const { return _airspaces_page24; }

  //Sideview
  CAirspace* GetNearestAirspaceForSideview() { return _sideview_nearest; }     // Get nearest instace for sideview drawing (use instance ptr as key only to asp manager (mutex!))

  //Locking
  Mutex& MutexRef() const { return _csairspaces; }

  // Airspaces detail system accessor
  void PopupAirspaceDetail(CAirspace * pAsp);
  void ProcessAirspaceDetailQueue();

  CAirspace* GetAirspacesForDetails() { return _detail_current; } // call this only inside Mutex Guard section !

  /// to Enable/disable aispace depending of day num (SAT/SUN)
  void AutoDisable(const NMEA_INFO& info);

private:
  static CAirspaceManager _instance;
  CAirspaceManager() { _selected_airspace = NULL; _sideview_nearest = NULL; }
  CAirspaceManager(const CAirspaceManager&) = delete;
  CAirspaceManager& operator=(const CAirspaceManager&) = delete;
  ~CAirspaceManager() { CloseAirspaces(); }

  // Airspaces data
  mutable Mutex _csairspaces; // recursive mutex is needed.
  CAirspaceList _airspaces;             // ALL
  CAirspaceList _airspaces_near;        // Near, in reachable range for warnings
  CAirspaceList _airspaces_page24;      // Airspaces for nearest 2.4 page
  CAirspace *_selected_airspace = nullptr;         // Selected airspace
  CAirspace *_sideview_nearest = nullptr;         // Neasrest asp instance for sideview

  unsigned last_day_of_week = ~0; // used for auto disable airspace SAT/SUN

  // Warning system data
  // User warning message queue
  AirspaceWarningMessageList _user_warning_queue;                // warnings to show
  CAirspaceList _airspaces_of_interest;

  // Airspaces detail system data
  CAirspace * _detail_current = nullptr;
  CAirspaceList _detail_queue;

  //Openair parsing functions, internal use
  bool FillAirspacesFromOpenAir(const TCHAR* szFile);
  
  static bool StartsWith(const TCHAR *Text, const TCHAR *LookFor);
  static void ReadAltitude(const TCHAR *Text, AIRSPACE_ALT *Alt);
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
  bool ReadAltitudeOpenAIP(const xml_node* node, AIRSPACE_ALT *Alt) const;

  //Airspace setting save/restore functions
  void SaveSettings() const;
  void LoadSettings();

};


//dlgAirspaceWarning
int dlgAirspaceWarningInit();
int dlgAirspaceWarningDeInit();

short ShowAirspaceWarningsToUser();


//Data struct for nearest airspace pages
struct LKAirspace_Nearest_Item {
  bool Valid;                               // Struct item is valid
  TCHAR Name[NAME_SIZE+1];                  // 1)  Name of airspace . We shall use only 15 to 25 characters max in any case
  TCHAR Type[5];                            // 2)  Type of airspace    like CTR   A B C etc.    we use 3-4 chars
  double Distance;                          // 3)  Distance
  double Bearing;                           // 4)  Bearing (so we can sort by airspaces we have in front of us, for example)
  bool Enabled;                             // 5)  Active - not active
  bool Selected;                            // 6)  Selected / Not selected (infobox calc)
  bool Flyzone;                             // 7)  True if this is a flyzone (normally fly-in zone)
  AirspaceWarningLevel_t WarningLevel;      // 8)  Actual warning level fro this airspace
  AirspaceWarningLevel_t WarningAckLevel;   // 9)  Actual ack level fro this airspace

  CAirspace *Pointer;                       // 10) Pointer to CAirspace class for further operations (don't forget CAirspacemanager mutex!)
};


#endif
