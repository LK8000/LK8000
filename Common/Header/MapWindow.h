/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: MapWindow.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#if !defined(AFX_MAPWINDOW_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_MAPWINDOW_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#include "Airspace.h"
#include "Sizes.h"
#include "Defines.h"
#include "Parser.h"
#include "Calculations.h"
#include "Screen/BrushReference.h"
#include "Screen/PenReference.h"
#include "Screen/LKBitmap.h"
#include "Screen/LKIcon.h"
#include "Screen/LKBitmapSurface.h"
#include "Screen/LKWindowSurface.h"
#include "Time/PeriodClock.hpp"
#include <array>
#include "RGB.h"
#include "Thread/Mutex.hpp"
#include "Enums.h"
#include "ContestMgr.h"

#ifndef ENABLE_OPENGL
#include "Poco/ThreadTarget.h"
#include "Poco/Thread.h"
#include "Poco/Event.h"
#endif
#define SCALELISTSIZE  24

#define TARGETUP   6
#define NORTHSMART 5
#define NORTHTRACK 4
#define TARGETCIRCLE 3
#define NORTHCIRCLE 2
#define NORTHUP 1
#define TRACKUP 0

#define DISPLAYNAME 0
#define DISPLAYNUMBER 1
#define DISPLAYNAMEIFINTASK 2
#define DISPLAYFIRSTTHREE 3
#define DISPLAYFIRSTFIVE 4
#define DISPLAYFIRST8 5
#define DISPLAYFIRST10 6
#define DISPLAYFIRST12 7
#define DISPLAYNONE 8
#define DISPLAYICAO 9

#define AIRPORT		0x01
#define TURNPOINT	0x02
#define LANDPOINT	0x04
#define HOME		0x08
#define START		0x10
#define FINISH		0x20
#define RESTRICTED	0x40
#define WAYPOINTFLAG	0x80


#define TEXT_NO_TEXT        0
#define TEXT_ABOVE_LEFT     1
#define TEXT_ABOVE_RIGHT    2
#define TEXT_ABOVE_CENTER   3
#define TEXT_UNDER_LEFT     4
#define TEXT_UNDER_RIGHT    5
#define TEXT_UNDER_CENTER   6
#define TEXT_MIDDLE_LEFT    7
#define TEXT_MIDDLE_RIGHT   8
#define TEXT_MIDDLE_CENTER  9


// The following 3 const vectors define map scale range and levels for each unit type.
static constexpr double ScaleListArrayMeters[SCALELISTSIZE] = {
    0.01,
    0.015,
    0.025,
    0.040,
    0.070,
    0.1,
    0.15,
    0.2,
    0.35,
    0.5,
    0.75,
    1.0,
    1.5,
    2.0,
    3.5,
    5.0,
    7.5,
    10.0,
    15.0,
    20.0,
    25.0,
    40.0,
    50.0,
    75.0
};
static constexpr double ScaleListArrayStatuteMiles[SCALELISTSIZE] = {
    50.0 * (0.0006214 / 3.281),
    80.0 * (0.0006214 / 3.281),
    130.0 * (0.0006214 / 3.281),
    200.0 * (0.0006214 / 3.281),
    350.0 * (0.0006214 / 3.281),
    500.0 * (0.0006214 / 3.281),
    800.0 * (0.0006214 / 3.281),
    0.2,
    0.35,
    0.5,
    0.75,
    1.0,
    1.5,
    2.0,
    3.5,
    5.0,
    7.5,
    10.0,
    15.0,
    20.0,
    25.0,
    40.0,
    50.0,
    75.0,
};
static constexpr double ScaleListNauticalMiles[SCALELISTSIZE] = {
    50.0 * (0.00053996 / 3.281),
    100.0 * (0.00053996 / 3.281),
    150.0 * (0.00053996 / 3.281),
    250.0 * (0.00053996 / 3.281),
    400.0 * (0.00053996 / 3.281),
    600.0 * (0.00053996 / 3.281),
    900.0 * (0.00053996 / 3.281),
    0.2,
    0.35,
    0.5,
    0.75,
    1.0,
    1.5,
    2.0,
    3.5,
    5.0,
    7.5,
    10.0,
    15.0,
    20.0,
    25.0,
    40.0,
    50.0,
    75.0
};

struct WAYPOINT
{
  // Number is used as an alternate short name, and can also be duplicated
  int Number;
  double Latitude;
  double Longitude;
  double Altitude;
  int Flags;
  TCHAR Name[NAME_SIZE + 1];
  TCHAR *Comment;
  int UnusedZoom;	// THIS IS UNUSED AND CAN BE REALLOCATED. WE DONT REMOVE TO KEEP COMPATIBILITY WITH OLD TASKS!
  BOOL Reachable;
  double AltArivalAGL;
  BOOL Visible;
  bool InTask;
  TCHAR *Details;
  bool FarVisible;
  int FileNum; // which file it is in, or -1 to delete
  // waypoint original format, LKW_DAT CUP etc.
  short Format;
  TCHAR Code[CUPSIZE_CODE+1];
  TCHAR Freq[CUPSIZE_FREQ+1];
  int   RunwayLen;
  int   RunwayDir;
  TCHAR Country[CUPSIZE_COUNTRY+1];
  short Style;
};

inline
GeoPoint GetWayPointPosition(const WAYPOINT& p) {
	return { p.Latitude, p.Longitude };
}

// This struct is separated from 'WAYPOINT' and will not be used in task files.
// It is managed by the same functions that manage WayPointList, only add variables here
// and use them like  WayPointCalc[n].Distance  for example.
// THIS STRUCTURE MUST BE INITIALIZED inside waypointparser function!!
struct WPCALC
{
//  long timeslot;
  double GR;       // GR from current position
  short VGR;       // Visual GR
  double Distance; // distance from current position
  double Bearing;  // used for radial
  double AltArriv[ALTA_SIZE];
  double NextETE;
  bool Preferred;  // Flag to be used by Preferred quick selection WP page (TODO) and by BestAlternate
  double AltReqd[ALTA_SIZE];
  short WpType;
  // quick flags. if not landable, it's a turnpoint
  bool IsLandable;
  // if on, then they are also landable for sure
  bool IsAirport;
  bool IsOutlanding;
};

typedef struct _SNAIL_POINT
{
  float Latitude;
  float Longitude;
  double Time;
  unsigned short Colour;
  BOOL Circling;
  double DriftFactor;
} SNAIL_POINT;

typedef struct _LONG_SNAIL_POINT
{
  float Latitude;
  float Longitude;
  bool FarVisible;
} LONG_SNAIL_POINT;

typedef struct {
    bool Border;
    bool FillBackground;
    bool AlligneRight;
    bool Reachable;
    bool AlligneCenter;
    bool WhiteBorder;
    bool WhiteBold;
    bool NoSetFont;  // VENTA5
    bool SetTextColor;  // Set text color in border mode
    LKColor Color;
}TextInBoxMode_t;


typedef struct{
  TCHAR Name[LKSIZEBUFFER];
  POINT Pos;
  TextInBoxMode_t Mode;
  int AltArivalAGL;
  bool inTask;
  bool isLandable; // VENTA5
  bool isThermal;
  int  index;
  short style;
}MapWaypointLabel_t;


typedef struct
{
  double fXMin, fXMax;
  double fYMin, fYMax;
  RECT rc;
} DiagrammStruct;

class ScreenProjection;

class MapWindow {
 public:
  /**
   * @brief Class responsible for handling all Map Zoom activities
   *
   * Zoom class is responsible for:
   *  - storing Map Zoom state (global and for each of the modes)
   *  - provide Zoom related calculations
   */
  class Zoom {
  private:

    enum TMapScaleType {
      SCALE_CRUISE,             /**< @brief Basic zoom for flight mode used for:
                                   - cruise mode when AutoZoom is disabled
                                   - thermalling mode when CirclingZoom and AutoZoom is disabled
                                   - restore user zoom when next waypoint with AutoZoom enabled
                                */
      SCALE_CIRCLING,           /**< @brief Zoom for thermalling mode when CirclingZoom is enabled */
      SCALE_PANORAMA,           /**< @brief Panorama 20 seconds zoom */
      SCALE_AUTO_ZOOM,          /**< @brief AutoZoom zoom */
      SCALE_PAN,                /**< @brief PAN zoom */
      SCALE_TARGET_PAN,         /**< @brief TARGET_PAN zoom */

      SCALE_NUM                 /**< @brief DO NOT USE THAT */
    };

    friend class MapWindow;
    bool  _bMapScale;
    bool _inited;                                 /**< @brief Object inited flag */
    bool _autoZoom;                               /**< @brief Stores information if AutoZoom is enabled */
    bool _circleZoom;                             /**< @brief Stores information if CirclingZoom is enabled */
    bool _bigZoom;                                /**< @brief Stores information if BigZoom was done and special refresh is needed */
    double _scale;                                /**< @brief Current map scale */
    double _realscale;                            /**< @brief Current map scale /1000 / DISTANCEMODIFY */
    double _modeScale[SCALE_NUM];                 /**< @brief Requested scale for each of scale types */
    double *_requestedScale;                      /**< @brief Requested scale for current scale type */
    // performance related members
    double _resScaleOverDistanceModify;
    double _drawScale;
    double _invDrawScale;

    double RequestedScale() const        { return *_requestedScale; }
    void RequestedScale(double value)    { *_requestedScale = value; }
    void CalculateTargetPanZoom();
    void CalculateAutoZoom();
    double ResScaleOverDistanceModify() const { return _resScaleOverDistanceModify; }
    double DrawScale() const             { return _drawScale; }
    double InvDrawScale() const          { return _invDrawScale; }

    double GetZoomInitValue(int parameter_number) const;

  public:

    Zoom();
    void Reset();

    void AutoZoom(bool enable)      { _autoZoom = enable; SwitchMode(); }
    bool AutoZoom() const           { return _autoZoom; }

    void CircleZoom(bool enable)    { _circleZoom = enable; SwitchMode(); }
    bool CircleZoom() const         { return _circleZoom; }

    void BigZoom(bool enable)       { _bigZoom = enable; }
    bool BigZoom() const            { return _bigZoom; }

    void SwitchMode();

    double Scale() const { return _scale; }
    double RealScale() const { return _realscale; }

    void EventAutoZoom(int vswitch);
    void EventSetZoom(double value);
    void EventScaleZoom(int vswitch);
    void SetLimitMapScale(BOOL bOnOff)    {  	_bMapScale =	bOnOff; };
    void UpdateMapScale();
    void ModifyMapScale();

    bool GetInitMapScaleText(int init_parameter, TCHAR *out, size_t size) const;

  };


  /**
   * @brief Class responsible for handling Map Display Mode data
   *
   * There are 2 types of map display modes:
   *  - fly
   *     - modes representing current flight mode (ie. cruise, circle, final glide)
   *     - only one of them can be active at a time
   *  - special
   *     - modes representing special actions like (PANORAMA, PAN, etc.)
   *     - none or more than one special state can be enabled at the same time
   */
  class Mode {
  public:
    /**
     * @brief Fly Modes
     */
    enum TModeFly {
      MODE_FLY_NONE        = 0x0000,
      MODE_FLY_CRUISE      = 0x0001,
      MODE_FLY_CIRCLING    = 0x0002,
      MODE_FLY_FINAL_GLIDE = 0x0004
    };
    static const unsigned FLY_MASK = 0x00FF;

    /**
     * @brief Special Modes
     */
    enum TModeSpecial {
      MODE_SPECIAL_NONE        = 0x0000,
      MODE_SPECIAL_PAN         = 0x0100,
      MODE_SPECIAL_TARGET_PAN  = 0x0200,
      MODE_SPECIAL_PANORAMA    = 0x0400
    };
    static const unsigned SPECIAL_MASK = 0xFF00;

    /**
     * @brief All Display Modes
     */
    enum TMode {
      MODE_NONE        = MODE_FLY_NONE,

      MODE_CRUISE      = MODE_FLY_CRUISE,
      MODE_CIRCLING    = MODE_FLY_CIRCLING,
      MODE_FINAL_GLIDE = MODE_FLY_FINAL_GLIDE,

      MODE_PAN         = MODE_SPECIAL_PAN,
      MODE_TARGET_PAN  = MODE_SPECIAL_TARGET_PAN,
      MODE_PANORAMA    = MODE_SPECIAL_PANORAMA
    };

  private:
    friend class Zoom;
    unsigned _mode;                    /**< @brief Current Map Display Mode */
    unsigned _lastMode;                /**< @brief Previous Map Display Mode */
    TModeFly _userForcedFlyMode;       /**< @brief Fly Mode forced by a user */
    bool _autoNorthUP ;


   public:
    Mode();
    bool autoNorthUP(){return _autoNorthUP;};
    void setAutoNorthUP(bool t_AutoNorthUP) {_autoNorthUP = t_AutoNorthUP;};

    TModeFly UserForcedMode() const    { return _userForcedFlyMode; }
    void UserForcedMode(TModeFly umode) { _userForcedFlyMode = umode; }

    bool Is(TMode tmode) const   { return _mode & tmode; }
    void Fly(TModeFly flyMode);
    TModeFly Fly() const        { return static_cast<TModeFly>(_mode & FLY_MASK); }

    void Special(TModeSpecial specialMode, bool enable);
    TModeSpecial Special() const { return static_cast<TModeSpecial>(_mode & SPECIAL_MASK); }

    bool AnyPan() const { return _mode & (MODE_SPECIAL_PAN | MODE_SPECIAL_TARGET_PAN); }
  };


  static bool IsDisplayRunning();
  static int iAirspaceMode[AIRSPACECLASSCOUNT];
#ifdef HAVE_HATCHED_BRUSH
  static int iAirspaceBrush[AIRSPACECLASSCOUNT];
#endif
  static int iAirspaceColour[AIRSPACECLASSCOUNT];
  static BOOL CLOSETHREAD;

  static const LKColor& GetAirspaceColour(int i) {
    return Colours[i];
  }

  static const LKBrush& GetAirspaceBrush(int i) {
    return hAirspaceBrushes[i];
  }
  static const LKBrush& GetAirspaceBrushByClass(int i) {
#ifdef HAVE_HATCHED_BRUSH
    return GetAirspaceBrush(iAirspaceBrush[i]);
#else
    return GetAirspaceBrush(iAirspaceColour[i]);
#endif
  }

  static const LKColor& GetAirspaceColourByClass(int i) {
    return Colours[iAirspaceColour[i]];
  }


  // initialize solid color brushes for airspace drawing (initializes hAirSpaceSldBrushes[])
  static void InitAirSpaceSldBrushes(const LKColor colours[]);


 private:

  static BOOL Initialised;
  static bool GliderCenter;
  static PeriodClock timestamp_newdata;
  static bool LandableReachable;

 public:

  // 12 is number of airspace types
  static LKColor Colours[NUMAIRSPACECOLORS];
  static LKPen hAirspacePens[AIRSPACECLASSCOUNT];
  static LKPen hBigAirspacePens[AIRSPACECLASSCOUNT];
  static LKPen hAirspaceBorderPen;

  static LKPen hSnailPens[NUMSNAILCOLORS+1];

  static LKBrush hAirspaceBrushes[NUMAIRSPACEBRUSHES];

  // solid brushes for airspace drawing (initialized in InitAirSpaceSldBrushes())
  static LKBrush hAirSpaceSldBrushes[NUMAIRSPACECOLORS];

#ifdef ENABLE_OPENGL
  static LKColor AboveTerrainColor;
#else
  static LKBrush hAboveTerrainBrush;
#endif

  static Zoom zoom;
  static Mode mode;

  static RECT MapRect;			// See explanation in MapWndProc
  static RECT DrawRect;
  static bool ForceVisibilityScan;
  static bool ThermalBarDrawn;

  static bool MapDirty;

  static bool EnableTrailDrift;
  static int GliderScreenPosition;
  static int GliderScreenPositionX;
  static int GliderScreenPositionY;
  static void RequestFastRefresh();

  static void UpdateTimeStats(bool start);

  // Drawing primitives

  static void DrawMulticolorDashLine(LKSurface& , const int , const POINT& , const POINT& ,
			   const LKColor& , const LKColor&,
			   const RECT&);

  static void DrawBitmapIn(LKSurface& Surface, const POINT &sc, const LKIcon& Icon);

  // ...
  static void RequestToggleFullScreen();
  static void RequestOnFullScreen();
  static void RequestOffFullScreen();

  static void Initialize();
  static void CloseDrawingThread(void);
  static void CreateDrawingThread(void);
  static void SuspendDrawingThread(void);
  static void ResumeDrawingThread(void);

  static LKColor GetOutlineColor(LKColor color);
  static const LKIcon* GetDrawBmpIcon(DrawBmp_t bmp);

  static void LKWriteText(LKSurface& Surface, const TCHAR* wText, int x, int y, const bool mode, const short align, const LKColor& rgb_tex, bool invertable, RECT* ClipRect = nullptr);
  static void LKWriteBoxedText(LKSurface& Surface, const RECT& clipRect, const TCHAR* wText, int x, int y, const short align, const LKColor& rgb_dir, const LKColor& rgb_inv );

  static bool LKFormatValue(const short fvindex, const bool longtitle, TCHAR *BufferValue, TCHAR *BufferUnit, TCHAR *BufferTitle,DrawBmp_t *BmpValue = NULL,DrawBmp_t *BmpTitle = NULL);
  static void LKgetOLCBmp(CContestMgr::TType Type,DrawBmp_t *BmpValue,TCHAR *BufferValue = NULL);
  static void LKFormatBrgDiff(const int wpindex, TCHAR *BufferValue, TCHAR *BufferUnit);

  static bool IsMapFullScreen();
  static bool ChangeDrawRect(const RECT rectarea);

  // input events or reused code
  static void Event_Pan(int vswitch);
  static void Event_TerrainTopology(int vswitch);
  static void Event_PanCursor(int dx, int dy);
  static bool Event_InteriorAirspaceDetails(double lon, double lat);
  static bool Event_NearestWaypointDetails(double lon, double lat);

  static void UpdateInfo(NMEA_INFO *nmea_info,
			 DERIVED_INFO *derived_info);
  static rectObj CalculateScreenBounds(double scale, const RECT& rc, const ScreenProjection& _Proj);
  static void ScanVisibility(rectObj *bounds_active);

  static int HeightToY(double fHeight,  DiagrammStruct* psDia);
  static int DistanceToX(double fDist,  DiagrammStruct* psDia)  ;
  static void RenderNearAirspace(LKSurface& Surface, const RECT& rci);
  static int SharedTopView(LKSurface& Surface, DiagrammStruct* pDia, double iAS_Bearing, double wpt_brg);
  static void RenderAirspace(LKSurface& Surface, const RECT& rc);
  static void DrawVisualGlide (LKSurface& Surface, const DiagrammStruct& sDia);
  static short GetVisualGlidePoints(unsigned short numslots );
  static void LKDrawFlarmRadar(LKSurface& Surface, const RECT& rci);
  static void LKDrawMultimap_Example(LKSurface& Surface, const RECT& rci);
  static void LKDrawMultimap_Test(LKSurface& Surface, const RECT& rci);
  static void LKDrawMultimap_Asp(LKSurface& Surface, const RECT& rci);
  static void LKDrawMultimap_Radar(LKSurface& Surface, const RECT& rci);

  static void DrawMultimap_SideTopSeparator(LKSurface& Surface, const RECT& rci);



  static int DrawFlarmObjectTrace(LKSurface& Surface,double fZoom, DiagrammStruct* Dia);

/**
 * DrawRunway : _Proj can be null only if Picto = true !!
 */
  static void DrawRunway(LKSurface& Surface, const WAYPOINT* wp, const RECT& rc, const ScreenProjection* _Proj,  double fScaleFact, BOOL Picto = false);

  static void DrawTaskPicto(LKSurface& Surface, int TaskIdx, const RECT& rc, double fScaleFact);
  static void DrawWaypointPictoBg(LKSurface& Surface, const RECT& rc);
  static void DrawWaypointPicto(LKSurface& Surface, const RECT& rc, const WAYPOINT* wp);
  static void DrawFlarmPicto(LKSurface& hDC, const RECT& rc, FLARM_TRAFFIC*);
  static void DrawWeatherStPicto(LKSurface& Surface, const RECT& rc, FANET_WEATHER* pWeather);
  static void DrawAircraft(LKSurface& Surface, const POINT& Orig);
 private:
  static void DrawAHRS(LKSurface& Surface, const RECT& rc);
  static void DrawCompassRose(LKSurface& Surface, const RECT& rc, double direction);
  static ScreenProjection CalculateScreenPositions(const POINT& Orig, const RECT& rc, POINT *Orig_Aircraft);
  static void CalculateScreenPositionsGroundline(const ScreenProjection& _Proj);
  static void CalculateScreenPositionsAirspace(const RECT& rcDraw, const ScreenProjection& _Proj);
  static void CalculateScreenPositionsThermalSources(const ScreenProjection& _Proj);
  static void LKCalculateWaypointReachable(const bool forced);

  static bool PointVisible(const POINT &P);
  static bool PointVisible(const double &lon, const double &lat);
  static bool PointInRect(const double &lon, const double &lat,
			  const rectObj &bounds);


  static void DrawHeading(LKSurface& Surface, const POINT& Orig, const RECT& rc); // VENTA10
  static void DrawBestCruiseTrack(LKSurface& Surface, const POINT& Orig);
  static void DrawTRI(LKSurface&, const RECT& rc);
  static void DrawAcceleration(LKSurface& Surface, const RECT& rc);
  static void DrawTarget(LKSurface& Surface, const RECT& rc,int ttop,int tbottom,int tleft,int tright);

  static void DrawWindAtAircraft2(LKSurface& Surface, const POINT& Orig, const RECT& rc);
  static void DrawAirSpace(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj);
#ifdef HAVE_HATCHED_BRUSH
  static void DrawAirSpacePattern(LKSurface& Surface, const RECT& rc);
#endif
  static void DrawAirSpaceBorders(LKSurface& Surface, const RECT& rc);
  static void DrawAirspaceLabels(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, const POINT& Orig_Aircraft);
  static void DrawWaypointsNew(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj);
  static void DrawLook8000(LKSurface& Surface, const RECT& rc);
  static void DrawTextOrBitmap(LKSurface& Surface, const TCHAR* wText, int x, int y, const bool lwmode, const short align, const LKColor& rgb_text, bool invertable, SIZE txtSize, DrawBmp_t bmp);
  static void DrawBottomBar(LKSurface& Surface, const RECT& rc);
  static void DrawMapSpace(LKSurface& Surface, const RECT& rc);
  static void DrawNearest(LKSurface& Surface, const RECT& rc);
  static void DrawAspNearest(LKSurface& Surface, const RECT& rc);
  static void DrawCommon(LKSurface& Surface, const RECT& rc);
  static void DrawInfoPage(LKSurface& Surface, const RECT& rc, const bool forceinit);
  static void DrawTraffic(LKSurface& Surface, const RECT& rc);
  static void DrawThermalHistory(LKSurface& Surface, const RECT& rc);
  static void DrawGAscreen(LKSurface& Surface, const POINT& AircraftPos, const RECT& rc);
  static void DrawHeadUpLine(LKSurface& Surface, const POINT& Orig, const RECT& rc , double, double);
  static void DrawFuturePos(LKSurface& Surface, const POINT& Orig, const RECT& rc, const bool headUpLine = false);
  static void DrawHSI(LKSurface& Surface, const RECT& rc, bool& usingQFU, bool& approach, bool& landing);
  static void WriteInfo(LKSurface& Surface, bool *showunit, TCHAR *BufferValue, TCHAR *BufferUnit, TCHAR *BufferTitle,
                                short *columnvalue, short *columntitle, short *row1, short *row2, short *row3,DrawBmp_t *BmpValue = NULL);
  // static bool LKFormatValue(const short fvindex, const bool longtitle, TCHAR *BufferValue, TCHAR *BufferUnit, TCHAR *BufferTitle);
  static void LKFormatDist(const int wpindex, TCHAR *BufferValue, TCHAR *BufferUnit);
  // static void LKFormatBrgDiff(const int wpindex, const bool wpvirtual, TCHAR *BufferValue, TCHAR *BufferUnit);
  static void LKFormatGR(const int wpindex, TCHAR *BufferValue, TCHAR *BufferUnit);
  static void LKFormatAltDiff(const int wpindex, TCHAR *BufferValue, TCHAR *BufferUnit);
  static void LKUpdateOlc(void);

  static void DrawWelcome8000(LKSurface& Surface, const RECT& rc);
  static void DrawFlightMode(LKSurface& Surface, const RECT& rc);
  static void DrawGPSStatus(LKSurface& Surface, const RECT& rc);
  static void DrawFunctions1HZ(LKSurface& Surface, const RECT& rc);

  static void DrawYGrid(LKSurface& Surface, const RECT& rc, double ticstep,double unit_step, double zero, int iTextAling,
		                const LKColor& color, DiagrammStruct *psDia, const TCHAR *pLable=NULL);
  static void DrawXGrid(LKSurface& Surface, const RECT& rc, double ticstep,double unit_step, double zero, int iTextAling,
                        const LKColor& color, DiagrammStruct *psDia,  const TCHAR *pLable=NULL);


  static void LKDrawTrail(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj);
  static void LKDrawLongTrail(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj);
  static void DrawTeammate(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj);
  static void DrawOffTrackIndicator(LKSurface& Surface, const RECT& rc);
  static void DrawProjectedTrack(LKSurface& Surface, const RECT& rc, const POINT& Orig);
  static void DrawTask(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, const POINT &Orig_Aircraft);
  static void DrawTaskSectors(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj) ;
  static void DrawFAIOptimizer(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, const POINT &Orig_Aircraft) ;
  static void DrawXC(LKSurface &Surface, const RECT &rc, const ScreenProjection &_Proj, const POINT &Orig_Aircraft);
  static void DrawThermalEstimate(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj);
  static void DrawThermalEstimateMultitarget(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj);
  static void DrawTaskAAT(LKSurface& Surface, const RECT& rc);
  static void DrawBearing(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj);
  static void DrawGreatCircle(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj,
                              double lon_start, double lat_start,
                              double lon_end, double lat_end);
protected:
  static void DrawMapScale(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj);
  static void DrawCrossHairs(LKSurface& Surface, const POINT& Orig, const RECT& rc);
  static void DrawCompass(LKSurface& Surface, const RECT& rc,const double angle);


private:
  static void DrawFinalGlide(LKSurface& Surface, const RECT& rc);
  static void DrawThermalBand(LKSurface& Surface, const RECT& rc);
  static void DrawGlideThroughTerrain(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj);
  static void DrawTerrainAbove(LKSurface& Surface, const RECT& rc);
  static void LKDrawFLARMTraffic(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, const POINT& Orig_Aircraft);
  static void LKDrawFanetData(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, const POINT& Orig_Aircraft);
  static void LKDrawVario(LKSurface& Surface, const RECT& rc);

  static bool TextInBox(LKSurface& Surface, const RECT *area, const TCHAR* Value, int x, int y, TextInBoxMode_t *Mode, bool noOverlap=false);
  static void VGTextInBox(LKSurface& Surface, const unsigned short nslot, const short numlines, const TCHAR* wText1, const TCHAR *wText2, const TCHAR *wText3, int x, int y, const LKColor& trgb, const LKBrush& bbrush);
  static void ToggleFullScreenStart();
  static bool WaypointInTask(int ind);


#ifndef ENABLE_OPENGL
private:
  static void DrawThread ();
  static Poco::ThreadTarget MapWindowThreadRun;
  static Poco::Event drawTriggerEvent;

  static LKBitmapSurface DrawSurface;

public:
  static Mutex Surface_Mutex; // Fast Mutex allow recursive lock only on Window Platform !

protected:
  static LKBitmapSurface BackBufferSurface;
  static Mutex BackBuffer_Mutex;
#else
protected:
  void Render(LKSurface& Surface, const PixelRect& Rect);
  static LKBitmapSurface BackBufferSurface;


#endif
private:
  static int iSnailNext;
  static int iLongSnailNext;

#ifndef ENABLE_OPENGL
  static LKWindowSurface WindowSurface; // used as AttribDC for Bitmap Surface.
  static LKBitmapSurface TempSurface;

  static LKMaskBitmapSurface hdcMask; // Only used For Airspaces drawing "Transparent Border" or "Paterns Borders"
  static LKBitmapSurface hdcbuffer; // Used For aispaces
#endif

  static double PanLatitude;
  static double PanLongitude;

  static double DisplayAngle;
  static double DisplayAircraftAngle;
  static unsigned targetPanSize;

 public:
  static void RefreshMap(); // set public VENTA

  static rectObj screenbounds_latlon;

  // this property is calculated by UpdateActiveScreenZone() on OnCreate(...) and OnSize(...) or user call
  static short Y_BottomBar; // this is different from BottomBarY
  static POINT P_Doubleclick_bottomright; // squared area for screen lock doubleclick, normally on right bottombar
  static POINT P_MenuIcon_DrawBottom; // Menu icon area (topleft coord)
  static POINT P_MenuIcon_noDrawBottom; // same, without bottombar drawn, forgot why it is different

  static POINT P_UngestureLeft;
  static POINT P_UngestureRight;

  static short Y_Up, Y_Down; // Up and Down keys vertical limits, ex. for zoom in out on map
  static short X_Left, X_Right; // Ungestured fast clicks on infopages (THE SAME AS IN: PROCESS_VIRTUALKEY)

  static BOOL THREADRUNNING;
  static BOOL THREADEXIT;

  static double LimitMapScale(double value);

  static void SetTargetPan(bool dopan, int task_index, unsigned dlgSize = 0);

  static void SetPanTaskEdit(unsigned TskPoint);

  static double GetPanLatitude() { return PanLatitude; }
  static double GetPanLongitude() { return PanLongitude; }
  static double GetInvDrawScale() { return zoom.InvDrawScale(); }
  static double GetDrawScale() { return zoom.DrawScale(); }
  static double GetDisplayAngle() { return DisplayAngle; }
  static void SetAutoOrientation();

  static int GetScaleListCount();

  static BrushReference hInvBackgroundBrush[LKMAXBACKGROUNDS]; // fixed number of backgrounds in MapWindow

  static      PenReference hpAircraft;
  static      LKPen hpWind;
  static      LKPen hpWindThick;
  static      LKPen hpThermalCircle;
  static      LKPen hpOvertarget;
  static      LKPen hpThermalBand;
  static      LKPen hpThermalBandGlider;
  static      LKPen hpFinalGlideAbove;
  static      LKPen hpFinalGlideBelow;
  static      LKPen hpTerrainLine;
  static      LKPen hpTerrainLineBg;
  static      LKPen hpStartFinishThick;
  static      LKPen hpStartFinishThin;

 private:

  static unsigned fpsTime0;

  static void CalculateOrigin(const RECT& rc, POINT *Orig);


protected:
  static void RenderMapWindow(LKSurface& Surface, const RECT& rc);
  static void UpdateCaches(const ScreenProjection& _Proj, bool force=false);

private:
  static void RenderOverlayGauges(LKSurface& Surface, const RECT& rc);
  static void RenderMapWindowBg(LKSurface& Surface, const RECT& rc);
  static double findMapScaleBarSize(const RECT& rc);

  static int ScaleListCount;
  static int ScaleCurrent;
  static double ScaleList[SCALELISTSIZE];
  static double StepMapScale(int Step);
  static double FindMapScale(double Value);

public:  
  static void FillScaleListForEngineeringUnits(void);


private:
  static void StoreRestoreFullscreen(bool);

 public:

  static double GetApproxScreenRange(void);
  static int GetMapResolutionFactor();

  static POINT GetOrigScreen(void) { return Orig_Screen; }

 private:
  static POINT Orig_Screen;
  static double TargetZoomDistance;
  static int TargetPanIndex;
  static void ClearAirSpace(bool fill, const RECT& rc);

 //----------------------
 /// map label declutering
 private:
  /// to store list of label coordinate already printed in vertical slot
  using slot_t = std::vector<PixelRect>;
  
  /// vertical slot list, vertical slot size is "ScreenSizeY / 8"
  // for ScreenSizeY = 480px each slot size is 60px
  using LabelBlockCoords_t = std::array<slot_t, 9>;
  static LabelBlockCoords_t LabelBlockCoords;

  static int nLabelBlocks; // count of label already printed;

 public:
  /// return true if @rc not overlap with previously checked rect.
  static bool checkLabelBlock(const RECT& rc, const RECT& clipRect);
  /// remove all previously checked rect
  static void ResetLabelDeclutter();
  /// after calling this, #checkLabelBlock() always return false.
  // call reset for revert to normal state.
  static void SaturateLabelDeclutter();
 //----------------------


  static bool RenderTimeAvailable();
  static bool TargetMoved(double &longitude, double &latitude);

    // Touch Screen Events Area
    static void UpdateActiveScreenZone(RECT rc);

protected:
	static void _OnSize(int cx, int cy);
	static void _OnCreate(Window& Wnd, int cx, int cy);
	static void _OnDestroy();

/////////////////////////////////////////////////////
// Mouse Event Handling /////////////////////////////
	static void _OnDragMove(const POINT& Pos);

	static void _OnLButtonDown(const POINT& Pos);
    static void _OnLButtonUp(const POINT& Pos);

	static void _OnLButtonDblClick(const POINT& Pos);

    // Values to be remembered
    static bool pressed;
    static double Xstart, Ystart;
    static PeriodClock tsDownTime;
    static double Xlat, Ylat;


/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
// Keyboard Event Handling //////////////////////////
    static void _OnKeyDown(unsigned KeyCode);

	static void key_bottombar_previous();
	static void key_bottombar_next();
	static void key_overtarget_rotate();
	static void key_topcenter();
	static void key_topleft();
	static void key_topright();
	static void key_enter();
	static void key_gesture_down();
	static void key_gesture_up();
	static void key_previous_page();
	static void key_next_page();
	static void key_down();
	static void key_up();
	static void key_previous_mode();
	static void key_next_mode();
/////////////////////////////////////////////////////

 private:
  static NMEA_INFO DrawInfo;
  static DERIVED_INFO DerivedDrawInfo;

  static void CalculateOrientationTargetPan(void);
  static void CalculateOrientationNormal(void);

#ifdef ENABLE_OPENGL
  static std::array<FloatPoint, NUMTERRAINSWEEPS+2> Groundline;
  static std::array<FloatPoint, NUMTERRAINSWEEPS+1> Groundline2;
#else
  static std::array<RasterPoint, NUMTERRAINSWEEPS+1> Groundline;
  static std::array<RasterPoint, NUMTERRAINSWEEPS+1> Groundline2;
#endif

  static bool targetMoved;
  static double targetMovedLat;
  static double targetMovedLon;

  // include declaration for alpha blended drawing
  #include "MapWindowA.h"
};

void PolygonRotateShift(POINT* poly, int n, int x, int y,
                        double angle);

void threadsafePolygonRotateShift(POINT* poly, const int n, const int xs, const int ys, const double angle);

#endif
