/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: MapWindow.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#if !defined(AFX_MAPWINDOW_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_MAPWINDOW_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#include "Airspace.h"

#define NORTHSMART 5
#define NORTHTRACK 4
#define TRACKCIRCLE 3
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


#include "RGB.h"

// NOT USED ANYMORE, USE RGB_xxx as color definition
// Used by MapWindow::TextColor 
// 5 bits (0-30) . Some colors unused
// #define TEXTBLACK 0
// #define TEXTWHITE 1
// #define TEXTGREEN 2
// #define TEXTRED 3
// #define TEXTBLUE 4
// #define TEXTYELLOW 5
// #define TEXTCYAN 6
// #define TEXTMAGENTA 7
// #define TEXTGREY 8
// #define TEXTORANGE 9
// #define TEXTLIGHTGREEN 10
// #define TEXTLIGHTRED 11
// #define TEXTLIGHTBLUE 12
// #define TEXTLIGHTYELLOW 13
// #define TEXTLIGHTCYAN 14
// #define TEXTLIGHTGREY 15
// #define TEXTLIGHTORANGE 16

// VENTA3 note> probably it would be a good idea to separate static WP data to dynamic values,
// by moving things like Reachable, AltArival , etc to WPCALC
// Currently at 5.2.2 the whole structure is saved into the task file, so everytime we 
// change the struct all old taks files become invalid... (there's a bug, btw, in this case)

typedef struct _WAYPOINT_INFO
{
  // Number is used as an alternate short name, and can also be duplicated
  int Number;
  double Latitude;
  double Longitude;
  double Altitude;
  int Flags;
  TCHAR Name[NAME_SIZE + 1];
  TCHAR *Comment;
  POINT	Screen;
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
} WAYPOINT;

// This struct is separated from _WAYPOINT_INFO and will not be used in task files.  
// It is managed by the same functions that manage WayPointList, only add variables here
// and use them like  WayPointCalc[n].Distance  for example. 
// THIS STRUCTURE MUST BE INITIALIZED inside waypointparser function!!
typedef struct _WAYPOINT_CALCULATED
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
} WPCALC;

typedef struct _SNAIL_POINT
{
  float Latitude;
  float Longitude;
  float Vario;
  double Time;
  POINT Screen;
  short Colour;
  BOOL Circling;
  bool FarVisible;
  double DriftFactor;
} SNAIL_POINT;



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
    int Color;
}TextInBoxMode_t;


typedef struct{
  TCHAR Name[NAME_SIZE+1];
  POINT Pos;
  TextInBoxMode_t Mode;
  int AltArivalAGL;
  bool inTask;
  bool isLandable; // VENTA5
  bool isAirport; // VENTA5
  bool isExcluded;
  int  index;
  short style;
}MapWaypointLabel_t;


typedef struct
{
  double fXMin, fXMax;
  double fYMin, fYMax;
  RECT rc;
} DiagrammStruct;



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
    // initial fixed zoom factors - in user distance units, km,mi what is selected!
    // Values are used in dlgConfiguration, to show values in config options
    static const double SCALE_CRUISE_INIT;
    static const double SCALE_CIRCLING_INIT;
    static const double SCALE_PANORAMA_INIT;
    static const double SCALE_PG_PANORAMA_INIT;
    static const double SCALE_INVALID_INIT;
    
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
    double _scaleOverDistanceModify;
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

    double GetPgClimbZoomInitValue(int parameter_number) const;
    double GetPgCruiseZoomInitValue(int parameter_number) const;

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

    bool GetPgClimbInitMapScaleText(int init_parameter, TCHAR *out, size_t size) const;
    bool GetPgCruiseInitMapScaleText(int init_parameter, TCHAR *out, size_t size) const;
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
    
  public:
    Mode();
    
    TModeFly UserForcedMode() const    { return _userForcedFlyMode; }
    void UserForcedMode(TModeFly mode) { _userForcedFlyMode = mode; }
    
    bool Is(TMode mode) const   { return _mode & mode; }
    void Fly(TModeFly flyMode);
    TModeFly Fly() const        { return static_cast<TModeFly>(_mode & FLY_MASK); }
    
    void Special(TModeSpecial specialMode, bool enable);
    TModeSpecial Special() const { return static_cast<TModeSpecial>(_mode & SPECIAL_MASK); }
    
    bool AnyPan() const { return _mode & (MODE_SPECIAL_PAN | MODE_SPECIAL_TARGET_PAN); }
  };
  

  static bool IsDisplayRunning();
  static int iAirspaceMode[AIRSPACECLASSCOUNT];
  static int iAirspaceBrush[AIRSPACECLASSCOUNT]; 
  static int iAirspaceColour[AIRSPACECLASSCOUNT];
  static bool bAirspaceBlackOutline;
  static BOOL CLOSETHREAD;

  static COLORREF GetAirspaceColour(int i) {
    return Colours[i];
  }
  static HBRUSH GetAirspaceBrush(int i) {
    return hAirspaceBrushes[i];
  }
  static COLORREF GetAirspaceColourByClass(int i) {
    return Colours[iAirspaceColour[i]];
  }
  static HBRUSH GetAirspaceBrushByClass(int i) {
    return hAirspaceBrushes[iAirspaceBrush[i]];
  }

 private:

  static BOOL Initialised;
  static bool GliderCenter;
  static DWORD timestamp_newdata;
  static bool LandableReachable;

 public:

  // 12 is number of airspace types
  static COLORREF Colours[NUMAIRSPACECOLORS];
  static HPEN hAirspacePens[AIRSPACECLASSCOUNT];
  static HPEN hAirspaceBorderPen;
  static HPEN hSnailPens[NUMSNAILCOLORS];
  static COLORREF hSnailColours[NUMSNAILCOLORS];
  static HBRUSH hAirspaceBrushes[NUMAIRSPACEBRUSHES];
  static HBRUSH hAboveTerrainBrush;

  static Zoom zoom;
  static Mode mode;

  static RECT MapRect;			// See explanation in MapWndProc
  static RECT DrawRect;
  static bool ForceVisibilityScan;

  static bool MapDirty;

  static unsigned char DeclutterLabels;
  static bool EnableTrailDrift;
  static int GliderScreenPosition;
  static int GliderScreenPositionX;
  static int GliderScreenPositionY;
  static void RequestFastRefresh();

  static void UpdateTimeStats(bool start);

  // Drawing primitives
  static void DrawDashLine(HDC , const int , const POINT , const POINT , 
			   const COLORREF , 
			   const RECT rc);

  #ifdef GTL2
  static void DrawDashPoly(HDC hdc, const int width, const COLORREF color,
                           POINT* pt, const int npoints, const RECT rc);
  #endif

  /* Not used
  static void DrawDotLine(HDC, const POINT , const POINT , const COLORREF , 
			  const RECT rc);
  */

  static void _DrawLine(HDC hdc, const int PenStyle, const int width, 
	       const POINT ptStart, const POINT ptEnd, 
	       const COLORREF cr, const RECT rc);
  static void _Polyline(HDC hdc, POINT* pt, const int npoints, const RECT rc);
  static void DrawBitmapIn(const HDC hdc, const POINT &sc, const HBITMAP h, const bool autostretch);
  static void DrawBitmapX(const HDC hdc, const int top, const int right,
		     const int sizex, const int sizey,
		     const HDC source,
		     const int offsetx, const int offsety,
		     const DWORD mode,const bool autostretch);

  // ...
  static void RequestToggleFullScreen();
  static void RequestOnFullScreen();
  static void RequestOffFullScreen();

  static void OrigScreen2LatLon(const int &x, const int &y, 
                                double &X, double &Y);
  static void Screen2LatLon(const int &x, const int &y, double &X, double &Y);

  static void LatLon2Screen(const double &lon, const double &lat, POINT &sc);
  static void LatLon2Screen(pointObj *ptin, POINT *ptout, const int n,
			    const int skip);

  static void CloseDrawingThread(void);
  static void CreateDrawingThread(void);
  static void SuspendDrawingThread(void);
  static void ResumeDrawingThread(void);
  static void LKWriteText(HDC hDC, const TCHAR* wText, int x, int y, int maxsize, const bool mode, const short align, COLORREF rgb_tex, bool invertable);
  static void LKWriteBoxedText(HDC hDC, const TCHAR* wText, int x, int y, int maxsize, const short align );
  static bool LKFormatValue(const short fvindex, const bool longtitle, TCHAR *BufferValue, TCHAR *BufferUnit, TCHAR *BufferTitle);
  static void LKFormatBrgDiff(const int wpindex, const bool wpvirtual, TCHAR *BufferValue, TCHAR *BufferUnit);

  static LRESULT CALLBACK MapWndProc (HWND hWnd, UINT uMsg, WPARAM wParam,LPARAM lParam);

  static bool IsMapFullScreen();
  static bool ChangeDrawRect(const RECT rectarea);

  // input events or reused code
  static void Event_Pan(int vswitch);
  static void Event_TerrainTopology(int vswitch);
  static void Event_PanCursor(int dx, int dy);
  static bool Event_InteriorAirspaceDetails(double lon, double lat);
  static bool Event_NearestWaypointDetails(double lon, double lat, double range, bool pan);

  static void UpdateInfo(NMEA_INFO *nmea_info,
			 DERIVED_INFO *derived_info);
  static rectObj CalculateScreenBounds(double scale);
  static void ScanVisibility(rectObj *bounds_active);

  static int HeightToY(double fHeight,  DiagrammStruct* psDia);
  static int DistanceToX(double fDist,  DiagrammStruct* psDia)  ;
  static void RenderNearAirspace(HDC hdc, const RECT rci);
  static int AirspaceTopView(HDC hdc,   DiagrammStruct* pDia, double iAS_Bearing, double wpt_brg);
  static void RenderAirspace(HDC hdc, const RECT rc);
  static void LKDrawFlarmRadar(HDC hdc, const RECT rci);
  static void LKDrawMultimap_Example(HDC hdc, const RECT rci);
  static void LKDrawMultimap_Test(HDC hdc, const RECT rci);
  static void LKDrawMultimap_Asp(HDC hdc, const RECT rci);
  static void LKDrawMultimap_Radar(HDC hdc, const RECT rci);
  static int DrawFlarmObjectTrace(HDC hDC,double fZoom, DiagrammStruct* Dia);
 private:
  static void CalculateScreenPositions(POINT Orig, RECT rc, 
                                       POINT *Orig_Aircraft);
  static void CalculateScreenPositionsGroundline();
  static void CalculateScreenPositionsAirspace();
  static void CalculateScreenPositionsThermalSources();
  static void LKCalculateWaypointReachable(short multicalc_slot, short numslots);
  
  static bool PointVisible(const POINT &P);
  static bool PointVisible(const double &lon, const double &lat);
  static bool PointInRect(const double &lon, const double &lat,
			  const rectObj &bounds);

  static void DrawAircraft(HDC hdc, const POINT Orig);
  static void DrawCrossHairs(HDC hdc, const POINT Orig, const RECT rc);
  static void DrawGlideCircle(HDC hdc, const POINT Orig, const RECT rc); // VENTA3
  static void DrawHeading(HDC hdc, const POINT Orig, const RECT rc); // VENTA10
  static void DrawBestCruiseTrack(HDC hdc, const POINT Orig);
  static void DrawCompass(HDC hdc, const RECT rc);
  static void DrawTRI(HDC hdc, const RECT rc);
  static void DrawAcceleration(HDC hdc, const RECT rc);
  static void DrawTarget(HDC hdc, const RECT rc,int ttop,int tbottom,int tleft,int tright);
  //  static void DrawHorizon(HDC hdc, const RECT rc);
  //  static void DrawWind(HDC hdc, POINT Orig, RECT rc);
  //  static void DrawWindAtAircraft(HDC hdc, POINT Orig, RECT rc);
  static void DrawWindAtAircraft2(HDC hdc, POINT Orig, RECT rc);
  static void DrawAirSpace(HDC hdc, const RECT rc);
  static void DrawAirspaceLabels(HDC hdc, const RECT rc, const POINT Orig_Aircraft);
  static void DrawWaypoints(HDC hdc, const RECT rc);
  static void DrawWaypointsNew(HDC hdc, const RECT rc);
  static void DrawLook8000(HDC hdc, const RECT rc);
  static void DrawMapSpace(HDC hdc, const RECT rc);
  static void DrawNearest(HDC hdc, const RECT rc);
  static void DrawAspNearest(HDC hdc, const RECT rc);
  static void DrawCommon(HDC hdc, const RECT rc);
  static void DrawInfoPage(HDC hdc, const RECT rc, const bool forceinit);
  static void DrawTraffic(HDC hdc, const RECT rc);
  static void DrawThermalHistory(HDC hdc, const RECT rc);
  static void WriteInfo(HDC hdc, bool *showunit, TCHAR *BufferValue, TCHAR *BufferUnit, TCHAR *BufferTitle,
                                short *columnvalue, short *columntitle, short *row1, short *row2, short *row3);
  // static bool LKFormatValue(const short fvindex, const bool longtitle, TCHAR *BufferValue, TCHAR *BufferUnit, TCHAR *BufferTitle);
  static void LKFormatDist(const int wpindex, const bool wpvirtual, TCHAR *BufferValue, TCHAR *BufferUnit);
  // static void LKFormatBrgDiff(const int wpindex, const bool wpvirtual, TCHAR *BufferValue, TCHAR *BufferUnit);
  static void LKFormatGR(const int wpindex, const bool wpvirtual, TCHAR *BufferValue, TCHAR *BufferUnit);
  static void LKFormatAltDiff(const int wpindex, const bool wpvirtual, TCHAR *BufferValue, TCHAR *BufferUnit);
  static void LKUpdateOlc(void);
//  static void LKWriteText(HDC hDC, const TCHAR* wText, int x, int y, int maxsize, const bool mode, const short align, COLORREF rgb_tex, bool invertable);

#ifdef CPUSTATS
  static void DrawCpuStats(HDC hdc, const RECT rc);
#endif
#ifdef DRAWDEBUG
  static void DrawDebug(HDC hdc, const RECT rc);
#endif
  static void DrawWelcome8000(HDC hdc, const RECT rc); 
  static void DrawLKStatus(HDC hdc, const RECT rc);
  static void DrawFlightMode(HDC hdc, const RECT rc);
  static void DrawGPSStatus(HDC hdc, const RECT rc);
  static void DrawFunctions1HZ(HDC hdc, const RECT rc);
  static void DrawLKAlarms(HDC hdc, const RECT rc);
  static void DrawFDRAlarms(HDC hdc, const RECT rc);



  static void DrawYGrid(HDC hdc, RECT rc, double ticstep,double unit_step, double zero, int iTextAling,
		                COLORREF color, DiagrammStruct *psDia);
  static void DrawXGrid(HDC hdc, RECT rc, double ticstep,double unit_step, double zero, int iTextAling,
                        COLORREF color, DiagrammStruct *psDia);


  static double LKDrawTrail(HDC hdc, const POINT Orig, const RECT rc);
  static void DrawTeammate(HDC hdc, const RECT rc);
  static void DrawOffTrackIndicator(HDC hdc, const RECT rc);
  static void DrawProjectedTrack(HDC hdc, const RECT rc, const POINT Orig);
  static void DrawStartSector(HDC hdc, const RECT rc, POINT &Start,
                              POINT &End, int Index);
  static void DrawTask(HDC hdc, RECT rc, const POINT &Orig_Aircraft);
  static void DrawThermalEstimate(HDC hdc, const RECT rc);
  static void DrawThermalEstimateMultitarget(HDC hdc, const RECT rc);
  static void DrawTaskAAT(HDC hdc, const RECT rc);
  static void DrawBearing(HDC hdc, const RECT rc);
  static void DrawGreatCircle(HDC hdc,
                              double lon_start, double lat_start,
                              double lon_end, double lat_end,
			      const RECT rc);
  // static void DrawMapScale(HDC hDC,RECT rc);
  static void DrawMapScale(HDC hDC, const RECT rc, 
			   const bool ScaleChangeFeedback);
  static void DrawMapScale2(HDC hDC, const RECT rc, 
			    const POINT Orig_Aircraft);
  static void DrawFinalGlide(HDC hDC, const RECT rc);
  static void DrawThermalBand(HDC hDC, const RECT rc);
  static void DrawGlideThroughTerrain(HDC hDC, const RECT rc);
  static void DrawTerrainAbove(HDC hDC, const RECT rc);
  static void LKDrawFLARMTraffic(HDC hDC, RECT rc, POINT Orig_Aircraft);
  static void LKDrawVario(HDC hDC, RECT rc);

    
  static void DrawSolidLine(const HDC&hdc, 
			    const POINT&start, 
			    const POINT&end ,
			    const RECT rc);
  static bool TextInBox(HDC hDC, TCHAR* Value, int x, int y, int size, TextInBoxMode_t *Mode, bool noOverlap=false);
  static void ToggleFullScreenStart();
  //static void TextColor(HDC hDC, short colorcode);
  static bool WaypointInTask(int ind);

 private:
  static int iSnailNext;
  static HBITMAP hDrawBitMap;
  static HBITMAP hDrawBitMapTmp;
  static HBITMAP hMaskBitMap;
  static HDC hdcDrawWindow;
  static HDC hdcScreen;
  static HDC hDCTemp;
  static HDC hDCMask;
#if NEWSMARTZOOM
  static HBITMAP hQuickDrawBitMap;
  static HDC hdcQuickDrawWindow;
#endif
  static double PanLatitude;
  static double PanLongitude;
  static DWORD  dwDrawThreadID;
  static HANDLE hDrawThread;
  static double DisplayAngle;
  static double DisplayAircraftAngle;
  static DWORD targetPanSize;
  
 public:
  static void RefreshMap(); // set public VENTA
  static HANDLE hRenderEvent;

  static rectObj screenbounds_latlon;
  
  static BOOL THREADRUNNING;
  static BOOL THREADEXIT;
  
  static double LimitMapScale(double value);

  static bool WaypointInRange(int i);

  static void SetTargetPan(bool dopan, int task_index, DWORD dlgSize = 0);

  static double GetPanLatitude() { return PanLatitude; }
  static double GetPanLongitude() { return PanLongitude; }
  static double GetInvDrawScale() { return zoom.InvDrawScale(); }
  static double GetDisplayAngle() { return DisplayAngle; }
  static void SetAutoOrientation(bool doreset);

  static HBRUSH   hInvBackgroundBrush[LKMAXBACKGROUNDS]; // fixed number of backgrounds in MapWindow

  static      HPEN hpAircraft;
  static      HPEN hpAircraftBorder;
  static      HPEN hpWind;
  static      HPEN hpWindThick;
  static      HPEN hpBearing;
  static      HPEN hpBestCruiseTrack;
  static      HPEN hpCompass;
  static	HPEN hpThermalCircle;
  static      HPEN hpOvertarget;
  static      HPEN hpThermalBand;
  static      HPEN hpThermalBandGlider;
  static      HPEN hpFinalGlideAbove;
  static      HPEN hpFinalGlideBelow;
  static      HPEN hpMapScale;
  static      HPEN hpMapScale2;
  static      HPEN hpTerrainLine;
  static      HPEN hpTerrainLineBg;
#ifdef GTL2
  static      HPEN hpTerrainLine2Bg; // for next-WP glide terrain line
#endif
  static      HPEN hpVisualGlideLightRed; // VENTA3
  static      HPEN hpVisualGlideHeavyRed; // 
  static      HPEN hpVisualGlideLightBlack; // VENTA3
  static      HPEN hpVisualGlideHeavyBlack; // 
  static      HPEN hpVisualGlideExtra; // future use
  static      HPEN hpStartFinishThick;
  static      HPEN hpStartFinishThin;
  
  static      HBRUSH hbCompass;
  static      HBRUSH hbThermalBand;
  static      HBRUSH hbBestCruiseTrack;
  static      HBRUSH hbFinalGlideBelow;
  static      HBRUSH hbFinalGlideAbove;
  static      HBRUSH hbWind;

  static HPEN    hpCompassBorder;

 private:

  static DWORD fpsTime0;

  static void CalculateOrigin(const RECT rc, POINT *Orig);


  static DWORD DrawThread (LPVOID);

  static void RenderMapWindow(  RECT rc);
  static void RenderMapWindowBg(HDC hdc, const RECT rc,
				const POINT &Orig,
				const POINT &Orig_Aircraft);
  static void UpdateCaches(bool force=false);
  static double findMapScaleBarSize(const RECT rc);

  #define SCALELISTSIZE  30
  static int ScaleListCount;
  static int ScaleCurrent;
  static double ScaleList[SCALELISTSIZE];
  static double StepMapScale(int Step);
  static double FindMapScale(double Value);
  static void FillScaleListForEngineeringUnits(void);
  

  #if TOPOFASTLABEL
public:
	static void FreeSlot(); 
private:
  // How many slots in screen, divided by horizontal blocks on vertical positions
  // How many parts of vertical screens we are using. H=480 mean 48 pixel sized slots
  // Each slot should contain MapWindowLogFont Hsize with some margin.
  #define SCREENVSLOTS	10
  // Max number of labels in each vblock
  #define MAXVLABELBLOCKS 15
  // total labels printed so far
  static int nLabelBlocks;
  static int nVLabelBlocks[SCREENVSLOTS+1];
  static RECT LabelBlockCoords[SCREENVSLOTS+1][MAXVLABELBLOCKS+1];
  static char *slot;
  #else
  #define MAXLABELBLOCKS 100
  static int nLabelBlocks;
  static RECT LabelBlockCoords[MAXLABELBLOCKS];
  #endif

  static void StoreRestoreFullscreen(bool);
 public:

  static double GetApproxScreenRange(void);
  static int GetMapResolutionFactor();

  static POINT GetOrigScreen(void) { return Orig_Screen; }

 private:
  static POINT Orig_Screen;
  static double TargetZoomDistance;
  static int TargetPanIndex; 
  static void ClearAirSpace(bool fill);

 public:
  #if TOPOFASTLABEL
  static bool checkLabelBlock(RECT *rc);
  #else
  static bool checkLabelBlock(RECT rc);
  #endif
  static void ResetLabelDeclutter(void);
  static void SaturateLabelDeclutter(void);
  static bool RenderTimeAvailable();
  static int SnailWidthScale; 
  static bool TargetMoved(double &longitude, double &latitude);

 private:
  static NMEA_INFO DrawInfo;
  static DERIVED_INFO DerivedDrawInfo;

  static void CalculateOrientationTargetPan(void);
  static void CalculateOrientationNormal(void);

  static POINT Groundline[NUMTERRAINSWEEPS+1];
#ifdef GTL2
  static POINT Groundline2[NUMTERRAINSWEEPS+1];
#endif

  static bool targetMoved;
  static double targetMovedLat;
  static double targetMovedLon;

  // include declaration for alpha blended drawing
  #include "MapWindowA.h"
};

void PolygonRotateShift(POINT* poly, int n, int x, int y, 
                        double angle);


#endif

