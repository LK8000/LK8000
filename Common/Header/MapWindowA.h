 // the folowing code is included inside MapWindow class
 
 // declaration of interface for alpha blended drawing 
 public:
   
  // check if alpha blending is supported (valid after calling AlphaBlendInit())
  static bool AlphaBlendSupported() {
    return TRUE;
  }    
 
 private:
   
  typedef BOOL (WINAPI *TAlphaBlendF)(HDC,int,int,int,int,HDC,int,int,int,int,BLENDFUNCTION); 

  // pointer to AlphaBlend() function (initialized in AlphaBlendInit())
  static TAlphaBlendF AlphaBlendF;
  
  // tries to locate AlphaBlend() function and initializes some data needed for alpha blending;
  // sets pointer to AlphaBlend function (AlphaBlendF) 
  // (returns false when AlphaBlending is not supported)
  static bool AlphaBlendInit();
  
  // release resources used for alpha blending
  static void AlphaBlendDestroy();
  
  // performs AlphaBlend
  static void DoAlphaBlend(HDC dstHdc, const RECT dstRect, HDC srcHdc, const RECT srcRect, BYTE globalOpacity);

 // declaration of interface for alpha blended air space drawing
 public:
  
  typedef enum 
  {
    asp_fill_border_only        = 0,  // airspace drawing using no filling
    asp_fill_patterns_full      = 1,  // airspace drawing using patterns
    asp_fill_patterns_borders   = 2,  // airspace drawing using patterns, borders
    asp_fill_ablend_full        = 3,  // airspace drawing using alpha blend 
    asp_fill_ablend_borders     = 4,  // airspace drawing using alpha blend, borders
  } EAirspaceFillType;
  
  // set airspace drawing type
  static void SetAirSpaceFillType(EAirspaceFillType fillType) {
    AirspaceFillType = fillType;
  }

  // get airspace drawing type
  static EAirspaceFillType GetAirSpaceFillType(void) {
    return AirspaceFillType;
  }
  
  // set alpha blended airspace opacity (0..100)
  static void SetAirSpaceOpacity(int opacity) {
    AirspaceOpacity = opacity;
  }

  // get alpha blended airspace opacity
  static int GetAirSpaceOpacity(void) {
    return AirspaceOpacity;    
  }
  
 private:

  // airspace drawing type
  static EAirspaceFillType AirspaceFillType;
  
  // alpha blended airspace opacity (0..100)
  static BYTE AirspaceOpacity;
 
  // solid brushes for airspace drawing (initialized in InitAirSpaceSldBrushes())
  static HBRUSH hAirSpaceSldBrushes[NUMAIRSPACECOLORS];
 
  // initialize solid color brushes for airspace drawing (initializes hAirSpaceSldBrushes[])
  static void InitAirSpaceSldBrushes(const COLORREF colours[]);

  static HBRUSH GetAirSpaceSldBrushByClass(int i) {
    return hAirSpaceSldBrushes[iAirspaceColour[i]];
  }

  // draw airspace using alpha blending
  static void ClearTptAirSpace(HDC hdc, const RECT rc);
  
  // draw airspace using alpha blending
  static void DrawTptAirSpace(HDC hdc, const RECT rc);
